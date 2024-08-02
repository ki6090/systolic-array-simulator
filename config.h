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
} result;

typedef struct _config {
    int array_w;
    int array_h;
    int dataflow;
    gemm mnk;
} config;

void read_gemm_config(config* config, char *path);