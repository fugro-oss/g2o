#pragma once

#pragma unmanaged
#include "g2o\core\hyper_graph_action.h"
#pragma managed

#include "gcroot_weak.h"
#include "Graph.h"

namespace Fugro
{
  namespace G2O
  {
    class PreIterationAction :public g2o::HyperGraphAction
    {
    private:
      gcroot_weak<Graph^> _graph;

    public:
      PreIterationAction(Graph^);
      g2o::HyperGraphAction* operator()(const g2o::HyperGraph*, Parameters*);
    };

    class PostIterationAction :public g2o::HyperGraphAction
    {
    private:
      gcroot_weak<Graph^> _graph;

    public:
      PostIterationAction(Graph^);
      g2o::HyperGraphAction* operator()(const g2o::HyperGraph*, Parameters*);
    };
  }
}