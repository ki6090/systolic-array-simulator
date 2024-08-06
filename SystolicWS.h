#ifndef SYSTOLIC_WS_H
#define SYSTOLIC_WS_H
#include "Config.h"
#include <vector>
#include <tuple>
#include <deque>

using namespace std;

class SystolicWS {
    public:
        int32_t compute_cycles(SimulationConfigs* configs, SimulationResults *results);
        float compute_utils(SimulationConfigs *configs, SimulationResults* results);
        void print_results(SimulationResults *results);
        void print_computations(vector<tuple<int, int, int>> *computations);
        void print_utils(float utils);
    protected:
        static void divide_weights(SimulationConfigs* configs, vector<tuple<int, int, int>>* computations);
        static void prefill_weights(SimulationConfigs *configs, vector<tuple<int, int, int>> *computations, std::deque<int> *weight_vp, deque<int> *ifmap_vp);
        void fill_weights(SimulationConfigs *configs, SimulationResults *results, tuple<int, int, int> i, deque<int> *weight_vp, int *cycles_p, int *stall_cycles_p);
        void activate_maps(SimulationConfigs *configs, SimulationResults *results, tuple<int, int, int> i, deque<int> *ifmap_vp, int *cycles_p, int *stall_cycles_p, int *ofmap_weights_p);
        void print_fill_weights(int32_t start_cycles, int32_t cycles, int32_t weight_cycles, int32_t off_chip_memory_cycles ,vector<tuple<int, int>> stalls);
        void print_activate_maps(int32_t start_cycles, int32_t cycles, int32_t ifmap_cycles, int32_t off_chip_memory_cycles, vector<tuple<int, int, bool>> stalls);
        void print_ofmap_stalls(int32_t cycles, int32_t off_chip_memory_cycles);
};
#endif