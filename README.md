# TVA: A multi-party computation system for secure and expressive time series analytics
This repository includes the implementation of the TVA multi-party computation (MPC) system for secure time series analytics described in the [USENIX Security'23 paper](https://www.usenix.org/conference/usenixsecurity23/presentation/faisal) by Muhammad Faisal, Jerry Zhang, John Liagouris, Vasiliki Kalavri, and Mayank Varia. TVA currently provides two replicated secret sharing protocols: (i) the semi-honest 3-party protocol by Araki et al. and (ii) the malicious-secure
Fantastic Four protocol by Dalskov et al.

This repository is organized as follows:
- The `./include/` folder contains the core functionality of TVA, including the implementation of MPC primitives, relational oblivious operators, and party communication.
- The `./src/micro` folder contains the implementation of various microbenchmarks and performance experiments.
- The `./src/queries/` folder contains example time-series data queries using our high level API in TVA.
- The `./tests/` folder contains various unit and end-to-end tests.
- The `./scripts/` contains various scripts for automating dependencies setup and experiments benchmarking.

Building TVA (One Node)
============
The following instructions assume a single-node setup. Check `./scripts/setup.sh` for an example setup script.

To build Secrecy, you will need to install:
- Compilers for C++14.
- CMake (>=3.15.0) and pkg-config (>=0.29.2).
- [Libsodium](https://libsodium.gitbook.io/doc/installation) (>=1.0.18).
- an MPI implementation, such as [OpenMPI](https://www.open-mpi.org/software/ompi/v4.0/) or [MPICH](https://www.mpich.org/downloads/).

The previous requirments should be enough for testing the system functionalities using a single machine that has all computing parties. 


Building and running using experiments the `CMakeLists.txt` file:
-------------
- Make sure you have all dependencies installed and then create the build directort:
   ```
   mkdir build
   ```
- To compile and run all test cases:
   ```
   cd ./scripts && ./run_tests.sh
   ```
- To run a specific test file or experiment: 
   - First use cmake to create the build dir and make file.
      ```
      mkdir build && cd ./build
      cmake ..
      ```
   - Build and run one test file:
      ```
      cd build
      make main
      mpirun -np 3 ./main
      ```


Building TVA (Cluster)
============
In order to use the framework in the LAN or WAN settings, we need to replicate the previous steps for each machine in the computing parties cluster. Make sure to use same paths on each machine when cloning and building the repository. Once, we have TVA working on every machine, we can now start building the cluster. For this we need to do the following:
- Make sure that machines have pair wise SSH access to each other (including themselves). This step depends on the cloud service provider as firewall settings and defaults are different. Incoming traffic should be allowed on all machines.
- Modify the `./etc/hosts` file on each machine to include other computing parties with names in the format `machine-i` for LAN and `machine-wan-i` for WAN. Also add an alias for localhost.
- Build the executables on any of the machines using either the semi-honest `semi` or the malicious protocol `mal` on each machine using the following script.
```
cd scripts
./build_experiments.sh semi  # mal
```
- Run single instance of an experiment as follows:
```
exp_name="cloud"
ROWS=1024
cd ./build
mpirun --host machine-1,machine-2,machine-3 -np 3 ./${exp_name} 16 1 8192 $ROWS
```
- OR Run one of the experiments under `./scripts` such as `cloud.sh` depending on which results you need to reproduce which would run the cloud experiment on different sizes up to 2^22 rows and write the time under directory `./results`.
```
cd ./scripts
./cloud.sh semi lan
```


Important Information
============
**NOTICE**: This is an academic proof-of-concept prototype and has not received careful code review. This implementation is NOT ready for production use.


**License**
TVA is distributed under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE (v3.0).
