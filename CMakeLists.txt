cmake_minimum_required(VERSION 3.15)
project(measurement)

# SGX parameters
SET(SGX_SDK /opt/intel/sgxsdk)
SET(SGX_MODE SIM)
SET(SGX_ARCH x64)
SET(SGX_DEBUG 1)
SET(SGX_PRERELEASE 0)
SET(SGX_COMMON_FLAG -m64)
SET(SGX_LIBRARY_PATH ${SGX_SDK}/lib64)
SET(SGX_EDGER8R ${SGX_SDK}/bin/x64/sgx_edger8r)
SET(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x64/sgx_sign)

IF(SGX_DEBUG EQUAL 1)
    IF(SGX_PRERELEASE EQUAL 1)
        MESSAGE( FATAL_ERROR "Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!" )
    ENDIF()
ENDIF()

IF(SGX_DEBUG EQUAL 1)
    SET(SGX_COMMON_FLAG "${SGX_COMMON_FLAG} -O0 -g")
ELSE()
    SET(SGX_COMMON_FLAG "${SGX_COMMON_FLAG} -O2")
ENDIF()

# Flag
SET(SGX_COMMON_FLAG "${SGX_COMMON_FLAG} -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
-Waddress -Wsequence-point -Wformat-security \
-Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
-Wcast-align -Wcast-qual -Wconversion -Wredundant-decls")
set(SGX_COMMON_CFLAG "${SGX_COMMON_FLAG} -Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants")
set(SGX_COMMON_CXXFLAGS "${SGX_COMMON_FLAG} -Wnon-virtual-dtor")

# Load cmake file within subdirs
add_subdirectory(App)
add_subdirectory(Enclave)