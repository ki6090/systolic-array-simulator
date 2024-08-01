#include "cycle.h"
#include <tuple>
#include <vector>
#include <iostream>

using namespace std;

int divide_weight_matrix(vector<tuple<int, int, int>> *matrix, config *config) {
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
            matrix->push_back(make_tuple(a_h, a_w, k));
            /* Update. */
            window_x += a_w;
            left_col -= a_w;
        }
        if (left_col)
            matrix->push_back(make_tuple(a_h, left_col, k));
        /* Update. */
        left_row -= a_h;
        window_y += a_h;
        /* Initialize. */
        left_col = col;
    }
    if (left_row) {
        window_x = a_w;
        while (window_x <= col) {
            matrix->push_back(make_tuple(left_row, a_w, k));
            window_x += a_w;
            left_col -= a_w;
        }
        if (left_col)
            matrix->push_back(make_tuple(left_row, left_col, k));
    }

    for(auto& i: *matrix) {
        std::cout << std::get<0>(i) << " " << std::get<1>(i) << " " << std::get<2>(i) << '\n';
    }

}

int compute_cycles_gemm_ws(config* config) {
    int m = config->mnk.m;
    int n = config->mnk.n;
    int k = config->mnk.k;
    int a_w = config->array_w;
    int a_h = config->array_h;

    vector<tuple<int, int, int>> matrix;
    divide_weight_matrix(&matrix, config);
    int cycle = 0;

    return 0;
}