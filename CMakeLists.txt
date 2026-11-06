cmake_minimum_required(VERSION 3.21)
project(CustomShopMaker VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The current C++ files are at the repository root.
add_library(${PROJECT_NAME} SHARED main.cpp ShopData.cpp)
setup_geode_mod(${PROJECT_NAME})
