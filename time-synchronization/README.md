VisibleSim + the Modular Robot Time Protocol (MRTP)
================================

This folder provides codes and scripts used for the evaluation of our work on network time synchronization.

In this thesis, we proposed the Modular Robot Time Protocol (MRTP). We compare our protocol to existing one.

## Associated publications

Naz, A., Piranda, B., Goldstein, S. C., & Bourgeois, J. (2016, February). A time synchronization protocol for modular robots. In Parallel, Distributed, and Network-Based Processing (PDP), 2016 24th Euromicro International Conference on (pp. 109-118). IEEE.

## How to have a look to the source code of our algorithms and the ones used for comparisons?

All the algorithm source codes are available the [applicationsSrc/timeSync](applicationsSrc/timeSync) folder.

Note that protocols to which we compare do not directly fit our target systems. We STRONGLY modified some of them but keep the same names. Please keep in mind that we implement MODIFIED VERSIONS OF THE COMPARED PROTOCOLS. 

Also notice that some protocols are implemented in the file of other protocols using macro to modify some parts of the code.

Here are links to the different main code of the protocols:

* [MRTP](applicationsSrc/timeSync/mrtp/mrtp.cpp)
* [FTSP](applicationsSrc/timeSync/ftsp/ftsp.cpp)
* [ATS](applicationsSrc/timeSync/ats/ats.cpp)
* [AD](applicationsSrc/timeSync/ad/ad.cpp)
* [PulseSync](applicationsSrc/timeSync/ftsp/ftsp.cpp)
* [TPSN](applicationsSrc/timeSync/mrtp/mrtp.cpp)
* [TPSN-MLE](applicationsSrc/timeSync/mrtp/mrtp.cpp)
* [WMTS](applicationsSrc/timeSync/wmts/wmts.cpp)

## How to run the algorithms?

Note than the protocols accomplish time synchronization but nothing else, so simulation runs does not display anithing else than the robotic system.

1. Compile the protocols and VisibleSim (the procedure is described below).
2. Run an algorithm
```
./timeSync -R -k BB -i -a [simulation seed] -c [xml configuration file] -P [protocol id] -Z [communication model] -B [protocol start date (us)] -S [runtime synchronization period (us)]

#options:
# -N: simulate a network load (trafic in queues using Poisson law, see thesis doc.)
# -L [size]: enable linear model computation with a windows of 'size' synchronization points
# -F: clocks can not be set backward
# -Q [period (us)]: calibration period (if any)
# -M [id] [time (us)]: time master and election date. Mandatory for MRTP and TPSN

# Possible parameters values are listed below.

# For instance:

## MRTP:
./timeSync -R -k BB -a 12 -c confs/l2d-2/10.xml -P 1 -Z 1 -B 2000000 -S 5000000 -N -L 5 -F -Q 2000000 -M 1 1000000

## FTSP (implicitely elected time master):
./timeSync -R -k BB -a 12 -c confs/l2d-2/10.xml -P 4 -Z 1 -B 2000000 -S 5000000 -N -L 5

## ATS (no time master):
/centrality -R -k BB -a 12 -c evaluation/confs/confs/bb/r/100/106.xml -A 10 -V 1
./timeSync -R -k BB -a 12 -c confs/l2d-2/10.xml -P 6 -Z 1 -B 2000000 -S 5000000 -N
```

Parameter possible values:
* Simulation seed: any number, "-1" for a randomly generated seed
* Protocol id:
  * MRTP = 1
  * TPSN = 2
  * TPSN_MLE = 3
  * FTSP = 4
  * AD = 5
  * ATS = 6
  * WMTS = 7
  * PULSE_SYNC = 10
* Communication model:
  * Sparse = 0
  * Intermediate = 1
  * Compact = 2
  
## How to reproduce the evaluation done for the thesis?

1. Compile the algorithms and VisibleSim (the procedure is described below).
2. Launch the evaluation script.
```
cd applicationsBin/timeSync/
./evaluation.sh
```
3. Extract collected data and plot graphs. Plots are generated in the 'evaluation/graphs' folder.
```
cd evaluation
./evaluation.sh
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