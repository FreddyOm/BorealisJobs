cmake_minimum_required(VERSION 3.19...3.29)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()

# Project information
project(
    BorealisJobs 
    VERSION 0.1.0
    LANGUAGES CXX C
)

message("Building ${CMAKE_PROJECT_NAME} v${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Project source files
set(SOURCES 
    src/main.cpp 
    src/job-system.cpp
)

set(HEADERS
    src/config.h
    src/job-system.h
    src/job.h
    src/scoped-spinlock.h
    src/spinlock.h
)

# Create executable
add_library(${CMAKE_PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SOURCES} ${HEADERS})