cmake_minimum_required(VERSION 3.15)

project(TVA VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3 -pthread ${PROTOCOL_VAR}")

find_package(Threads REQUIRED)

# Adding MPI Library
find_package(MPI REQUIRED)
if (MPI_FOUND)
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
else (MPI_FOUND)
    message(SEND_ERROR "This application cannot compile without MPI")
endif (MPI_FOUND)

# Using libsodium
find_package(PkgConfig REQUIRED)
pkg_check_modules(_LIBSODIUM REQUIRED libsodium)

find_path(SODIUM_INCLUDE_DIR sodium.h HINTS ${LIBSODIUM_INCLUDE_DIRS} /usr/local/include /opt/local/include /opt/include)
find_library(SODIUM_LIBRARY NAMES sodium HINTS ${LIBSODIUM_LIBRARY_DIRS} /usr/local/lib /opt/local/lib /opt/lib)


######################################################
##################### EXECUTABLES ####################
######################################################
# Targets (main|benchmark_memory|benchmark_mpi)
foreach (_target main)
    add_executable(${_target} "src/${_target}.cpp")
    target_link_libraries(${_target} ${MPI_LIBRARIES} ${SODIUM_LIBRARY})
    target_include_directories(${_target} PUBLIC ${SODIUM_INCLUDE_DIR})
endforeach ()

file(GLOB TEST_EXECUTABLES src/micro/*.cpp)
foreach (_target ${TEST_EXECUTABLES})
    get_filename_component(fileName ${_target} NAME_WLE)
    add_executable(${fileName} ${_target})
    target_link_libraries(${fileName} ${MPI_LIBRARIES} ${SODIUM_LIBRARY})
    target_include_directories(${fileName} PUBLIC ${SODIUM_INCLUDE_DIR})
endforeach ()

file(GLOB TEST_EXECUTABLES src/queries/*.cpp)
foreach (_target ${TEST_EXECUTABLES})
    get_filename_component(fileName ${_target} NAME_WLE)
    add_executable(${fileName} ${_target})
    target_link_libraries(${fileName} ${MPI_LIBRARIES} ${SODIUM_LIBRARY})
    target_include_directories(${fileName} PUBLIC ${SODIUM_INCLUDE_DIR})
endforeach ()

file(GLOB TEST_EXECUTABLES tests/*.cpp)
foreach (_target ${TEST_EXECUTABLES})
    get_filename_component(fileName ${_target} NAME_WLE)
    add_executable(${fileName} ${_target})
    target_link_libraries(${fileName} ${MPI_LIBRARIES} ${SODIUM_LIBRARY})
    target_include_directories(${fileName} PUBLIC ${SODIUM_INCLUDE_DIR})
endforeach ()