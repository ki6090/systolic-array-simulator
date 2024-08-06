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
    protected:
        static void divide_weights(SimulationConfigs* configs, vector<tuple<int, int, int>>* computations);
        static void prefill_weights(SimulationConfigs *configs, vector<tuple<int, int, int>> *computations, std::deque<int> *weight_vp, deque<int> *ifmap_vp);
        void fill_weights(SimulationConfigs *configs, SimulationResults *results, tuple<int, int, int> i, deque<int> *weight_vp, int *cycles_p, int *stall_cycles_p);
        void activate_ifmaps(SimulationConfigs *configs, SimulationResults *results, tuple<int, int, int> i, deque<int> *ifmap_vp, int *cycles_p, int *stall_cycles_p, int *ofmap_weights_p);
};
#endif