#ifndef CONFIG_H
#define CONFIG_H
#include <vector>
#include <tuple>
#define WS 1
#define OS 2
#define IS 3 

using namespace std;
typedef struct _SimulationGEMM {
    int32_t m;
    int32_t n;
    int32_t k;
} SimulationGEMM;

typedef struct _SimulationResults {
    std::vector<std::tuple<int, int, int>> computations;
    int32_t weight_fill_cycles;
    int32_t activation_cycles;
    int32_t total_cycles;
    int32_t stall_cycles;
} SimulationResults;

typedef struct _SimulationConfigs {
    int32_t array_w;
    int32_t array_h;
    int32_t dataflow;
    SimulationGEMM mnk;
    int32_t ifmap_sram_size;
    int32_t ofmap_sram_size;
    int32_t filter_sram_size;
    int32_t off_chip_memory_cycles;
    char *path;
} SimulationConfigs;

class Config {
    public: 
        int32_t read_gemm(SimulationConfigs* config, char *path);
        int32_t read_arch(SimulationConfigs* config, char *path);
        void print_sim_infos(SimulationConfigs *configs);
        SimulationConfigs simulation_configs;

    private:
        int32_t get(vector<string>* vp, char *target);
        string trim(const string& str);
};

class Result {
    public:
        SimulationResults simulation_results;
};
#endif