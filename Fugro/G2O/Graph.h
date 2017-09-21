#pragma once

#pragma unmanaged

#pragma warning(push, 0)
#include "g2o/core/sparse_optimizer.h"
#pragma warning(pop) 

#pragma managed

#include "BaseState.h"
#include "Observation.h"
#include "IterationEventArgs.h"

using namespace System::Collections::Generic;

namespace Fugro
{
  namespace G2O
  {
    public ref class Graph
    {
    protected:
      !Graph();

    public:

      /// <summary>
      /// Create a new graph which can be optimized after states and observations are added.
      /// </summary>
      Graph();

      /// <summary>
      /// Add a new observation to the graph.
      /// </summary>
      /// <param name="observation">The observation to add.</param>
      void AddObservation(Observation^ observation);

      /// <summary>
      /// Add multiple new observation to the graph.
      /// </summary>
      /// <param name="observations">The observation to add.</param>
      void AddObservation(IEnumerable<Observation^>^ observations);

      /// <summary>
      /// Start the optimization of this graph.
      /// </summary>
      void Optimize()
      {
        Optimize(100);
      }

      /// <summary>
      /// Start the optimization of this graph.
      /// </summary>
      /// <param name="maxIterations">The maximum number of iterations after which the optimization will stop. The default maximum is 100</param>
      void Optimize(int maxIterations);

      /// <summary>
      /// Gets the weighted and robustified squared error of the last iteration.
      /// </summary>
      property double SquaredError
      {
      public:
        double get();
      };

      /// <summary>
      /// Gets all observations in this graph.
      /// </summary>
      property IReadOnlyList<Observation^>^ Observations
      {
      public:
        IReadOnlyList<Observation^>^ get();
      };

      /// <summary>
      /// Gets all states in this graph.
      /// </summary>
      property IReadOnlyList<BaseState^>^ States
      {
      public:
        IReadOnlyList<BaseState^>^ get();
      };

      /// <summary>
      /// Occurs when a new iteration is starting.
      /// </summary>
      event EventHandler<IterationEventArgs^>^ IterationStarting;

      /// <summary>
      /// Occurs when an iteration has finished.
      /// </summary>
      event EventHandler<IterationEventArgs^>^ IterationFinished;

    internal:
      void OnIterationStarting(int);
      void OnIterationFinished(int);

    private:
      bool initialized;
      bool* _stopFlag;
      OptimizationAlgorithm* _optimizationAlgorithm;
      SparseOptimizer* _optimizer;

      HyperGraphAction* _preIterationAction;
      HyperGraphAction* _postIterationAction;

      initonly HashSet<BaseState^>^ _states;
      initonly HashSet<Observation^>^ _observations;

      void AddState(BaseState^ state);

      void Stop();
    };
  }

}
