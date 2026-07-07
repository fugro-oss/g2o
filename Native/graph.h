#pragma once

#include "fugro_g2o_api.h"

#include "g2o/core/sparse_optimizer.h"
#include "g2o/core/hyper_graph_action.h"

namespace fugro
{
  namespace g2o_native
  {
    class Graph;

    class PreIterationAction : public g2o::HyperGraphAction
    {
    private:
      Graph* _graph;

    public:
      explicit PreIterationAction(Graph* graph) : _graph(graph) {}
      g2o::HyperGraphAction* operator()(const g2o::HyperGraph*, Parameters* parameters);
    };

    class PostIterationAction : public g2o::HyperGraphAction
    {
    private:
      Graph* _graph;

    public:
      explicit PostIterationAction(Graph* graph) : _graph(graph) {}
      g2o::HyperGraphAction* operator()(const g2o::HyperGraph*, Parameters* parameters);
    };

    // A SparseOptimizer configured like the original wrapper: Levenberg-Marquardt
    // with a variable block size CSparse solver ("lm_var"). The graph does not
    // own vertices or edges (G2O_NO_IMPLICIT_OWNERSHIP_OF_OBJECTS); the managed
    // wrappers do.
    class Graph : public g2o::SparseOptimizer
    {
    private:
      bool _stopFlag;
      void* _user;
      fugro_g2o_iteration_cb _preIteration;
      fugro_g2o_iteration_cb _postIteration;
      PreIterationAction* _preAction;
      PostIterationAction* _postAction;
      g2o::OptimizationAlgorithm* _ownedAlgorithm;

    public:
      Graph(void* user, fugro_g2o_iteration_cb preIteration, fugro_g2o_iteration_cb postIteration);
      ~Graph();

      void requestStop()
      {
        _stopFlag = true;
      }

      void onIterationStarting(int iteration);
      void onIterationFinished(int iteration);

      int optimizeGraph(int maxIterations);
    };
  }
}
