#include <vector>
#include <tuple>
#define WS 1
#define OS 2
#define IS 3 

typedef struct _gemm {
    int m;
    int n;
    int k;
} gemm;

typedef struct _result {
    std::vector<std::tuple<int, int, int>> computations;
    int weight_fill_cycles;
    int activation_cycles;
    int total_cycles;
    int stall_cycles;
} result;

typedef struct _config {
    int array_w;
    int array_h;
    int dataflow;
    gemm mnk;
    int ifmap_sram_size;
    int ofmap_sram_size;
    int filter_sram_size;
    int off_chip_memory_cycles;
} config;

int read_arch_config(config* config, char *path);
int read_gemm_config(config* config, char *path);