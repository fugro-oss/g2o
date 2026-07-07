using System;
using System.Collections.Generic;

namespace Fugro.G2O
{
    /// <summary>
    /// A state with an estimate of type <typeparamref name="T"/> that is refined
    /// during graph optimization. Derive from this class and implement
    /// <see cref="OnUpdate"/>.
    /// </summary>
    public abstract class State<T> : BaseState
    {
        private readonly Stack<T> _backup;
        private T _estimate;

        /// <summary>
        /// Create a new state that will be estimated during optimization.
        /// </summary>
        protected State(T initial, int dimension)
            : base(dimension)
        {
            if (initial == null)
            {
                throw new ArgumentNullException(nameof(initial));
            }

            _estimate = initial;
            _backup = new Stack<T>(1);
        }

        /// <summary>
        /// Gets the current estimated value of this state.
        /// </summary>
        public T Estimate
        {
            get
            {
                return _estimate;
            }
        }

        /// <summary>
        /// Converts a state to its current estimated value.
        /// </summary>
        public static implicit operator T(State<T> state)
        {
            return state._estimate;
        }

        /// <summary>
        /// Get precision matrix of parameters.
        /// </summary>
        /// <remarks>Precision matrix is also called information or weight matrix, which is the inverse of covariance matrix.</remarks>
        public double[] GetPrecisionMatrix()
        {
            if (Fixed)
            {
                throw new InvalidOperationException("Fixed state does not have a precision matrix.");
            }

            var precisionVector = new double[dimension * dimension];
            if (NativeMethods.VertexCopyHessian(vertex, precisionVector) == 0)
            {
                throw new InvalidOperationException("State is not part of an optimized graph.");
            }

            return precisionVector;
        }

        /// <summary>
        /// Update the current estimate with a given delta.
        /// </summary>
        /// <param name="delta">The change to apply on the current estimate</param>
        /// <returns>The updated estimate</returns>
        protected abstract T OnUpdate(double[] delta);

        /// <summary>
        /// Update the current estimate with a positive small delta on the given index in the state array.
        /// </summary>
        /// <param name="index">Position in state array to apply a change on the current estimate</param>
        /// <returns>The updated estimate</returns>
        protected virtual T OnPositiveJacobianUpdate(int index)
        {
            var deltas = new double[dimension];
            deltas[index] = JacobianDelta;

            return OnUpdate(deltas);
        }

        /// <summary>
        /// Update the current estimate with a negative small delta on the given index in the state array.
        /// </summary>
        /// <param name="index">Position in state array to apply a change on the current estimate</param>
        /// <returns>The updated estimate</returns>
        protected virtual T OnNegativeJacobianUpdate(int index)
        {
            var deltas = new double[dimension];
            deltas[index] = -JacobianDelta;

            return OnUpdate(deltas);
        }

        internal sealed override void Oplus(double[] delta)
        {
            _estimate = OnUpdate(delta);
        }

        internal sealed override void AddOplusDelta(int index)
        {
            _estimate = OnPositiveJacobianUpdate(index);
        }

        internal sealed override void SubtractOplusDelta(int index)
        {
            _estimate = OnNegativeJacobianUpdate(index);
        }

        internal sealed override void Push()
        {
            _backup.Push(_estimate);
        }

        internal sealed override void Pop()
        {
            _estimate = _backup.Pop();
        }

        internal sealed override void DiscardTop()
        {
            _backup.Pop();
        }

        internal sealed override int GetStackSize()
        {
            return _backup.Count;
        }
    }
}
