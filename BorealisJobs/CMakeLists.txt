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

# Fetch Boost library
set(BOOST_INCLUDE_LIBRARIES fiber)
set(BOOST_ENABLE_CMAKE ON)

include(FetchContent)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.86.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Boost)

# Create library
add_library(${CMAKE_PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Boost::fiber)

# Include directories
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${HEADERS})