cmake_minimum_required(VERSION 3.19...3.29)

set(BOREALIS_VERSION 0.1.0)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()

message("Building BoostrealisJobs library...")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Project source files
set(SOURCES 
src/job-system.cpp
src/scheduler.cpp
)

set(HEADERS
src/config.h
src/job-system.h
src/job.h
src/scoped-spinlock.h
src/spinlock.h
src/scheduler.h
)

message("Setting compiler definitions...")

if(MSVC)
add_compile_definitions(WIN32)
add_compile_definitions(BOREALIS_BUILD_DLL)
elseif(LINUX)
add_compile_definitions(LINUX)
else()
# Anything else

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
add_library(BoostrealisJobs SHARED ${SOURCES} ${HEADERS})

# Libraries
target_link_libraries(BoostrealisJobs PRIVATE Boost::fiber Boost::lockfree)

# Include directories
target_include_directories(BoostrealisJobs PUBLIC ${HEADERS})