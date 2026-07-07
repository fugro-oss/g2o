using System;

namespace Fugro.G2O
{
    /// <summary>
    /// Event arguments for the <see cref="Graph.IterationStarting"/> and
    /// <see cref="Graph.IterationFinished"/> events.
    /// </summary>
    public sealed class IterationEventArgs : EventArgs
    {
        internal IterationEventArgs(int iteration)
        {
            Iteration = iteration;
        }

        /// <summary>
        /// Gets the zero based iteration count.
        /// </summary>
        public int Iteration { get; }

        /// <summary>
        /// Gets or sets a value indicating whether to cancel the optimization.
        /// </summary>
        public bool Cancel { get; set; }
    }
}
