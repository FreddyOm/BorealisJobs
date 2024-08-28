cmake_minimum_required(VERSION 3.19...3.29)

set(BOREALIS_VERSION 0.1.0)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

unset(SOURCES)
unset(HEADERS)

# Project source files
set(SOURCES 
    src/main.cpp
)

# Set platform preprocessor info  
if(WIN32)
    add_compile_definitions(WIN32)
else()
    # Anything else
endif()

message("Building test target for BorealisJobs...")

add_executable(BorealisJobsTest EXCLUDE_FROM_ALL ${SOURCES})

# target_include_directories(BorealisJobsTest PRIVATE ) # Not needed here since there is no header file yet
target_link_libraries(BorealisJobsTest PRIVATE BorealisJobs)