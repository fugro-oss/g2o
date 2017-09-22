#pragma once

#include "Vertex.h"
#include "BaseState.h"

using namespace System::Collections::Generic;

namespace Fugro
{
  namespace G2O
  {
    generic <class T>
      public ref class State abstract : BaseState
      {
      public:

        /// <summary>
        /// Create a new state that will be estimated during optimization.
        /// </summary>
        State(T initial, int dimension);

        /// <summary>
        /// Get precision matrix of parameters.
        /// </summary>
        /// <remarks>Precision matrix is also called information or weigth matrix, which is the inverse of covariance matrix.</remarks>
        array<double>^ State<T>::GetPrecisionMatrix();

        /// <summary>
        /// Gets the current estimated value of this state.
        /// </summary>
        property T Estimate
        {
        public:
          T get();
        };

        static operator T(State^ state)
        {
          return state->_estimate;
        }

      protected:
        !State();

        /// <summary>
        /// Update the current estimate with a given delta.
        /// </summary>
        /// <param name="delta">The change to apply on the current estimate</param>
        /// <returns>The updated estimate</returns>
        virtual T OnUpdate(array<double>^ delta) = 0;

        /// <summary>
        /// Update the current estimate with a positive small delta on the given index in the state array.
        /// </summary>
        /// <param name="index">Position in state array to apply a change on the current estimate</param>
        /// <returns>The updated estimate</returns>
        virtual T OnPositiveJacobianUpdate(int index);

        /// <summary>
        /// Update the current estimate with a negative small delta on the given index in the state array.
        /// </summary>
        /// <param name="index">Position in state array to apply a change on the current estimate</param>
        /// <returns>The updated estimate</returns>
        virtual T OnNegativeJacobianUpdate(int index);

      internal:
        virtual void Push() override;
        virtual void Pop() override;
        virtual void DiscardTop() override;
        virtual int GetStackSize() override;
        virtual void Oplus(const double*) override;
        virtual void AddOplusDelta(int) override;
        virtual void SubtractOplusDelta(int) override;

      private:
        initonly Stack<T>^ _backup;
        T _estimate;

      };
  }
}
