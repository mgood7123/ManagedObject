cmake_minimum_required(VERSION 3.4.1)

project(ManagedObject)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT COMMAND testBuilder_build)
    add_subdirectory(testBuilder)
    testBuilder_set_current_working_directory_to_default_binary_directory()
endif()

add_subdirectory(llvm_mcjit)

testBuilder_add_source(gc_stub_boehm stub_bdwgc/alloc.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/reclaim.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/allchblk.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/misc.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/mach_dep.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/os_dep.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/mark_rts.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/headers.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/mark.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/obj_map.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/blacklst.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/finalize.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/new_hblk.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/dbg_mlc.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/malloc.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/dyn_load.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/typd_mlc.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/ptr_chck.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/mallocx.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/gcj_mlc.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/fnlz_mlc.c)
testBuilder_add_source(gc_stub_boehm stub_bdwgc/backgraph.c)
testBuilder_add_compile_option(gc_stub_boehm "SHELL:-DUSE_SEGV_SIGACT")
testBuilder_add_compile_option(gc_boehm "SHELL:-DUSE_SEGV_SIGACT")
testBuilder_build_shared_library(gc_stub_boehm)
target_include_directories(gc_stub_boehm PRIVATE stub_bdwgc/include)

testBuilder_add_include(gc_mps mps_gc/code)
testBuilder_add_include(gc_mps include)
testBuilder_add_source(gc_mps mps_gc/code/mps.c)
testBuilder_add_source(gc_mps src/managed_object_hashmap.c)
testBuilder_add_source(gc_mps src/managed_object_obj.c)
testBuilder_add_library(gc_mps gc_stub_boehm)
testBuilder_add_library(gc_mps llvm_orcjit)

testBuilder_build_shared_library(gc_mps)
# CONFIG_VAR_COOL
# CONFIG_VAR_HOT
# CONFIG_VAR_RASH
target_compile_options(gc_mps PRIVATE "SHELL:-DCONFIG_VAR_RASH")

target_include_directories(gc_mps PRIVATE stub_bdwgc/include)
target_compile_options(gc_stub_boehm PRIVATE "SHELL:-DSTACK_GROWS_DOWN")

#testBuilder_add_include(managed_object include)
#testBuilder_add_source(managed_object src/managed_object_heap.cpp)
#testBuilder_add_source(managed_object src/managed_object.cpp)
#testBuilder_add_library(managed_object gc_mps)
#testBuilder_build_shared_library(managed_object)

testBuilder_add_include(managed_object_debug include)
testBuilder_add_source(managed_object_debug src/managed_object_heap.cpp)
testBuilder_add_source(managed_object_debug src/managed_object.cpp)
testBuilder_add_compile_option(managed_object_debug "SHELL:-D MANAGED_OBJECT_HEAP_DEBUG=1")
testBuilder_add_library(managed_object_debug gc_mps)
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
