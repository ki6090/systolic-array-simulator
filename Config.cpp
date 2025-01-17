#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstddef>
#include <vector>
#include "Config.h"

using namespace std;

string Config::trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

int32_t Config::get(vector<string>* vp, char *target) {
    for (const string& line: *vp) {
        size_t pos = line.find(target);
        if (pos != string::npos) {
            pos = line.find(':');
            string str_val = trim(line.substr(pos + 1));

            if (str_val == (char*)"ws") return (int)WS;
            else if (str_val == (char*)"os") return (int)OS;
            else if (str_val == (char*)"is") return (int)IS;
            else return stoi(str_val);
        }
    }
    return -1;
}

int32_t Config::read_arch(SimulationConfigs* config, char *path) {
    ifstream file;
    file.open(path);
    if (!file.is_open()) {
        cout << "No such file or directory.\n";
        return -1;
    }

    string line;
    vector<string> v_arch;
    bool b_arch = false;
    while (getline(file, line)) {
        string trimmed_line = trim(line);
        if (b_arch && trimmed_line == "[end]")
            break;
        if (trimmed_line == "[architecture_presets]") {
            b_arch = true;
        }
        if (b_arch && trimmed_line != "[architecture_presets]") {
            v_arch.push_back(line);
        }
    }

    int a_h = get(&v_arch, (char*)"ArrayHeight");
    int a_w = get(&v_arch, (char*)"ArrayWidth");
    int dataflow = get(&v_arch, (char*)"Dataflow");
    int ifmap_sram_size = get(&v_arch, (char*)"IfmapSRAMSize");
    int ofmap_sram_size = get(&v_arch, (char*)"OfmapSRAMSize");
    int filter_sram_size = get(&v_arch, (char*)"FilterSRAMSize");
    int off_chip_memory_cycles = get(&v_arch, (char*)"OffChipMemoryCycles");

    if (a_h < 0 || a_w < 0 || dataflow < 0 || ifmap_sram_size <= a_h || ofmap_sram_size <= a_w || filter_sram_size <= a_w || off_chip_memory_cycles < 0) {
        cout << "configuration not found or wrong input.\n";
        return -1;
    }

    config->array_h = a_h;
    config->array_w = a_w;
    config->dataflow = dataflow;
    config->ifmap_sram_size = ifmap_sram_size;
    config->ofmap_sram_size = ofmap_sram_size;
    config->filter_sram_size = filter_sram_size;
    config->off_chip_memory_cycles = off_chip_memory_cycles;
    config->path = path;

    file.close();

    return 0;
}

int32_t Config::read_gemm(SimulationConfigs* config, char *path) {
    ifstream file;
    file.open(path);
    if (!file.is_open()) {
        cout << "No such file or directory.\n";
        return -1;
    }

    string line;
    vector<string> v_arch;
    vector<string> v_gemm;
    bool b_gemm = false;
    while (getline(file, line)) {
        string trimmed_line = trim(line);
        if (b_gemm && trimmed_line == "[end]")
            break;
        if (trimmed_line == "[gemm_mnk]") {
            b_gemm = true;
        }
        if (b_gemm && trimmed_line != "[gemm_mnk]") {
            v_gemm.push_back(line);
        }
    }

    int m = get(&v_gemm, (char*)"MSize");
    int n = get(&v_gemm, (char*)"NSize");
    int k = get(&v_gemm, (char*)"KSize");
    
    if (m < 0 || n < 0 || k < 0) {
        cout << "configuration not found or wrong input.\n";
        return -1;
    }

    config->mnk.m = m;
    config->mnk.n = n;
    config->mnk.k = k;

    file.close();

    return 0;
}

void Config::print_sim_infos(SimulationConfigs *configs) {
    cout << "=========SYSTOLIC-ARRAY-SIM=========\n";
    string dataflow;
    if (configs->dataflow == WS)
        dataflow = "Weight Stationary\n";
    else if (configs->dataflow == OS)
        dataflow = "Output Stationary\n";
    else if (configs->dataflow == IS)
        dataflow = "Input Stationary\n";

    cout << "Array Size: " << configs->array_h << "x" << configs->array_w << '\n';
    cout << "Data Flow: " << dataflow;
    cout << "GEMM Size: " << "[" << configs->mnk.m << "x" << configs->mnk.n << "]" << "x" << "[" << configs->mnk.n << "x" << configs->mnk.k << "]\n";
    cout << "IFMAP SRAM Size: " << configs->ifmap_sram_size << '\n';
    cout << "OFMAP SRAM Size: " << configs->ofmap_sram_size << '\n';
    cout << "Filter SRAM Size: " << configs->filter_sram_size << '\n';
    cout << "Off-Chip Memory Cycles: " << configs->off_chip_memory_cycles << '\n';
    cout << "Config Path: " << "./" << configs->path << '\n';
}