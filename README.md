#OblivSketch
This repository contains the implementation of OblivSketch, an oblivious network measurement service. This service leverages Intel SGX
to protect the sensitive network statistics while providing an efficient network measurement service. It also utilises
oblivious data structures and algorithms to avoid memory access side-channels against Trusted Execution Environments (TEE).
The detailed construction and security proof are presented in our NDSS'21 paper [1].


## Requirements

* Git
* Intel SGX (The SGX simulation mode can be used to run our code, but it does not offer the hardware security feature, and
  the performance gain is not significant because it does not have the paging issue)
* Ubuntu 18.04
* g++-7 (7.5.0 in ubuntu 18.04)
* cmake 3.17
* openssl 1.1.1 (SGXSSL is needed)

## Building

```bash
git clone https://github.com/MonashCybersecurityLab/measurement.git
cd measurement
mkdir build
cd build
# use cmake to build the code
cmake ..
cmake --build . --target [strawman|strawman_HW|oblivious|oblivious_HW|cleanup]
```
The above commands build the strawman and oblivious designs discusssed in the paper.
Note that *_HW requires SGX-enabled hardware to execute.

## Usage
After compiling the project, users can run `app {path_to_traffic}` to start the test program. The test program generates
the sketch from the traffic dump and executes all measurement tasks.

## Feedback

- [Submit an issue](https://github.com/MonashCybersecurityLab/measurement/issues/new)
- Email the authors: shifeng.sun@monash.edu, shangqi.lai@monash.edu, xingliang.yuan@monash.edu
- Note 1: This is a test program relying on the pre-processed traffic. We note that some features are implemented but 
  require further scrutinisation, so they are currently in the `test` branch. We will try to clean it up by the end of Feb. 
- Note 2: The code for OVS integration is deeply coupled with the OVS. I will simplify it and provide the patch file, so 
everyone can easily update the OVS code for the integration purpose.

##Reference
[1] Shangqi Lai, Xingliang Yuan, Joseph K. Liu, Xun Yi, Qi Li, Dongxi Liu, and Surya Nepal. 2021. OblivSketch: Oblivious 
Network Measurement as a Cloud Service. In *the Network and Distributed System Security Symposium* (*NDSS*). DOI:
https://dx.doi.org/10.14722/ndss.2021.24330

