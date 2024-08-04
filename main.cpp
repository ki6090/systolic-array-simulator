#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstddef>
#include "include/config.h"
#include "include/print.h"
#include "include/ws.h"

/* Mode */
#define GEMM 1

using namespace std;

int main(int argc, char **argv) {
    if (argc <= 2) {
        cout << "Usage: > " << argv[0] << " [file_name].config " << "[MODE]" <<'\n';
        cout << "MODE: [-gemm]\n";
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
    result.stall_cycles = 0;
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
        if (error == -1) return 0;
        break;
    }    
    default:
        cout << "Please set the mode.\n";
        return 0;
    }

    error = read_arch_config(&config, argv[1]);
    if (error == -1) return 0;
    
    print_scale_sim_infos(&config);

    cout << '\n' << "============COMPUTATIONS============\n";
    switch (config.dataflow)
    {
    case WS:
    {
        switch (mode)
        {
        case GEMM:
            compute_cycles_gemm_ws(&config, &result);
            compute_util_gemm_ws(&config, &result);
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
    cout << "====================================\n";
    return 0;
}

