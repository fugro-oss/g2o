#pragma once

#include "Edge.h"
#include "State.h"

namespace Fugro
{
  namespace G2O
  {
    using namespace System::Runtime::InteropServices;

    public ref class Observation abstract
    {
    public:

      /// <summary>
      /// Create a new observation that represents a measurement.
      /// </summary>
      /// <param name="dimension">Number of dimensions of this observation.</param>
      /// <param name="states">All the states to which this observation relates.</param>
      /// <remarks>Add this observatoin to a graph to be used.</remarks>
      Observation(int dimension, ... array<BaseState^>^ states);

      /// <summary>
      /// Create a new observation that represents a measurement.
      /// </summary>
      /// <param name="stdevs">The standard deviation of the measurement for each dimension.</param>
      /// <param name="states">All the states to which this observation relates.</param>
      /// <remarks>Add this observatoin to a graph to be used.</remarks>
      Observation(array<double>^ stdevs, ... array<BaseState^>^ states);

      /// <summary>
      /// Create a new observation that represents a measurement.
      /// </summary>
      /// <param name="dimension">Number of dimensions of this observation.</param>
      /// <param name="precisionMatrix">The covariance of the measurement. The precision matrix is row-wise flattened.</param>
      /// <param name="states">All the states to which this observation relates.</param>
      /// <remarks>Add this observatoin to a graph to be used.</remarks>
      Observation(int dimension, array<double>^ precisionMatrix, ... array<BaseState^>^ states);

      /// <summary>
      /// Gets the calculated squared error of the last iteration.
      /// </summary>
      property double SquaredError
      {
      public:
        double get();
      };

      /// <summary>
      /// Gets the states of this observation.
      /// </summary>
      property IReadOnlyList<BaseState^>^ States
      {
      public:
        IReadOnlyList<BaseState^>^ get();
      };

    internal:
      Edge* edge;

      initonly array<BaseState^>^ _states;

      array<double>^ ComputeError();

      void Robustify(double squaredError, double% scaledError, double% firstDerivative)
      {
        OnRobustify(squaredError, scaledError, firstDerivative);
      }

    protected:
      !Observation();

      /// <summary>
      /// Calculate the new error based on the updated states.
      /// </summary>
      /// <returns>The calculated error</returns>
      /// <remarks>The lenght of the error should be equal to the dimension of this observation.</remarks>
      virtual array<double>^ OnComputeError() = 0;

      /// <summary>
      /// Enable robust outlier filtering.
      /// </summary>
      virtual void OnRobustify(double squaredError, [Out]double% scaledError, [Out]double% firstDerivative)
      {
        scaledError = squaredError;
        firstDerivative = 1.0;
      }

    private:
      RobustKernel* _robustKernel;
      void InitializeEge(MatrixXd information);
    };
  }
}