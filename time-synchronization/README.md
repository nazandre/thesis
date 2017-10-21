VisibleSim + the Modular Robot Time Protocol (MRTP)
================================

This folder provides codes and scripts used for the evaluation of our work on network time synchronization.

In this thesis, we proposed the Modular Robot Time Protocol (MRTP). We compare our protocol to modified version of existing ones:  AD [1], ATS [2], FTSP [3], MLE-TPSN [7] + [4], PulseSync [5,6], TPSN [7], WMTS [8,9]. We modified these protocols to make them fit our target system (see thesis document for more details).

## Associated publications

Naz, A., Piranda, B., Goldstein, S. C., & Bourgeois, J. (2016, February). A time synchronization protocol for modular robots. In Parallel, Distributed, and Network-Based Processing (PDP), 2016 24th Euromicro International Conference on (pp. 109-118). IEEE.

## How to have a look to the source code of our algorithms and the ones used for comparisons?

All the algorithm source codes are available the [applicationsSrc/timeSync](applicationsSrc/timeSync) folder.

Note that protocols to which we compare do not directly fit our target system model. We **STRONGLY** modified some of them but keep the same names. So, please keep in mind that we implemented **STRONGLY MODIFIED VERSIONS OF THE COMPARED PROTOCOLS**. 

Also notice that some protocols are implemented in the file of other protocols using macro to modify some parts of the code.

Here are direct links to the main code of the different protocols:

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
./timeSync -R -k BB -a 12 -c confs/l2d-2/10.xml -P 6 -Z 1 -B 2000000 -S 5000000 -N
```

Parameter possible values:
* Simulation seed: any number, "-1" for a randomly generated seed.
* Protocol id:
  * MRTP = 1
  * TPSN = 2
  * MLE_TPSN = 3
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
./evaluation.sh Ball
```
3. Extract collected data and plot graphs. Plots are generated in the `evaluation/graphs` folder.
```
cd evaluation
./evaluation.sh Ball
```

## How to compile VisibleSim and its applications?

Applications are implicitly compiled when VisibleSim is compiled.

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

## References

[1] Li, Q., and Rus, D. (2006). Global clock synchronization in sensor networks. Computers, IEEE Transactions on, 55(2):214–226.

[2] Schenato, L., and Fiorentin, F. (2011). Average timesynch: A consensus-based protocol for clock synchronization in wireless sensor networks. Automatica, 47(9):1878–1886.

[3] Maróti, M., Kusy, B., Simon, G., and Lédeczi, Á. (2004). The flooding time synchronization protocol. In Proceedings of the 2nd international conference on Embedded networked sensor systems, pages 39–49. ACM.

[4] Leng, M., and Wu, Y.-C. (2010). On clock synchronization algorithms for wireless sensor networks under unknown delay. IEEE Transactions on Vehicular Technology, 59(1):182–109.

[5] Lenzen, C., Sommer, P., and Wattenhofer, R. (2009). Optimal clock synchronization in networks. In Proceedings of the 7th ACM Conference on Embedded Networked Sensor Systems, pages 225–238. ACM.

[6] Lenzen, C., Sommer, P., and Wattenhofer, R. (2015). Pulsesync: An efficient and scalable clock synchronization protocol. IEEE/ACM Transactions on Networking (TON), 23(3):717–727.

[7] Ganeriwal, S., Kumar, R., and Srivastava, M. B. (2003). Timing-sync protocol for sensor networks. In Proceedings of the 1st international conference on Embedded networked sensor systems, pages 138–149. ACM.

[8] He, J., Cheng, P., Shi, L., Chen, J., and Sun, Y. (2014a). Time synchronization in wsns: A maximum-value-based consensus approach. IEEE Transactions on Automatic Control, 59(3):660–675.

[9] He, J., Li, H., Chen, J., and Cheng, P. (2014b). Study of consensus-based time synchronization in wireless sensor networks. 53(2):347–357.