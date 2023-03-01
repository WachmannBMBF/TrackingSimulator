cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
project(chase-simulator)

set(Boost_INCLUDE_DIR "/usr/include")
find_package(Boost 1.71 REQUIRED program_options)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

if (DEFINED EMSCRIPTEN)
    add_link_options("SHELL:-s \"EXPORTED_RUNTIME_METHODS=ccall,cwrap\"")
    add_compile_options("-pthread")
    add_link_options("SHELL:-s \"USE_PTHREADS=1\"")
    add_link_options("SHELL:-s \"PTHREAD_POOL_SIZE=navigator.hardwareConcurrency\"")
    add_link_options("SHELL:-s \"INITIAL_MEMORY=134217728\"") # 100 MB
endif ()

file(GLOB SRC_FILES *.cpp)
set(CHASIMULATOR_FILES ${SRC_FILES})
list(REMOVE_ITEM CHASIMULATOR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/evaluator.cpp)
add_executable(chasimulator ${CHASIMULATOR_FILES})

if (DEFINED EMSCRIPTEN)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-gsource-map) # Generates emscripten C++ sourcemaps
        add_definitions(-O1)
        target_compile_definitions(chasimulator PRIVATE DEBUG=1)
    else ()
        add_definitions(-Oz)
    endif ()
    target_compile_definitions(chasimulator PRIVATE EMSCRIPTEN=1)
else ()
    set(EVALUATOR_FILES ${SRC_FILES})
    list(REMOVE_ITEM EVALUATOR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/chasimulator.cpp)
    add_executable(evaluator ${EVALUATOR_FILES})
    target_link_libraries(evaluator ${Boost_LIBRARIES})
endif ()


target_link_libraries(chasimulator ${Boost_GRAPH_LIBRARY})


