#include <iostream>
#include "include/config.h"
#include "include/print.h"
using namespace std;

void print_results_gemm_ws(result *result) {
    cout << "--------------RESULTS" << "---------------" << '\n';
    cout << "Total Weight Filling Cycles: " << result->weight_fill_cycles << '\n';
    cout << "Total Activation Cycles: " << result->activation_cycles << '\n';
    cout << "Total Stall Cycles: " << result->stall_cycles << '\n';
    cout << "Computation Cycles: " << result->total_cycles << '\n';
    return;
}

void print_scale_sim_infos(config *config) {
    cout << "============MY-SCALE-SIM============\n";
    string dataflow;
    if (config->dataflow == WS)
        dataflow = "Weight Stationary\n";
    else if (config->dataflow == OS)
        dataflow = "Output Stationary\n";
    else if (config->dataflow == IS)
        dataflow = "Input Stationary\n";

    cout << "Array Size: " << config->array_h << "x" << config->array_w << '\n';
    cout << "Data Flow: " << dataflow;
    cout << "GEMM Size: " << "[" << config->mnk.m << "x" << config->mnk.n << "]" << "x" << "[" << config->mnk.n << "x" << config->mnk.k << "]\n";
    cout << "IFMAP SRAM Size: " << config->ifmap_sram_size << '\n';
    cout << "OFMAP SRAM Size: " << config->ofmap_sram_size << '\n';
    cout << "Filter SRAM Size: " << config->filter_sram_size << '\n';
    cout << "Off-Chip Memory Cycles: " << config->off_chip_memory_cycles << '\n';
    cout << "Config Path: " << "./" << config->path << '\n';
}