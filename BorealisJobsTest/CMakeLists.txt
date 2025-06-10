cmake_minimum_required(VERSION 3.19...3.29)

set(BOREALIS_VERSION 0.1.0)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project information
project(
    BorealisJobsTest
    VERSION 0.1.0
    LANGUAGES CXX C
)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)


# Project source files
set(SOURCES 
    src/test_jobs.cpp
)

# Set platform preprocessor info  
if(WIN32)
    message("Configuring tests for Windows platform")
    add_compile_definitions(BOREALIS_WIN)
elseif(APPLE)
    message("Configuring tests for MacOS platform")
    add_compile_definitions(BOREALIS_OSX)
elseif(UNIX)
    message("Configuring tests for Linux platform")
    add_compile_definitions(BOREALIS_LINUX)
endif()


message("Building test target for BorealisJobs...")

add_executable(BorealisJobsTest ${SOURCES})

# Define preprocessor macros for build configurations
target_compile_definitions(BorealisJobsTest PRIVATE
        $<$<CONFIG:Debug>:BOREALIS_DEBUG>
        $<$<CONFIG:Release>:BOREALIS_RELEASE>
        $<$<CONFIG:RelWithDebInfo>:BOREALIS_RELWITHDEBINFO>
        $<$<CONFIG:MinSizeRel>:BOREALIS_MINSIZEREL>
)


if(WIN32)

#Copy necessary dll to output directory
add_custom_command(
    TARGET BorealisJobsTest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:BorealisJobsTest>
        $<TARGET_FILE_DIR:BorealisJobsTest>
    COMMENT "Copying DLLs to output directory..."
    COMMAND_EXPAND_LISTS
)

elseif(UNIX OR APPLE)

# Set path for unix and osx to point to lib output dir
set_target_properties(BorealisJobsTest PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}/../src"
)

endif()

# Link libraries
target_link_libraries(BorealisJobsTest PRIVATE 
    BorealisJobsLib
    GTest::gtest_main
)

# Include directories
#target_include_directories(BorealisJobsTests PRIVATE 
#    ${CMAKE_SOURCE_DIR}/src
#)

# Tests
include(GoogleTest)
gtest_discover_tests(BorealisJobsTest)