#include "fugro_g2o_api.h"
#include "graph.h"
#include "vertex.h"
#include "edge.h"

#include "g2o/core/robust_kernel.h"

#include <atomic>
#include <cstring>

namespace fugro
{
  namespace g2o_native
  {
    class CallbackRobustKernel : public g2o::RobustKernel
    {
    private:
      void* _user;
      fugro_g2o_robustify_cb _robustify;

    public:
      CallbackRobustKernel(void* user, fugro_g2o_robustify_cb robustify) :
        _user(user),
        _robustify(robustify)
      {
      }

      void robustify(double squaredError, Eigen::Vector3d& rho) const
      {
        rho[2] = 0.0;
        _robustify(_user, squaredError, &rho[0], &rho[1]);
      }
    };

    // Pairs an edge with the robust kernel it owns, so a single destroy call
    // releases both. The handle passed across the API is the EdgeHandle.
    struct EdgeHandle
    {
      Edge* edge;
      CallbackRobustKernel* kernel;
    };
  }
}

using namespace fugro::g2o_native;

void* fugro_g2o_graph_create(void* user, fugro_g2o_iteration_cb pre_iteration, fugro_g2o_iteration_cb post_iteration)
{
  return new Graph(user, pre_iteration, post_iteration);
}

void fugro_g2o_graph_destroy(void* graph)
{
  delete static_cast<Graph*>(graph);
}

int32_t fugro_g2o_graph_add_vertex(void* graph, void* vertex)
{
  Graph* g = static_cast<Graph*>(graph);
  Vertex* v = static_cast<Vertex*>(vertex);

  if (v->graph() != nullptr && v->graph() != static_cast<g2o::OptimizableGraph*>(g))
  {
    return 1;
  }

  return g->addVertex(v) ? 0 : 2;
}

int32_t fugro_g2o_graph_add_edge(void* graph, void* edge)
{
  Graph* g = static_cast<Graph*>(graph);
  EdgeHandle* handle = static_cast<EdgeHandle*>(edge);

  return g->addEdge(handle->edge) ? 0 : 1;
}

int32_t fugro_g2o_graph_optimize(void* graph, int32_t max_iterations)
{
  return static_cast<Graph*>(graph)->optimizeGraph(max_iterations);
}

void fugro_g2o_graph_request_stop(void* graph)
{
  static_cast<Graph*>(graph)->requestStop();
}

double fugro_g2o_graph_chi2(void* graph)
{
  return static_cast<Graph*>(graph)->activeRobustChi2();
}

void* fugro_g2o_vertex_create(
    int32_t dimension,
    void* user,
    fugro_g2o_oplus_cb oplus,
    fugro_g2o_jacobian_step_cb jacobian_step,
    fugro_g2o_stack_op_cb stack_op,
    fugro_g2o_stack_size_cb stack_size)
{
  static std::atomic<int> counter(0);

  VertexCallbacks callbacks;
  callbacks.user = user;
  callbacks.oplus = oplus;
  callbacks.jacobianStep = jacobian_step;
  callbacks.stackOp = stack_op;
  callbacks.stackSize = stack_size;

  Vertex* vertex = new Vertex(dimension, callbacks);
  vertex->setId(++counter);
  vertex->updateCache();

  return vertex;
}

void fugro_g2o_vertex_destroy(void* vertex)
{
  delete static_cast<Vertex*>(vertex);
}

int32_t fugro_g2o_vertex_get_fixed(void* vertex)
{
  return static_cast<Vertex*>(vertex)->fixed() ? 1 : 0;
}

void fugro_g2o_vertex_set_fixed(void* vertex, int32_t fixed)
{
  static_cast<Vertex*>(vertex)->setFixed(fixed != 0);
}

void* fugro_g2o_vertex_get_graph(void* vertex)
{
  return static_cast<Vertex*>(vertex)->graph();
}

int32_t fugro_g2o_vertex_copy_hessian(void* vertex, double* destination)
{
  Vertex* v = static_cast<Vertex*>(vertex);

  const double* data = v->hessianData();
  if (data == nullptr)
  {
    return 0;
  }

  memcpy(destination, data, (size_t)v->dimension() * v->dimension() * sizeof(double));

  return 1;
}

void* fugro_g2o_edge_create(
    int32_t dimension,
    const double* information_row_major,
    int32_t vertex_count,
    void* const* vertices,
    void* user,
    fugro_g2o_compute_error_cb compute_error,
    fugro_g2o_robustify_cb robustify)
{
  Eigen::MatrixXd information(dimension, dimension);
  for (int32_t i = 0; i < dimension; i++)
  {
    for (int32_t j = 0; j < dimension; j++)
    {
      information(i, j) = information_row_major[dimension * i + j];
    }
  }

  EdgeCallbacks callbacks;
  callbacks.user = user;
  callbacks.computeError = compute_error;
  callbacks.robustify = robustify;

  EdgeHandle* handle = new EdgeHandle();
  handle->edge = new Edge(information, callbacks);
  handle->edge->resize(vertex_count);

  handle->kernel = new CallbackRobustKernel(user, robustify);
  handle->edge->setRobustKernel(handle->kernel);

  for (int32_t i = 0; i < vertex_count; i++)
  {
    handle->edge->setVertex(i, static_cast<Vertex*>(vertices[i]));
  }

  return handle;
}

void fugro_g2o_edge_destroy(void* edge)
{
  EdgeHandle* handle = static_cast<EdgeHandle*>(edge);
  delete handle->edge;
  delete handle->kernel;
  delete handle;
}

double fugro_g2o_edge_chi2(void* edge)
{
  return static_cast<EdgeHandle*>(edge)->edge->chi2();
}

double fugro_g2o_jacobian_delta(void)
{
  return Vertex::jacobianDelta();
}
