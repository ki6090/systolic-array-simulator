#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstddef>
#include <vector>
#include "config.h"

using namespace std;

static string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

static int get_config(vector<string>* vp, char *target) {
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

int read_gemm_config(config* config, char *path) {
    ifstream file;
    file.open(path);
    if (!file.is_open()) {
        cout << "No such file or directory.\n";
        return -1;
    }

    string line;
    vector<string> v_arch;
    vector<string> v_gemm;
    bool b_arch = false;
    bool b_gemm = false;
    while (getline(file, line)) {
        string trimmed_line = trim(line);
        if (trimmed_line == "[architecture_presets]") {
            b_arch = true; b_gemm = false;
        }
        else if (trimmed_line == "[gemm_mnk]") {
            b_arch = false; b_gemm = true;
        }

        if (b_arch && trimmed_line != "[architecture_presets]") {
            v_arch.push_back(line);
        }
        else if (b_gemm && trimmed_line != "[gemm_mnk]") {
            v_gemm.push_back(line);
        }
    }

    int a_h = get_config(&v_arch, (char*)"ArrayHeight");
    int a_w = get_config(&v_arch, (char*)"ArrayWidth");
    int dataflow = get_config(&v_arch, (char*)"Dataflow");

    if (a_h < 0 || a_w < 0 || dataflow < 0) {
        cout << "configuration not found or wrong input.\n";
        return -1;
    }

    config->array_h = a_h;
    config->array_w = a_w;
    config->dataflow = dataflow;

    int m = get_config(&v_gemm, (char*)"MSize");
    int n = get_config(&v_gemm, (char*)"NSize");
    int k = get_config(&v_gemm, (char*)"KSize");
    
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