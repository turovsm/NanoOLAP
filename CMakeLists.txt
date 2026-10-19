cmake_minimum_required(VERSION 3.20)
project(NanoOLAP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target 1: Main Application
add_executable(nano_olap
    main.cpp
    data_store.cpp
    analytics_engine.cpp
)

# Target 2: Test Suite
add_executable(nano_olap_tests
    test_main.cpp
    data_store.cpp
    analytics_engine.cpp
)
