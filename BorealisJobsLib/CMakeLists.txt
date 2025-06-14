cmake_minimum_required(VERSION 3.19...3.29)

set(BOREALIS_VERSION 0.1.0)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()


message("Building BorealisJobsLib ...")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project information
project(
    BorealisJobsLib
    VERSION 0.1.0
    LANGUAGES CXX C
)

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

# Set platform preprocessor info  
if(WIN32)
    message("Configuring tests for Windows platform")
    add_compile_definitions(BOREALIS_BUILD_DLL)
    add_compile_definitions(BOREALIS_WIN)
elseif(APPLE)
    message("Configuring tests for MacOS platform")
    add_compile_definitions(BOREALIS_OSX)
elseif(UNIX)
    message("Configuring tests for Linux platform")
    add_compile_definitions(BOREALIS_LINUX)
endif()

message("Fetching dependencies...")

# Fetch Boost library
set(BOOST_INCLUDE_LIBRARIES fiber lockfree)
set(BOOST_ENABLE_CMAKE ON)

include(FetchContent)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.86.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Boost)

message("Create library...")

# Create library
add_library(BorealisJobsLib SHARED ${SOURCES} ${HEADERS})

target_compile_definitions(BorealisJobsLib PRIVATE
        $<$<CONFIG:Debug>:BOREALIS_DEBUG>
        $<$<CONFIG:Release>:BOREALIS_RELEASE>
        $<$<CONFIG:RelWithDebInfo>:BOREALIS_RELWITHDEBINFO>
        $<$<CONFIG:MinSizeRel>:BOREALIS_MINSIZEREL>
)

# Libraries
target_link_libraries(BorealisJobsLib PRIVATE Boost::fiber Boost::lockfree)

# Include directories
target_include_directories(BorealisJobsLib PUBLIC ${HEADERS})
