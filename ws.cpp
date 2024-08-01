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

int compute_cycles_gemm_ws(config* config) {
    int m = config->mnk.m;
    int n = config->mnk.n;
    int k = config->mnk.k;
    int a_w = config->array_w;
    int a_h = config->array_h;

    vector<tuple<int, int, int>> computations;
    divide_ws(&computations, config);
    int cnt = 1;
    for (auto& i: computations) {
        cout << "Computation"<< cnt++ << ": ";
        cout << "(" << get<0>(i) << "x" << get<1>(i) << ")x(" << get<1>(i) << "x" << get<2>(i) << ")\n";
    }
    cout << "============CYCLE-COUNT============\n";
    int cycle = 0;
    cnt = 1;
    for (auto& i: computations) {
        ++cycle;
        cout << " -----------COMPUTATION" << cnt++ << "---------- " << '\n';
        cout << "Arrays Fill Cycles: " << config->array_h;
        cout << "(" << cycle << "~" << (cycle + config->array_h - 1) << ")\n";
        cycle += config->array_h;
        int ifmap_cycle = ((get<2>(i) - 1) + (get<1>(i) - 1) + (get<0>(i) - 1));
        cout << "IFMap Cycles: " << ifmap_cycle;
        cout << "(" << cycle << "~" << (cycle + ifmap_cycle) << ")\n";
        cycle += ifmap_cycle;
    }
    return 0;
}