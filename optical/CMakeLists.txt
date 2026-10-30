################################################################################
#   CMakeLists.txt for sithopt: light collection of one SiTH triangular bar   #
################################################################################

cmake_minimum_required(VERSION 3.16)
project(SiTHOpt)

find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(sithopt sithopt.cc ${sources} ${headers})
target_link_libraries(sithopt ${Geant4_LIBRARIES})
