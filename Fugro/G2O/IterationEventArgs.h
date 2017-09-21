#pragma once

using namespace System;

namespace Fugro{
  namespace G2O {
    public ref class IterationEventArgs : EventArgs
    {
    private:
      int iteration;
      bool cancel;

    internal:
      IterationEventArgs(int iteration);

    public:

      /// <summary>
      /// Gets the zero based iteration count.
      /// </summary>
      property int Iteration
      {
      public:
        int get();
      };

      /// <summary>
      /// Gets or sets a value indicating whether to cancel the optimization.
      /// </summary>
      property bool Cancel
      {
      public:
        bool get();
        void set(bool);
      };
    };
  };
}