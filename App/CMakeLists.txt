##### App Compile Settings ####
IF(SGX_MODE EQUAL HW)
    SET(Urts_Library_Name sgx_urts)
ELSEIF(SGX_MODE EQUAL SIM)
    SET(Urts_Library_Name sgx_urts_sim)
ENDIF()

# Add all cpp files
FILE(GLOB_RECURSE App_Cpp_Files "*.cpp")

# Add all headers
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Include ${PROJECT_SOURCE_DIR}/App ${SGX_SDK}/include)

# Flag
SET(App_C_Flags "${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
IF(SGX_DEBUG EQUAL 1)
    SET(App_C_Flags "${App_C_Flags} -DDEBUG -UNDEBUG -UEDEBUG")
ELSEIF(SGX_PRERELEASE EQUAL 1)
    SET(App_C_Flags "${App_C_Flags} -DNDEBUG -DEDEBUG -UDEBUG")
ELSE()
    SET(App_C_Flags "${App_C_Flags} -DNDEBUG -UEDEBUG -UDEBUG")
ENDIF()
SET(App_Cpp_Flags "${App_C_Flags}")

# Link flag
SET(App_Link_Flags "-L${SGX_LIBRARY_PATH} -l${Urts_Library_Name}")
IF(SGX_MODE EQUAL HW)
    SET(App_Link_Flags, "${App_Link_Flags} -lsgx_uae_service")
ELSEIF(SGX_MODE EQUAL SIM)
    SET(App_Link_Flags, "${App_Link_Flags} -lsgx_uae_service_sim")
ENDIF()

# Set cmake parameters
SET(CMAKE_C_FLAGS ${App_C_Flags})
SET(CMAKE_CXX_FLAGS ${App_Cpp_Flags})
SET(CMAKE_LINK_LIBRARY_FLAG ${App_Link_Flags})
SET(CMAKE_CXX_STANDARD 14)

# Build App
ADD_CUSTOM_COMMAND(OUTPUT Enclave_u.h Enclave_u.c
        COMMAND  ${SGX_EDGER8R} --untrusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path ${SGX_SDK}/include
        COMMENT  "make ucode"
)

ADD_CUSTOM_TARGET(
        ucode ALL
        DEPENDS Enclave_u.h Enclave_u.c
)

ADD_EXECUTABLE(App ${App_Cpp_Files} ../Enclave/Enclave.cpp)
