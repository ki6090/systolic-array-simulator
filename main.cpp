#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstddef>
#include "ws.h"

/* Mode */
#define GEMM 1

using namespace std;

int main(int argc, char **argv) {
    if (argc <= 1) {
        std::cout << "Usage: >" << argv[0] << " [file_name].config\n";
        return 0;
    }
    else if (argc > 3) {
        std::cout << "Too many arguments.\n";
        return 0;
    }
    
    config config;
    result result;
    result.activation_cycles = 0;
    result.weight_fill_cycles = 0;
    result.total_cycles = 0;
    result.computations.empty();

    int mode = 0;
    int error = 0;
    string token = argv[2];
    if (!token.compare("-gemm")) {
        mode = GEMM;
    }

    switch (mode)
    {
    case GEMM:
    {
        error = read_gemm_config(&config, argv[1]);
        break;
    }    
    default:
        cout << "Please set the mode.\n";
        return 0;
    }
    
    if (error == -1)
        return 0;
    
    string dataflow;
    if (config.dataflow == WS)
        dataflow = "Weight Stationary\n";
    else if (config.dataflow == OS)
        dataflow = "Output Stationary\n";
    else if (config.dataflow == IS)
        dataflow = "Input Stationary\n";
    
    cout << "============MY-SCALE-SIM===========\n";
    cout << "Array Size: " << config.array_h << "x" << config.array_w << '\n';
    cout << "Data Flow: " << dataflow;
    cout << "GEMM Size: " << "[" << config.mnk.m << "x" << config.mnk.n << "]" << "x" << "[" << config.mnk.n << "x" << config.mnk.k << "]\n";
    cout << "Config Path: " << "./" << argv[1] << '\n';
    cout << "============COMPUTATIONS===========\n";
    int compute_cycles = 0;
    float util;
    
    switch (config.dataflow)
    {
    case WS:
    {
        switch (mode)
        {
        case GEMM:
            compute_cycles = compute_cycles_gemm_ws(&config, &result);
            util = compute_util_gemm_ws(&config, &result);
            break;
        default:
            break;
        }
    }
        break;
    case OS:
        break;
    case IS:
        break;
    default:
        break;
    }
    cout << "===================================\n";
    return 0;
}