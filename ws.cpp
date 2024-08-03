#include "ws.h"
#include <tuple>
#include <vector>
#include <iostream>
#include <assert.h>
using namespace std;

static void divide_ws(vector<tuple<int, int, int>> *computations, config *config) {
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
    cout << "============UTILIZATION============\n";
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


int compute_cycles_gemm_ws(config* config, result *result) {
    vector<tuple<int, int, int>> *computations = &result->computations;
    divide_ws(computations, config);
    int cnt = 1;
    for (auto& i: *computations) {
        cout << "Computation"<< cnt++ << ": ";
        cout << "[" << get<0>(i) << "x" << get<1>(i) << "]x[" << get<1>(i) << "x" << get<2>(i) << "]\n";
    }
    cout << "============CYCLE-COUNT============\n";
    int cycles = 1;
    int stall_cycles = 0;
    cnt = 1;
    for (auto& i: *computations) {
        cout << " -----------COMPUTATION" << cnt++ << "---------- " << '\n';
        /* Filter Initialize. */
        vector<tuple<int, int, bool>> stalls;
        stalls.empty();
        int total_weights = get<0>(i) * get<1>(i);
        int left_weights = total_weights;
        int on_chip_weights = 0;
        int filter_sram_size = config->filter_sram_size;
        int off_chip_memory_cycles = config->off_chip_memory_cycles;
        int start_cycles = cycles;
        on_chip_weights = filter_sram_size <= left_weights ? filter_sram_size : left_weights;

        stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, false));
        cycles += off_chip_memory_cycles;
        left_weights -= on_chip_weights;

        /* Weights Filling Simulation. */
        int acc = 0;
        int add_acc = get<0>(i);
        int weight_cycles = config->array_h;
        
        for (int c = 1; c <= weight_cycles; c++) {
            if (acc < total_weights) {
                on_chip_weights -= add_acc;
                acc += add_acc;
            }
            if (on_chip_weights < 0) {
                /* Stall Updates. */
                on_chip_weights += add_acc;
                left_weights += on_chip_weights;
                on_chip_weights = filter_sram_size <= left_weights ? filter_sram_size : left_weights;
                left_weights -= on_chip_weights;

                /* Stall Cycles. */
                stall_cycles += off_chip_memory_cycles;
                stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, false));
                cycles += off_chip_memory_cycles;
            }
            cycles++;   
        }
        /* Weights Finish. */
        assert(acc == total_weights);
        cout << "Weight Filling Cycles: " << weight_cycles;
        result->weight_fill_cycles += weight_cycles;
        cout << "(" << start_cycles << "~" << (cycles - 1) << ")\n";
        for (auto& s: stalls) {
            cout << "*Stall Cycles(Filter): " << off_chip_memory_cycles;
            cout << "(" << get<0>(s) << "~" << get<1>(s) << ")\n";
        }
        stalls.clear();

        /* Ifmap Initialize. */
        total_weights = get<1>(i) * get<2>(i);
        left_weights = total_weights;
        on_chip_weights = 0;
        int ifmap_sram_size = config->ifmap_sram_size;
        on_chip_weights = ifmap_sram_size <= left_weights ? ifmap_sram_size : left_weights;
        stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, false));
        cycles += off_chip_memory_cycles;
        left_weights -= on_chip_weights;

        /* Ofmap Initialize. */
        int ofmap_sram_size = config->ofmap_sram_size;
        int ofmap_weights = 0;
        /* Activation Simulation. */
        acc = 0;
        add_acc = 0;
        int ifmap_cycles = config->array_h + (config->array_w - 1) + get<2>(i);
        start_cycles = cycles;
        for (int c = 1; c <= ifmap_cycles; c++) {
            /* add_acc Updates. */
            if (c <= get<1>(i)) {
                add_acc += 1;
            }
            if (c > get<2>(i) && add_acc > 0) {
                add_acc -= 1;
            }

            if (acc < total_weights) {
                on_chip_weights -= add_acc;
                acc += add_acc;
            }
            if (on_chip_weights < 0) {
                /* Stall Updates. */
                on_chip_weights += add_acc;
                left_weights += on_chip_weights;
                on_chip_weights = ifmap_sram_size <= left_weights ? ifmap_sram_size : left_weights;

                /* Stall Cycles. */
                stall_cycles += off_chip_memory_cycles;
                stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, false));
                cycles += off_chip_memory_cycles;
            }
            /* Results Goes to Ofmap every get<1>(i) cycles. */
            if (c > ifmap_cycles - get<2>(i)) {
                ofmap_weights += get<0>(i);
                if (ofmap_weights > ofmap_sram_size) {
                    ofmap_weights = get<0>(i);
                    /* Stall Cycles. */
                    stall_cycles += off_chip_memory_cycles;
                    stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, true));
                    cycles += off_chip_memory_cycles;
                }
            }
            cycles++;
        }
        assert(acc == total_weights);
        
        /* Sends results to off-chip memory. */
        if (ofmap_weights > 0) {
            stall_cycles += off_chip_memory_cycles;
            stalls.push_back(make_tuple(cycles, cycles + off_chip_memory_cycles - 1, true));
            cycles += off_chip_memory_cycles;
        }
            
        cout << "Activation Cycles: " << ifmap_cycles;
        result->activation_cycles += ifmap_cycles;
        cout << "(" << start_cycles << "~" << (cycles - 1) << ")\n";    
        for (auto& s: stalls) {
            if (!get<2>(s)) {
                cout << "*Stall Cycles(Ifmap): " << off_chip_memory_cycles;
                cout << "(" << get<0>(s) << "~" << get<1>(s) << ")\n";
            }
            else {
                cout << "*Stall Cycles(Ofmap): " << off_chip_memory_cycles;
                cout << "(" << get<0>(s) << "~" << get<1>(s) << ")\n";
            }
            
        }
        stalls.clear();
    }
    result->total_cycles = cycles - 1;
    result->stall_cycles = stall_cycles;

    cout << " -------------RESULTS" << "------------- " << '\n';
    cout << "Total Weight Filling Cycles: " << result->weight_fill_cycles << '\n';
    cout << "Total Activation Cycles: " << result->activation_cycles << '\n';
    cout << "Total Stall Cycles: " << result->stall_cycles << '\n';
    cout << "Computation Cycles: " << result->total_cycles << '\n';

    return cycles - 1;
}