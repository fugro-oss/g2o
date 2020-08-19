#include "Graph.h"
#include "IterationActions.h"
#include "IterationEventArgs.h"

#pragma unmanaged

#include "g2o\core\optimization_algorithm_factory.h"
#include "g2o\core\optimization_algorithm_levenberg.h"

#pragma managed

using namespace System;
using namespace Fugro::G2O;

Graph::Graph() : initialized(false)
{
  OptimizationAlgorithmProperty props;
  _optimizationAlgorithm = g2o::OptimizationAlgorithmFactory::instance()->construct("lm_var", props);

  _stopFlag = new bool(false);

  _optimizer = new SparseOptimizer();
  _optimizer->setAlgorithm(_optimizationAlgorithm);
  _optimizer->setForceStopFlag(_stopFlag);

  _preIterationAction = new PreIterationAction(this);
  _postIterationAction = new PostIterationAction(this);

  _optimizer->addPreIterationAction(_preIterationAction);
  _optimizer->addPostIterationAction(_postIterationAction);

  _states = gcnew HashSet<BaseState^>();
  _observations = gcnew HashSet<Observation^>();
}

Graph::!Graph()
{
  delete _optimizer;
  delete _optimizationAlgorithm;
  delete _preIterationAction;
  delete _postIterationAction;
  delete _stopFlag;
}

void Graph::Stop()
{
  *_stopFlag = true;
}

void Graph::OnIterationStarting(int iteration)
{
  if (iteration != -1)
  {
    IterationEventArgs^ args = gcnew IterationEventArgs(iteration);

    IterationStarting(this, args);

    if (args->Cancel)
    {
      Stop();
    }
  }
}

void Graph::OnIterationFinished(int iteration)
{
  if (iteration != -1)
  {
    IterationEventArgs^ args = gcnew IterationEventArgs(iteration);

    IterationFinished(this, args);

    if (args->Cancel)
    {
      Stop();
    }
  }
}

void Graph::AddState(BaseState^ state)
{
  if (state->vertex->graph() != nullptr && state->vertex->graph() != _optimizer)
  {
    throw gcnew System::InvalidOperationException("This state is already part of another graph.");
  }

  if (!_optimizer->addVertex(state->vertex))
  {
    throw gcnew System::InvalidOperationException("Unable to add state to graph.");
  }
}

void Graph::AddObservation(Observation^ observation)
{
  initialized = false;

  // Keep managed observations alive, in case the client code doesn't.
  _observations->Add(observation);

  for each (BaseState^ state in observation->_states)
  {
    if (_states->Add(state))
    {
      AddState(state);
    }
  }

  if (!_optimizer->addEdge(observation->edge))
  {
    throw gcnew System::InvalidOperationException("Unable to add this observation to the graph.");
  }
}

void Graph::AddObservation(IEnumerable<Observation^>^ observations)
{
  for each(Observation^ observation in observations)
  {
    AddObservation(observation);
  }
}

void Graph::Optimize(int maxIterations)
{
  *_stopFlag = false;

  if (!initialized)
  {
    _optimizer->initializeOptimization();
  }

  _optimizer->optimize(maxIterations);

  GC::KeepAlive(this);
}

double Graph::SquaredError::get()
{
  return _optimizer->activeRobustChi2();
}

IReadOnlyList<Observation^>^ Graph::Observations::get()
{
  return System::Linq::Enumerable::ToList(_observations);
}

IReadOnlyList<BaseState^>^ Graph::States::get()
{
  return gcnew List<BaseState^>(_states);
}
