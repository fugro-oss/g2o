#pragma once

#include "Vertex.h"

namespace Fugro
{
  namespace G2O
  {
    public ref class BaseState abstract
    {
    public:

      /// <summary>
      /// Gets or sets a value indicating whether this state should be estimated.
      /// </summary>
      property bool Fixed
      {
        bool get()
        {
          return vertex->fixed();
        }

        void set(bool value)
        {
          vertex->setFixed(value);
        }
      };

      /// <summary>
      /// Gets the small delta used to calculate the Jacobian.
      /// </summary>
      static property double JacobianDelta
      {
      public:
        double get()
        {
          return Vertex::jacobianDelta();
        }
      };

    internal:
      Vertex* vertex;

      virtual void Push() = 0;
      virtual void Pop() = 0;
      virtual void DiscardTop() = 0;
      virtual int GetStackSize() = 0;
      virtual void Oplus(const double*) = 0;
      virtual void AddOplusDelta(int) = 0;
      virtual void SubtractOplusDelta(int) = 0;
    };
  }
}
