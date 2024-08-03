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


/* ((Total #of working MACs)/(#MACs)) / (Total Cycles)*/
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
        /* #Weights Calculation. */
        int total_weights = config->array_h * config->array_w;
        int left_weights = total_weights;
        int on_chip_weights = 0;
        int filter_sram_size = config->filter_sram_size;
        on_chip_weights = filter_sram_size <= left_weights ? filter_sram_size : left_weights;
        left_weights -= on_chip_weights;
        /* Weights Filling Simulation. */
        int acc = 0;
        int add_acc = get<0>(i);
        int weight_cycles = config->array_h;
        int stall_cycles = 0;
        int off_chip_memory_cycles = config->off_chip_memory_cycles;
        int start_cycles = cycles;
        for (int c = 1; c <= weight_cycles; c++) {
            on_chip_weights -= add_acc;
            if (on_chip_weights < 0) {
                /* Updates. */
                on_chip_weights += add_acc;
                left_weights += on_chip_weights;
                on_chip_weights = filter_sram_size <= left_weights ? filter_sram_size : left_weights;
                left_weights -= on_chip_weights;

                /* Stall Cycles. */
                stall_cycles += off_chip_memory_cycles;
                cout << "Stall Cycles: " << off_chip_memory_cycles;
                cout << "(" << cycles << "~" << (cycles + off_chip_memory_cycles - 1) << ")\n";
                cycles += off_chip_memory_cycles;
            }
            acc += add_acc;
            cycles++;   
        }
        /* Weights Finish. */
        assert(acc == total_weights);
        cout << "Weight Filling Cycles: " << weight_cycles;
        result->weight_fill_cycles += weight_cycles;
        cout << "(" << start_cycles << "~" << (cycles - 1) << ")\n";

        

        total_weights = get<1>(i) * get<2>(i);


        int ifmap_cycles = get<2>(i) + (get<1>(i) - 1) + get<0>(i);
        for (int c = 1; c <= ifmap_cycles; c++) {

        }
        cout << "Activation Cycles: " << ifmap_cycles;
        result->activation_cycles += ifmap_cycles;
        cout << "(" << cycles << "~" << (cycles + ifmap_cycles - 1) << ")\n";
        cycles += ifmap_cycles;

        /* Stalls Update. */
        result->stall_cycles = stall_cycles;
    }
    result->total_cycles = cycles - 1;

    cout << " -------------RESULTS" << "------------- " << '\n';
    cout << "Total Weight Filling Cycles: " << result->weight_fill_cycles << '\n';
    cout << "Total Activation Cycles: " << result->activation_cycles << '\n';
    cout << "Total Stall Cycles: " << result->stall_cycles << '\n';
    cout << "Computation Cycles: " << result->total_cycles << '\n';

    return cycles - 1;
}