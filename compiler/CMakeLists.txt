cmake_minimum_required(VERSION 3.16)
project(compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Mirror Makefile: bin/compiler
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(compiler ${SOURCES})

target_include_directories(compiler PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_compile_options(compiler PRIVATE -Wall -Wextra)

enable_testing()

set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

# add_compiler_test(<name> <fixture> <expected-exit> [stderr-regex])
function(add_compiler_test name fixture expected_exit)
  add_test(NAME ${name}
           COMMAND ${TEST_DIR}/run_case.sh $<TARGET_FILE:compiler>
                   ${TEST_DIR}/${fixture} ${expected_exit} ${ARGN})
endfunction()

add_compiler_test(valid_basic test.onnx 0)
add_compiler_test(valid_chain valid_chain.onnx 0)
add_compiler_test(invalid_dup invalid_dup.onnx 1 "duplicate name")
add_compiler_test(invalid_mismatch invalid_mismatch.onnx 1 "size mismatch")
add_compiler_test(invalid_rank invalid_rank.onnx 1 "must have rank")
add_compiler_test(invalid_activation invalid_activation.onnx 1 "unknown activation")
add_compiler_test(invalid_nolayers invalid_nolayers.onnx 1 "has no layers")
add_compiler_test(invalid_op invalid_op.onnx 1 "unsupported op")
add_compiler_test(invalid_range invalid_range.onnx 1 "does not fit fixed-point")
add_compiler_test(missing_file nonexistent.onnx 1 "could not open file")

# add_golden_test(<name> <fixture> <stage>)  -- compares stdout to expected/<name>.txt
function(add_golden_test name fixture stage)
  add_test(NAME ${name}
           COMMAND ${TEST_DIR}/run_golden.sh $<TARGET_FILE:compiler>
                   ${TEST_DIR}/${fixture} ${stage}
                   ${TEST_DIR}/expected/${name}.txt)
endfunction()

add_golden_test(golden_tir_basic test.onnx tir)
add_golden_test(golden_mir_basic test.onnx mir)
add_golden_test(golden_tir_chain valid_chain.onnx tir)
add_golden_test(golden_mir_chain valid_chain.onnx mir)
add_golden_test(golden_pool_quant quant.onnx ast)
