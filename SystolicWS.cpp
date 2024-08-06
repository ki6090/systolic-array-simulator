#include <iostream>
#include <assert.h>
#include <numeric>
#include <deque>
#include "SystolicWS.h"

using namespace std;

void SystolicWS::print_results(SimulationResults *results) {
    cout << "--------------RESULTS" << "---------------" << '\n';
    cout << "Total Weight Filling Cycles: " << results->weight_fill_cycles << '\n';
    cout << "Total Activation Cycles: " << results->activation_cycles << '\n';
    cout << "Total Stall Cycles: " << results->stall_cycles << '\n';
    cout << "Computation Cycles: " << results->total_cycles << '\n';
    return;
}

void SystolicWS::print_computations(vector<tuple<int, int, int>> *computations) {
    int cnt = 1;
    printf("\n============COMPUTATIONS============\n");
    for (auto& i: *computations) {
        cout << "Computation" << cnt++ << ": ";
        cout << "[" << get<0>(i) << "x" << get<1>(i) << "]x[" << get<1>(i) << "x" << get<2>(i) << "]\n";
    }
    return;
}

void SystolicWS::print_utils(float utils) {
    printf("\n============UTILIZATION=============\n");
    cout << "Overall Utilization: " << utils << "%\n";
    return;
}

void SystolicWS::print_fill_weights(int32_t start_cycles, int32_t cycles, int32_t weight_cycles, int32_t off_chip_memory_cycles ,vector<tuple<int, int>> stalls) {
    cout << "Weight Filling: ";
    cout << start_cycles << "~" << (cycles - 1) << "(" << weight_cycles << "cycles)\n";
    if (!off_chip_memory_cycles) return;
    for (auto& s: stalls) {
        cout << " *Stalls(Filter): ";
        cout << get<0>(s) << "~" << get<1>(s) << "(" << off_chip_memory_cycles << "cycles)\n";
    }
}

void SystolicWS::print_activate_maps(int32_t start_cycles, int32_t cycles, int32_t ifmap_cycles, int32_t off_chip_memory_cycles, vector<tuple<int, int, bool>> stalls) {
    cout << "Activations: ";
    cout << start_cycles << "~" << (cycles - 1) << "(" << ifmap_cycles << "cycles)\n";
    if (off_chip_memory_cycles) return;
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

void SystolicWS::print_ofmap_stalls(int32_t cycles, int32_t off_chip_memory_cycles) {
    cout << "Stalls(Ofmap): ";
    cout << cycles << "~" << cycles + off_chip_memory_cycles - 1 << "(" << off_chip_memory_cycles << "cycles)\n";
    return;
}

void SystolicWS::divide_weights(SimulationConfigs* configs, vector<tuple<int, int, int>> *computations) {
    int col = configs->mnk.n;
    int row = configs->mnk.m;
    int k = configs->mnk.k;
    int a_w = configs->array_w;
    int a_h = configs->array_h;
    int window_x;      
    int left_col = col;
    int left_row = row;
    int window_y = a_h;  

    while (window_y <= row) {
        window_x = a_w;
        while (window_x <= col) {
            computations->push_back(make_tuple(a_h, a_w, k));
            window_x += a_w;
            left_col -= a_w;
        }
        if (left_col) computations->push_back(make_tuple(a_h, left_col, k));
        left_row -= a_h;
        window_y += a_h;
        left_col = col;
    }
    if (left_row) {
        window_x = a_w;
        while (window_x <= col) {
            computations->push_back(make_tuple(left_row, a_w, k));
            window_x += a_w;
            left_col -= a_w;
        }
        if (left_col) computations->push_back(make_tuple(left_row, left_col, k));
    }
    return;
}

/* ((Total #workingMACs) / (#MACs)) / (Total Cycles) */
float SystolicWS::compute_utils(SimulationConfigs *configs, SimulationResults* results) {
    float total_cycles = (float)results->total_cycles;
    float total_macs = (float)configs->array_h * configs->array_w;
    int col_macs = configs->array_w;
    int col_acts;
    int active_macs = 0;

    float acc = 0.0;
    int add_acc = 0;
    float utils;
    for (auto& i: results->computations) {
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
    
    utils = ((acc / total_macs) / (total_cycles)) * 100;
    SystolicWS::print_utils(utils);

    return utils;
}

void SystolicWS::prefill_weights(SimulationConfigs *configs, vector<tuple<int, int, int>> *computations, deque<int> *weight_vp, deque<int> *ifmap_vp) {
    int cnt = 1;
    int weight_sum = 0;
    int filter_sram_size = configs->filter_sram_size;
    int ifmap_sram_size = configs->ifmap_sram_size;
    for (auto& i: *computations) {
        int weights = get<0>(i) * get<1>(i);
        int ifmaps = get<1>(i) * get<2>(i);
        if (weight_sum < filter_sram_size) {
            int put = weights < (filter_sram_size - weight_sum) ? weights : (filter_sram_size - weight_sum);
            weight_vp->push_back(put);
        }
        else weight_vp->push_back(0);

        if (ifmaps <= ifmap_sram_size) 
            ifmap_vp->push_back(ifmaps);
        else if (ifmaps > ifmap_sram_size && ifmap_vp->empty())
            ifmap_vp->push_back(ifmap_sram_size);
        else if (ifmaps > ifmap_sram_size && !ifmap_vp->empty()) 
            ifmap_vp->push_back(0);

        weight_sum = accumulate(weight_vp->begin(), weight_vp->end(), 0);
    }

    return;
}

void SystolicWS::fill_weights(SimulationConfigs *configs, SimulationResults *results, tuple<int, int, int> i, deque<int> *weight_vp, int *cycles_p, int *stall_cycles_p) {
    vector<tuple<int, int>> stalls;
    int m = get<0>(i);
    int n = get<1>(i);
    int weights = m * n;
    int left_weights = weights;
    int on_chip_weights = 0;
    int start_cycles = *cycles_p;
    int stall_cycles = *stall_cycles_p;
    int filter_sram_size = configs->filter_sram_size;
    int off_chip_memory_cycles = configs->off_chip_memory_cycles;
    int cycles = *cycles_p;

    on_chip_weights = weight_vp->front();
    weight_vp->pop_front();
    
    int acc = 0;
    int add_acc = m;
    int weight_cycles = configs->array_h;
    for (int c = 1; c <= weight_cycles; c++) {
        if (acc < weights) {
            left_weights -= add_acc;
            on_chip_weights -= add_acc;
            acc += add_acc;
        }
        if (on_chip_weights < 0) {
            left_weights += add_acc;
            on_chip_weights = filter_sram_size <= left_weights ? filter_sram_size : left_weights;
            stall_cycles += off_chip_memory_cycles;
            stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1));
            cycles += off_chip_memory_cycles;

            left_weights -= add_acc;
            on_chip_weights -= add_acc;
        }
        cycles++;   
    }
    assert(on_chip_weights == 0);
    assert(left_weights == 0);
    assert(acc == weights);

    results->weight_fill_cycles += weight_cycles;
    *cycles_p = cycles;
    *stall_cycles_p = stall_cycles;

    SystolicWS::print_fill_weights(start_cycles, cycles, weight_cycles, off_chip_memory_cycles, stalls);

    return;
}


void SystolicWS::activate_maps(SimulationConfigs *configs, SimulationResults *results, tuple<int, int, int> i, deque<int> *ifmap_vp, int *cycles_p, int *stall_cycles_p, int *ofmap_weights_p) {
    int m = get<0>(i);
    int n = get<1>(i);
    int k = get<2>(i);
    int weights = n * k;
    int left_weights = weights;
    int on_chip_weights = 0;
    int start_cycles = *cycles_p;
    int ifmap_sram_size = configs->ifmap_sram_size;
    int ofmap_sram_size = configs->ofmap_sram_size;
    int off_chip_memory_cycles = configs->off_chip_memory_cycles;
    int ofmap_weights = *ofmap_weights_p;
    int stall_cycles = *stall_cycles_p;
    int cycles = *cycles_p;
    vector<tuple<int, int, bool>> stalls;
    stalls.empty();

    on_chip_weights = ifmap_vp->front();
    ifmap_vp->pop_front();

    int ofmap_acc = 0;
    if (ofmap_weights >= ofmap_sram_size && off_chip_memory_cycles) {
        stall_cycles += off_chip_memory_cycles;
        cycles += off_chip_memory_cycles;  
        *ofmap_weights_p = 0;
        ofmap_weights = 0;
        SystolicWS::print_ofmap_stalls(cycles, off_chip_memory_cycles);
    }

    int acc = 0;
    int add_acc = 0;
    int ifmap_cycles = configs->array_h + (configs->array_w - 1) + k;
    start_cycles = cycles;
    for (int c = 1; c <= ifmap_cycles; c++) {
        if (c <= n) 
            add_acc += 1;
        if (c > k && add_acc > 0) 
            add_acc -= 1;
        if (acc < weights) {
            left_weights -= add_acc;
            on_chip_weights -= add_acc;
            acc += add_acc;
        }
        if (on_chip_weights < 0) {
            left_weights += add_acc;
            on_chip_weights = ifmap_sram_size <= left_weights ? ifmap_sram_size : left_weights;
            stall_cycles += off_chip_memory_cycles;
            stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, false));
            cycles += off_chip_memory_cycles;

            left_weights -= add_acc;
            on_chip_weights -= add_acc;
        }
        if (c > ifmap_cycles - k) {
            ofmap_weights += m;
            ofmap_acc += m;
            if (ofmap_weights > ofmap_sram_size) {
                ofmap_weights = m;
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

    *ofmap_weights_p = ofmap_weights;
    *cycles_p = cycles;
    *stall_cycles_p = stall_cycles;
    results->activation_cycles += ifmap_cycles;

    SystolicWS::print_activate_maps(start_cycles, cycles, ifmap_cycles, off_chip_memory_cycles, stalls);

    return;
}


int32_t SystolicWS::compute_cycles(SimulationConfigs* configs, SimulationResults* results) {
    int cycles = 1;
    int stall_cycles = 0;
    int ofmap_weights = 0;
    int cnt = 1;

    vector<tuple<int, int, int>> *computations = &results->computations; 
    deque<int> weight_v;
    deque<int> ifmap_v;
    
    /* Divide Workloads. */
    SystolicWS::divide_weights(configs, computations);

    /* Pre-Filling to SRAM. Stalls are not calculated. */
    SystolicWS::prefill_weights(configs, computations, &weight_v, &ifmap_v);

    SystolicWS::print_computations(computations);

    printf("\n============CYCLE-COUNT=============\n");
    for (auto& i: *computations) {
        printf("------------COMPUTATION%d------------\n", cnt++);
        /* Weights Filling Simulation. */
        SystolicWS::fill_weights(configs, results, i, &weight_v, &cycles, &stall_cycles);

        /* Activation(Ifmap, Ofmap) Simulation. */
        SystolicWS::activate_maps(configs, results, i, &ifmap_v, &cycles, &stall_cycles, &ofmap_weights);
    }
    results->total_cycles = cycles - 1;
    results->stall_cycles = stall_cycles;

    SystolicWS::print_results(results);
    
    return cycles - 1;
}
