# Systolic Array Simulator 
This is a systolic array simulator to caculate the compuation cycle counts. This code heavily references the following SCALE-SIM paper. This code is written in C++ instead of Python which is used to simulate SCALE-SIM. 

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

./sim test1.config
```
### Write a Config File
This is the example configuration file to run the simulator. You have to close with the ```[end]``` bracket. This simulator can only simulate GEMM (general matrix multiplication) with Weight Stationary(WS). Another calcualtion method will be added in future. 

SRAM size indicates the number of parameters not bytes. If your Ifmap matrix size is ```[128x128]```, ```IfmapSRAMSize``` should be ```16384```. 
```MSize``` also indicates the number of row of first matrix in ```[MxN] x [NxK]```.
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
###









