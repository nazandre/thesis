VisibleSim + C2SR implementation
================================

## The Cylindrical Catoms Self-Reconfiguration (C2SR) algorithm

Naz, A., Piranda, B., Bourgeois, J., & Goldstein, S. C. (2016, October). A distributed self-reconfiguration algorithm for cylindrical lattice-based modular robots. In Network Computing and Applications (NCA), 2016 IEEE 15th International Symposium on (pp. 254-263). IEEE.

## How to have a look at C2SR source code?

The C2SR main source code is [here](applicationsSrc/c2sr.cpp).

## How to run C2SR?

1. Compile C2SR and VisibleSim (the procedure is described below).
2. Run C2SR:
```bash
cd applicationsBin/c2sr/
./c2sr -r -c evaluation/confs/car/car-120.xml
```

## How to reproduce the evaluation done for the thesis?

1. Compile C2SR and VisibleSim (the procedure is described below).
2. Run C2SR:
```
cd applicationsBin/c2sr/
./evaluation.sh
./evaluation-rate.sh
```
3. Plot the graphs
```
cd evaluation
./extractAndPlotAll.sh
```

## How to compile VisibleSim and its applications?

Applications are implicitely compiled when VisibleSim is compiled.

### Dependencies:
 - Boost C++ Libraries >= 1.47.0 (If using Meld Process only)
 - GLUT
 - GLEW
 - SBCL - to compile Meld programs
 - MUPARSER - to use CSG

### Ubuntu Installation:

1\. Install dependencies:
```shell
sudo apt-get install libboost-all-dev
sudo apt-get install freeglut3-dev
sudo apt-get install libglew-dev
sudo apt-get install sbcl
sudo apt-get install libmuparser-dev
```
2\. `make`