cmake_minimum_required(VERSION 3.16)
project(fugro_g2o CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External)

file(GLOB CSPARSE_SOURCES ${EXTERNAL_DIR}/external/csparse/cs_*.c)

set(G2O_CORE_SOURCES
  ${EXTERNAL_DIR}/g2o/core/batch_stats.cpp
  ${EXTERNAL_DIR}/g2o/core/cache.cpp
  ${EXTERNAL_DIR}/g2o/core/estimate_propagator.cpp
  ${EXTERNAL_DIR}/g2o/core/factory.cpp
  ${EXTERNAL_DIR}/g2o/core/hyper_dijkstra.cpp
  ${EXTERNAL_DIR}/g2o/core/hyper_graph.cpp
  ${EXTERNAL_DIR}/g2o/core/hyper_graph_action.cpp
  ${EXTERNAL_DIR}/g2o/core/jacobian_workspace.cpp
  ${EXTERNAL_DIR}/g2o/core/marginal_covariance_cholesky.cpp
  ${EXTERNAL_DIR}/g2o/core/matrix_structure.cpp
  ${EXTERNAL_DIR}/g2o/core/optimizable_graph.cpp
  ${EXTERNAL_DIR}/g2o/core/optimization_algorithm.cpp
  ${EXTERNAL_DIR}/g2o/core/optimization_algorithm_dogleg.cpp
  ${EXTERNAL_DIR}/g2o/core/optimization_algorithm_factory.cpp
  ${EXTERNAL_DIR}/g2o/core/optimization_algorithm_gauss_newton.cpp
  ${EXTERNAL_DIR}/g2o/core/optimization_algorithm_levenberg.cpp
  ${EXTERNAL_DIR}/g2o/core/optimization_algorithm_with_hessian.cpp
  ${EXTERNAL_DIR}/g2o/core/parameter.cpp
  ${EXTERNAL_DIR}/g2o/core/parameter_container.cpp
  ${EXTERNAL_DIR}/g2o/core/robust_kernel.cpp
  ${EXTERNAL_DIR}/g2o/core/robust_kernel_factory.cpp
  ${EXTERNAL_DIR}/g2o/core/robust_kernel_impl.cpp
  ${EXTERNAL_DIR}/g2o/core/solver.cpp
  ${EXTERNAL_DIR}/g2o/core/sparse_optimizer.cpp
  ${EXTERNAL_DIR}/g2o/core/sparse_optimizer_terminate_action.cpp
)

set(G2O_STUFF_SOURCES
  ${EXTERNAL_DIR}/g2o/stuff/command_args.cpp
  ${EXTERNAL_DIR}/g2o/stuff/filesys_tools.cpp
  ${EXTERNAL_DIR}/g2o/stuff/os_specific.c
  ${EXTERNAL_DIR}/g2o/stuff/property.cpp
  ${EXTERNAL_DIR}/g2o/stuff/sampler.cpp
  ${EXTERNAL_DIR}/g2o/stuff/sparse_helper.cpp
  ${EXTERNAL_DIR}/g2o/stuff/string_tools.cpp
  ${EXTERNAL_DIR}/g2o/stuff/tictoc.cpp
  ${EXTERNAL_DIR}/g2o/stuff/timeutil.cpp
)

set(G2O_SOLVER_SOURCES
  ${EXTERNAL_DIR}/g2o/solvers/csparse/csparse_helper.cpp
)

set(SHIM_SOURCES
  Native/api.cpp
  Native/edge.cpp
  Native/graph.cpp
  Native/vertex.cpp
)

add_library(fugro_g2o SHARED
  ${CSPARSE_SOURCES}
  ${G2O_CORE_SOURCES}
  ${G2O_STUFF_SOURCES}
  ${G2O_SOLVER_SOURCES}
  ${SHIM_SOURCES}
)

target_include_directories(fugro_g2o PRIVATE
  ${EXTERNAL_DIR}
  ${EXTERNAL_DIR}/external
  ${EXTERNAL_DIR}/external/csparse
  Native
)

if(MSVC)
  target_compile_definitions(fugro_g2o PRIVATE
    WIN32 _WINDOWS WINDOWS _WITHOUTLAPACK
    _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS NOMINMAX)
  target_compile_options(fugro_g2o PRIVATE /wd4267 /wd4244 /wd4996)
else()
  target_compile_options(fugro_g2o PRIVATE -Wno-deprecated-declarations)
  set_target_properties(fugro_g2o PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden)
endif()

# Flatten per-configuration output dirs (VS generators) so the .NET projects
# can always find the binary at build/bin.
set_target_properties(fugro_g2o PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin>"
  LIBRARY_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin>"
)
