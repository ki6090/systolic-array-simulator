#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstddef>
#include "Config.h"
#include "SystolicWS.h"

/* Mode */
#define GEMM 1

using namespace std;

int main(int argc, char **argv) {
    if (argc <= 1) {
        cout << "Usage: > " << argv[0] << " [file_name].config " <<'\n';
        return 0;
    }
    else if (argc > 2) {
        printf("Too many arguments.\n");
        return 0;
    }
    Config sim_configs = Config();
    Result sim_results = Result();
    SimulationConfigs configs = sim_configs.simulation_configs;
    SimulationResults results = sim_results.simulation_results;
    results.activation_cycles = 0;
    results.weight_fill_cycles = 0;
    results.total_cycles = 0;
    results.stall_cycles = 0;
    results.computations.empty();

    int8_t error = sim_configs.read_gemm(&configs, argv[1]);
    error = sim_configs.read_arch(&configs, argv[1]);
    if (error == -1) return 0;
    
    sim_configs.print_sim_infos(&configs);

    switch (configs.dataflow)
    {
    case WS:
    {
    SystolicWS systolic_ws = SystolicWS();
    systolic_ws.compute_cycles(&configs, &results);
    systolic_ws.compute_utils(&configs, &results);
    break;
    }
    case OS:
        break;
    case IS:
        break;
    default:
        break;
    }
    return 0;
}

