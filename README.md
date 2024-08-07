# Systolic Array Simulator 
This is a systolic array simulator to caculate the compuation cycle counts. 
This code heavily references the following SCALE-SIM paper. However, this code is significantly different from the SCALE-SIM code. This code is written in C++ instead of Python which is used to simulate SCALE-SIM. 

original paper url: https://arxiv.org/abs/1811.02883

original github repo: https://github.com/ARM-software/SCALE-Sim

----
## Getting Started
### Clone
To run this simulator, clone it to your machine. Linux is highly recommended for operating system. 

```
git clone https://github.com/ki6090/systolic-array-simulator.git
```
### Compile and Run
To run the real simulator, you have to complie with corresponding command in root repository folder. 
```
make sim
```

```
./sim configs/test1.config
```
### Write a Config File
This is the example configuration file to run the simulator. You have to close with the ```[end]``` bracket. This simulator can only simulate GEMM (general matrix multiplication) with Weight Stationary(WS). Another calcualtion method will be added in future. 

SRAM size indicates the number of parameters not bytes. If your Ifmap matrix size is ```[128x128]```, ```IfmapSRAMSize``` should be ```16384```. 
```MSize``` also indicates the number of row of first matrix in ```[MxN] x [NxK]```.

Also, you can set the stall cycles to access off-chip DRAM with ```OffChipMemoryCycles```.
```
[architecture_presets]
ArrayHeight: 16
ArrayWidth: 16
Dataflow: ws
IfmapSRAMSize: 16384
OfmapSRAMSize: 16384
FilterSRAMSize: 16384
OffChipMemoryCycles: 500
[end]
[gemm_mnk]
MSize: 128
NSize: 128
KSize: 128
[end]
```

## Example Results 
You can see the simulation result after running the prior commands. The cycle results is shown for each computation in the systolic array. 
```
=========SYSTOLIC-ARRAY-SIM=========
Array Size: 128x128
Data Flow: Weight Stationary
GEMM Size: [256x256]x[256x128]
IFMAP SRAM Size: 16384
OFMAP SRAM Size: 16384
Filter SRAM Size: 16384
Off-Chip Memory Cycles: 500
Config Path: ./configs/test8.config

============COMPUTATIONS============
Computation1: [128x128]x[128x128]
Computation2: [128x128]x[128x128]
Computation3: [128x128]x[128x128]
Computation4: [128x128]x[128x128]

============CYCLE-COUNT=============
------------COMPUTATION1------------
Weight Filling: 1~128(128cycles)
Activations: 129~511(383cycles)
------------COMPUTATION2------------
Weight Filling: 512~1139(128cycles)
 *Stalls(Filter): 512~1011(500cycles)
Stalls(Ofmap): 1640~2139(500cycles)
Activations: 1640~2022(383cycles)
------------COMPUTATION3------------
Weight Filling: 2023~2650(128cycles)
 *Stalls(Filter): 2023~2522(500cycles)
Stalls(Ofmap): 3151~3650(500cycles)
Activations: 3151~3533(383cycles)
------------COMPUTATION4------------
Weight Filling: 3534~4161(128cycles)
 *Stalls(Filter): 3534~4033(500cycles)
Stalls(Ofmap): 4662~5161(500cycles)
Activations: 4662~5044(383cycles)
--------------RESULTS---------------
Total Weight Filling Cycles: 512
Total Activation Cycles: 1532
Total Stall Cycles: 3000
Computation Cycles: 5044

============UTILIZATION=============
Overall Utilization: 10.1507%
```











