# Borealis Jobs library and test project

cmake_minimum_required(VERSION 3.19...3.29)

set(BOREALIS_VERSION 0.1.0)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Project information
project(
    BorealisJobs 
    VERSION ${BOREALIS_VERSION}
    LANGUAGES CXX C
)

message("Building ${CMAKE_PROJECT_NAME} v${CMAKE_PROJECT_VERSION}")

link_directories(BorealisJobs/Debug/)

add_subdirectory(BoostrealisJobs)
add_subdirectory(BorealisJobs)
add_subdirectory(BorealisJobsTest)
