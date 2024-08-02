#include "ws.h"
#include <tuple>
#include <vector>
#include <iostream>

using namespace std;

static int divide_ws(vector<tuple<int, int, int>> *computations, config *config) {
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
    int m = config->mnk.m;
    int n = config->mnk.n;
    int k = config->mnk.k;
    int a_w = config->array_w;
    int a_h = config->array_h;

    vector<tuple<int, int, int>> *computations = &result->computations;
    divide_ws(computations, config);
    int cnt = 1;
    for (auto& i: *computations) {
        cout << "Computation"<< cnt++ << ": ";
        cout << "[" << get<0>(i) << "x" << get<1>(i) << "]x[" << get<1>(i) << "x" << get<2>(i) << "]\n";
    }
    cout << "============CYCLE-COUNT============\n";
    int cycles = 1;
    cnt = 1;
    for (auto& i: *computations) {
        cout << " -----------COMPUTATION" << cnt++ << "---------- " << '\n';
        cout << "Weight Filling Cycles: " << config->array_h;
        result->weight_fill_cycles += config->array_h;
        cout << "(" << cycles << "~" << (cycles + config->array_h - 1) << ")\n";
        cycles += config->array_h;
        
        int ifmap_cycles = get<2>(i) + (get<1>(i) - 1) + get<0>(i);
        cout << "Activation Cycles: " << ifmap_cycles;
        result->activation_cycles += ifmap_cycles;
        cout << "(" << cycles << "~" << (cycles + ifmap_cycles - 1) << ")\n";
        cycles += ifmap_cycles;
    }
    result->total_cycles = cycles - 1;

    cout << " -------------RESULTS" << "------------- " << '\n';
    cout << "Total Weight Filling Cycles: " << result->weight_fill_cycles << '\n';
    cout << "Total Activation Cycles: " << result->activation_cycles << '\n';
    cout << "Computation Cycles: " << result->total_cycles << '\n';

    return cycles - 1;
}