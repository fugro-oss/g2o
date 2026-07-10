#pragma once

// C API exported by the fugro_g2o native library. This is the boundary
// between the managed Fugro.G2O assembly (P/Invoke) and the g2o C++ code.
// All callbacks are invoked on the thread that calls fugro_g2o_graph_optimize.

#include <stdint.h>

#if defined(_WIN32)
#define FUGRO_G2O_EXPORT __declspec(dllexport)
#else
#define FUGRO_G2O_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Stack operations, mirrors the managed backup stack of a state estimate.
#define FUGRO_G2O_STACK_PUSH 0
#define FUGRO_G2O_STACK_POP 1
#define FUGRO_G2O_STACK_DISCARD_TOP 2

// Applies a full update vector (length = vertex dimension) to the estimate.
typedef void (*fugro_g2o_oplus_cb)(void* user, const double* update);

// Applies +/- jacobianDelta on a single component of the estimate.
typedef void (*fugro_g2o_jacobian_step_cb)(void* user, int32_t index, int32_t sign);

// Push/pop/discard the estimate backup stack.
typedef void (*fugro_g2o_stack_op_cb)(void* user, int32_t op);

typedef int32_t (*fugro_g2o_stack_size_cb)(void* user);

// Fills error (length = edge dimension). Return 0 on success; any other
// value aborts the optimization (used to surface managed exceptions).
typedef int32_t (*fugro_g2o_compute_error_cb)(void* user, double* error, int32_t dimension);

// Robustifier: maps squared error to (rho0, rho1).
typedef void (*fugro_g2o_robustify_cb)(void* user, double squared_error, double* rho0, double* rho1);

// Called before/after each iteration. Return non-zero to stop optimizing.
typedef int32_t (*fugro_g2o_iteration_cb)(void* user, int32_t iteration);

// ----- graph -----

FUGRO_G2O_EXPORT void* fugro_g2o_graph_create(void* user, fugro_g2o_iteration_cb pre_iteration, fugro_g2o_iteration_cb post_iteration);
FUGRO_G2O_EXPORT void fugro_g2o_graph_destroy(void* graph);

// Returns 0 on success, 1 when the vertex belongs to another graph, 2 on failure.
FUGRO_G2O_EXPORT int32_t fugro_g2o_graph_add_vertex(void* graph, void* vertex);

// Returns 0 on success, non-zero on failure.
FUGRO_G2O_EXPORT int32_t fugro_g2o_graph_add_edge(void* graph, void* edge);

FUGRO_G2O_EXPORT int32_t fugro_g2o_graph_optimize(void* graph, int32_t max_iterations);
FUGRO_G2O_EXPORT void fugro_g2o_graph_request_stop(void* graph);
FUGRO_G2O_EXPORT double fugro_g2o_graph_chi2(void* graph);

// ----- vertex -----

FUGRO_G2O_EXPORT void* fugro_g2o_vertex_create(
    int32_t dimension,
    void* user,
    fugro_g2o_oplus_cb oplus,
    fugro_g2o_jacobian_step_cb jacobian_step,
    fugro_g2o_stack_op_cb stack_op,
    fugro_g2o_stack_size_cb stack_size);
FUGRO_G2O_EXPORT void fugro_g2o_vertex_destroy(void* vertex);
FUGRO_G2O_EXPORT int32_t fugro_g2o_vertex_get_fixed(void* vertex);
FUGRO_G2O_EXPORT void fugro_g2o_vertex_set_fixed(void* vertex, int32_t fixed);
FUGRO_G2O_EXPORT void* fugro_g2o_vertex_get_graph(void* vertex);

// Copies the dimension x dimension hessian block. Returns 0 when the
// hessian has not been mapped yet (graph not optimized), 1 on success.
FUGRO_G2O_EXPORT int32_t fugro_g2o_vertex_copy_hessian(void* vertex, double* destination);

// ----- edge -----

FUGRO_G2O_EXPORT void* fugro_g2o_edge_create(
    int32_t dimension,
    const double* information_row_major,
    int32_t vertex_count,
    void* const* vertices,
    void* user,
    fugro_g2o_compute_error_cb compute_error,
    fugro_g2o_robustify_cb robustify);
FUGRO_G2O_EXPORT void fugro_g2o_edge_destroy(void* edge);
FUGRO_G2O_EXPORT double fugro_g2o_edge_chi2(void* edge);

FUGRO_G2O_EXPORT double fugro_g2o_jacobian_delta(void);

#ifdef __cplusplus
}
#endif
