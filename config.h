#define WS 1
#define OS 2
#define IS 3 

typedef struct _gemm {
    int m;
    int n;
    int k;
} gemm;

typedef struct _config {
    int array_w;
    int array_h;
    int dataflow;
    gemm mnk;
} config;

void read_config_file_gemm(config* config, char *path);