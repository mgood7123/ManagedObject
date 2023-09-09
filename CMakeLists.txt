cmake_minimum_required(VERSION 3.4.1)

project(ManagedObject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT COMMAND testBuilder_build)
    add_subdirectory(testBuilder)
    testBuilder_set_current_working_directory_to_default_binary_directory()
endif()


testBuilder_add_include(managed_object include)
testBuilder_add_source(managed_object src/managed_object_heap.cpp)
testBuilder_add_source(managed_object src/managed_object.cpp)
testBuilder_build_shared_library(managed_object)

testBuilder_add_include(managed_object_debug include)
testBuilder_add_source(managed_object_debug src/managed_object_heap.cpp)
testBuilder_add_source(managed_object_debug src/managed_object.cpp)
testBuilder_add_compile_option(managed_object_debug "SHELL:-D MANAGED_OBJECT_HEAP_DEBUG=1")
testBuilder_build_shared_library(managed_object_debug)

if (NOT TARGET gtest AND NOT TARGET gtest_main)
    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(googletest)

    # The gtest/gtest_main targets carry header search path
    # dependencies automatically when using CMake 2.8.11 or
    # later. Otherwise we have to add them here ourselves.
    if (CMAKE_VERSION VERSION_LESS 2.8.11)
        include_directories("${gtest_SOURCE_DIR}/include")
    endif()
endif()

add_subdirectory(tests)
