#include <tuple>
#include <vector>
#include <iostream>
#include <assert.h>
#include <numeric>
#include <deque>
#include "include/print.h"
#include "include/config.h"
#include "include/ws.h"

using namespace std;

static void divide_gemm_ws(config* config, vector<tuple<int, int, int>> *computations) {
    int col = config->mnk.n;
    int row = config->mnk.m;
    int k = config->mnk.k;
    int a_w = config->array_w;
    int a_h = config->array_h;

    /* Divide weight matrix. */
    int window_x;      
    int left_col = col;
    int left_row = row;

    /* Initialize. */
    int window_y = a_h;  

    /* Iterate. */
    while (window_y <= row) {
        /* Initialize. */
        window_x = a_w;
        /* Iterate. */
        while (window_x <= col) {
            computations->push_back(make_tuple(a_h, a_w, k));
            /* Update. */
            window_x += a_w;
            left_col -= a_w;
        }
        if (left_col)
            computations->push_back(make_tuple(a_h, left_col, k));
        /* Update. */
        left_row -= a_h;
        window_y += a_h;
        /* Initialize. */
        left_col = col;
    }
    if (left_row) {
        window_x = a_w;
        while (window_x <= col) {
            computations->push_back(make_tuple(left_row, a_w, k));
            window_x += a_w;
            left_col -= a_w;
        }
        if (left_col)
            computations->push_back(make_tuple(left_row, left_col, k));
    }
    return;
}

/* ((Total #workingMACs) / (#MACs)) / (Total Cycles) */
float compute_util_gemm_ws(config *config, result* result) {
    cout << '\n' << "============UTILIZATION=============\n";
    float total_cycles = (float)result->total_cycles;
    float total_macs = (float)config->array_h * config->array_w;
    int col_macs = config->array_w;
    int col_acts;
    int active_macs = 0;

    float acc = 0.0;
    int add_acc = 0;
    float util;
    for (auto& i: result->computations) {
        add_acc = get<1>(i);
        col_acts = get<2>(i);
        for (int c = 1; c <= get<2>(i) + get<0>(i); c++) {
            if (c <= col_macs && c <= col_acts) 
                active_macs += add_acc;
            else if (c >= col_macs && c > col_acts) 
                active_macs -= add_acc;
            acc += (float)active_macs;
        }
        
    }
    util = ((acc / total_macs) / (total_cycles)) * 100;
    cout << "Overall Utilization: " << util << "%\n";
    return util;
}

static void simulate_prefill_gemm_ws(config *config, vector<tuple<int, int, int>> *computations, deque<int> *weight_vp, deque<int> *ifmap_vp) {
    int cnt = 1;
    int weight_sum = 0;
    int filter_sram_size = config->filter_sram_size;
    int ifmap_sram_size = config->ifmap_sram_size;
    for (auto& i: *computations) {
        cout << "Computation"<< cnt++ << ": ";
        cout << "[" << get<0>(i) << "x" << get<1>(i) << "]x[" << get<1>(i) << "x" << get<2>(i) << "]\n";
        int weights = get<0>(i) * get<1>(i);
        int ifmaps = get<1>(i) * get<2>(i);
        /* Filter. */
        if (weight_sum < filter_sram_size) {
            int put = weights < (filter_sram_size - weight_sum) ? weights : (filter_sram_size - weight_sum);
            weight_vp->push_back(put);
        }
        else {
            weight_vp->push_back(0);
        }
        /* Ifmap. */
        if (ifmaps <= ifmap_sram_size) {
            ifmap_vp->push_back(ifmaps);
        }
        else if (ifmaps > ifmap_sram_size && ifmap_vp->empty()){
            ifmap_vp->push_back(ifmap_sram_size);
        }
        else if (ifmaps > ifmap_sram_size && !ifmap_vp->empty()) {
            ifmap_vp->push_back(0);
        }
        weight_sum = accumulate(weight_vp->begin(), weight_vp->end(), 0);
    }

    return;
}

static void simulate_filling_weights_gemm_ws(config *config, result *result, tuple<int, int, int> i, deque<int> *weight_vp, int *cycles_p, int *stall_cycles_p) {
    vector<tuple<int, int>> stalls;
    int m = get<0>(i);
    int n = get<1>(i);
    int weights = m * n;
    int left_weights = weights;
    int on_chip_weights = 0;
    int start_cycles = *cycles_p;
    int stall_cycles = *stall_cycles_p;
    int filter_sram_size = config->filter_sram_size;
    int off_chip_memory_cycles = config->off_chip_memory_cycles;
    int cycles = *cycles_p;
    on_chip_weights = weight_vp->front();
    weight_vp->pop_front();

    /* Weights Filling Simulation. */
    int acc = 0;
    int add_acc = m;
    int weight_cycles = config->array_h;
    for (int c = 1; c <= weight_cycles; c++) {
        if (acc < weights) {
            left_weights -= add_acc;
            on_chip_weights -= add_acc;
            acc += add_acc;
        }
        if (on_chip_weights < 0) {
            /* Stall Updates. */
            left_weights += add_acc;
            on_chip_weights = filter_sram_size <= left_weights ? filter_sram_size : left_weights;

            /* Stall Cycles. */
            stall_cycles += off_chip_memory_cycles;
            stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1));
            cycles += off_chip_memory_cycles;

            /* Updates. */
            left_weights -= add_acc;
            on_chip_weights -= add_acc;
        }
        cycles++;   
    }
    /* Weights Finish. */
    assert(on_chip_weights == 0);
    assert(left_weights == 0);
    assert(acc == weights);

    cout << "Weight Filling: ";
    result->weight_fill_cycles += weight_cycles;
    cout << start_cycles << "~" << (cycles - 1) << "(" << weight_cycles << "cycles)\n";
    if (off_chip_memory_cycles) {
        for (auto& s: stalls) {
            cout << " *Stalls(Filter): ";
            cout << get<0>(s) << "~" << get<1>(s) << "(" << off_chip_memory_cycles << "cycles)\n";
        }
    }
    *cycles_p = cycles;
    *stall_cycles_p = stall_cycles;
    return;
}

static void simulate_activation_gemm_ws(config *config, result *result, tuple<int, int, int> i, deque<int> *ifmap_vp, int *cycles_p, int *stall_cycles_p, int *ofmap_weights_p) {
/* Ifmap Initialize. */
    int m = get<0>(i);
    int n = get<1>(i);
    int k = get<2>(i);
    int weights = n * k;
    int left_weights = weights;
    int on_chip_weights = 0;
    int start_cycles = *cycles_p;
    int ifmap_sram_size = config->ifmap_sram_size;
    int ofmap_sram_size = config->ofmap_sram_size;
    int off_chip_memory_cycles = config->off_chip_memory_cycles;
    int ofmap_weights = *ofmap_weights_p;
    int stall_cycles = *stall_cycles_p;
    int cycles = *cycles_p;
    vector<tuple<int, int, bool>> stalls;
    stalls.empty();

    on_chip_weights = ifmap_vp->front();
    ifmap_vp->pop_front();

    /* Ofmap Initialize. */
    int ofmap_acc = 0;
    
    /* Move Results to Off-Chip if exists. */
    if (ofmap_weights > 0 && off_chip_memory_cycles) {
        /* Stall Cycles. */
        stall_cycles += off_chip_memory_cycles;
        cout << "Stalls(Ofmap): ";
        cout << cycles << "~" << cycles + off_chip_memory_cycles - 1 << "(" << off_chip_memory_cycles << "cycles)\n";
        cycles += off_chip_memory_cycles;  
        /* Initialize Ofmap Weights. */
        *ofmap_weights_p = 0;
        ofmap_weights = 0;
    }

    /* Activation Simulation. */
    int acc = 0;
    int add_acc = 0;
    int ifmap_cycles = config->array_h + (config->array_w - 1) + k;
    start_cycles = cycles;
    for (int c = 1; c <= ifmap_cycles; c++) {
        /* add_acc Updates. */
        if (c <= n) {
            add_acc += 1;
        }
        if (c > k && add_acc > 0) {
            add_acc -= 1;
        }
        if (acc < weights) {
            left_weights -= add_acc;
            on_chip_weights -= add_acc;
            acc += add_acc;
        }
        if (on_chip_weights < 0) {
            /* Stall Updates. */
            left_weights += add_acc;
            on_chip_weights = ifmap_sram_size <= left_weights ? ifmap_sram_size : left_weights;

            /* Stall Cycles. */
            stall_cycles += off_chip_memory_cycles;
            stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, false));
            cycles += off_chip_memory_cycles;

            /* Updates. */
            left_weights -= add_acc;
            on_chip_weights -= add_acc;
        }
        /* Results Goes to Ofmap every get<1>(i) cycles. */
        if (c > ifmap_cycles - k) {
            ofmap_weights += m;
            ofmap_acc += m;
            if (ofmap_weights > ofmap_sram_size) {
                ofmap_weights = m;
                /* Stall Cycles. */
                stall_cycles += off_chip_memory_cycles;
                stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, true));
                cycles += off_chip_memory_cycles;
            }
        }
        cycles++;
    }
    assert(left_weights == 0);
    assert(on_chip_weights == 0);
    assert(acc == weights);
    assert(ofmap_acc == m * k);

    cout << "Activations: ";
    result->activation_cycles += ifmap_cycles;
    cout << start_cycles << "~" << (cycles - 1) << "(" << ifmap_cycles << "cycles)\n";
    if (off_chip_memory_cycles) {
        for (auto& s: stalls) {
            if (!get<2>(s)) {
                cout << " *Stalls(Ifmap): ";
                cout << get<0>(s) << "~" << get<1>(s) << "(" << off_chip_memory_cycles << "cycles)\n";
            }
            else if (get<2>(s)){
                cout << " *Stalls(Ofmap): ";
                cout << get<0>(s) << "~" << get<1>(s) << "(" << off_chip_memory_cycles << "cycles)\n";
            }       
        }
    }
    *ofmap_weights_p = ofmap_weights;
    *cycles_p = cycles;
    *stall_cycles_p = stall_cycles;
    return;
}

int compute_cycles_gemm_ws(config* config, result *result) {
    int cycles = 1;
    int stall_cycles = 0;
    int ofmap_weights = 0;
    int cnt = 1;
    vector<tuple<int, int, int>> *computations = &result->computations; 
    deque<int> weight_v;
    deque<int> ifmap_v;
    
    /* Divide Workloads. */
    divide_gemm_ws(config, computations);

    /* Pre-Filling to SRAM. Stalls are not calculated. */
    simulate_prefill_gemm_ws(config, computations, &weight_v, &ifmap_v);

    cout << '\n' << "============CYCLE-COUNT=============\n";
    for (auto& i: *computations) {
        cout << "------------COMPUTATION" << cnt++ << "------------" << '\n';
        /* Weights Filling Simulation. */
        simulate_filling_weights_gemm_ws(config, result, i, &weight_v, &cycles, &stall_cycles);

        /* Activation(Ifmap, Ofmap) Simulation. */
        simulate_activation_gemm_ws(config, result, i, &ifmap_v, &cycles, &stall_cycles, &ofmap_weights);
    }
    result->total_cycles = cycles - 1;
    result->stall_cycles = stall_cycles;

    print_results_gemm_ws(result);
    
    return cycles - 1;
}
