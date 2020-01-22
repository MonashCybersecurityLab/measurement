##### Enclave Compile Settings ####
SET(Enclave_Version_Script Enclave/Enclave_debug.lds)

IF(SGX_MODE EQUAL HW)
    SET(Trts_Library_Name sgx_trts)
    SET(Service_Library_Name sgx_tservice)
    IF(SGX_DEBUG EQUAL 0)
        IF(SGX_PRERELEASE EQUAL 0)
            # Use Enclave.lds for hardware release
            SET(Enclave_Version_Script Enclave/Enclave.lds)
        ENDIF()
    ENDIF()
ELSEIF(SGX_MODE EQUAL SIM)
    SET(Trts_Library_Name sgx_trts_sim)
    SET(Service_Library_Name sgx_tservice_sim)
ENDIF()
SET(Crypto_Library_Name sgx_tcrypto)

# Add all cpp files
FILE(GLOB_RECURSE Enclave_Cpp_Files "*.cpp")

# Add all headers
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Include ${PROJECT_SOURCE_DIR}/Enclave ${SGX_SDK}/include ${SGX_SDK}/include/tlibc ${SGX_SDK}/include/libcxx)

# Flag
SET(Enclave_C_Flags "${SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector")
SET(Enclave_Cpp_Flags "${Enclave_C_Flags} -nostdinc++")

SET(Enclave_Security_Link_Flags "-Wl,-z,relro,-z,now,-z,noexecstack")
SET(Enclave_Link_Flags "${Enclave_Security_Link_Flags} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${Trts_Library_Name} -Wl,--no-whole-archive \
    -Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l${Crypto_Library_Name} -l${Service_Library_Name} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
    -Wl,--version-script=${Enclave_Version_Script}")

# Set cmake parameters
SET(CMAKE_C_FLAGS ${Enclave_C_Flags})
SET(CMAKE_CXX_FLAGS ${Enclave_Cpp_Flags})
SET(CMAKE_LINK_LIBRARY_FLAG ${Enclave_Link_Flags})
SET(CMAKE_CXX_STANDARD 14)

add_custom_command(OUTPUT Enclave_t.h Enclave_t.c
        COMMAND  ${SGX_EDGER8R} --trusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path ${SGX_SDK}/include
        COMMENT  "make tcode"
)

ADD_CUSTOM_TARGET(
        tcode ALL
        DEPENDS Enclave_t.h Enclave_t.c
)

add_executable(enclave.so ${Enclave_Cpp_Files} EnclaveUtil.cpp EnclaveUtil.h)
