#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstddef>
#include </home/shboo/my-scale-sim/cycle.h>

using namespace std;

int main(int argc, char **argv) {
    if (argc <= 1) {
        std::cout << "Usage: >" << argv[0] << " [file_name].config\n";
        return 0;
    }
    else if (argc > 2) {
        std::cout << "Too many arguments.\n";
        return 0;
    }
    config c;
    read_config_file_gemm(&c, argv[1]);
    string dataflow;
    if (c.dataflow == WS)
        dataflow = "Weight Stationary\n";
    else if (c.dataflow == OS)
        dataflow = "Output Stationary\n";
    else if (c.dataflow == IS)
        dataflow = "Input Stationary\n";
    
    cout << "============MY-GEMM-SIM============\n";
    cout << "Array Size: " << c.array_h << "x" << c.array_w << '\n';
    cout << "Data Flow: " << dataflow;
    cout << "GEMM Size: " << "(" << c.mnk.m << "x" << c.mnk.n << ")" << "x" << "(" << c.mnk.n << "x" << c.mnk.k << ")\n";
    cout << "Config Path: " << "./" << argv[1] << '\n';
    cout << "===================================\n";

    
    int compute_cycles = compute_cycles_gemm_ws(&c);
    return 0;
}