cmake_minimum_required(VERSION 3.19...3.29)

set(BOREALIS_VERSION 0.1.0)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()


message("Building BorealisJobs library...")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Project source files
set(SOURCES 
src/job-system.cpp
)

set(HEADERS
src/config.h
src/job-system.h
src/job.h
src/scoped-spinlock.h
src/spinlock.h
)

if(WIN32)
add_compile_definitions(WIN32)
add_compile_definitions(BOREALIS_BUILD_DLL)
elseif(LINUX)
add_compile_definitions(LINUX)
else()
# Anything else

endif()

# Create library
add_library(${CMAKE_PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(BorealisJobs PUBLIC ${HEADERS})
