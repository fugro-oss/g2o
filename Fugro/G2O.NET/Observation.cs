using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Fugro.G2O
{
    /// <summary>
    /// Base class for measurements that relate one or more states.
    /// Derive from this class and implement <see cref="OnComputeError"/>.
    /// </summary>
    public abstract class Observation
    {
        private static readonly NativeMethods.ComputeErrorCallback s_ComputeError = OnNativeComputeError;
        private static readonly NativeMethods.RobustifyCallback s_Robustify = OnNativeRobustify;

        private GCHandle _selfHandle;

        internal IntPtr edge;
        internal readonly BaseState[] states;

        private readonly int _dimension;

        /// <summary>
        /// Create a new observation that represents a measurement.
        /// </summary>
        /// <param name="dimension">Number of dimensions of this observation.</param>
        /// <param name="states">All the states to which this observation relates.</param>
        /// <remarks>Add this observation to a graph to be used.</remarks>
        protected Observation(int dimension, params BaseState[] states)
        {
            if (states == null)
            {
                throw new ArgumentNullException(nameof(states));
            }

            if (dimension < 1)
            {
                throw new ArgumentException("Dimension should be greater than 1.", nameof(dimension));
            }

            var information = new double[dimension * dimension];
            for (int i = 0; i < dimension; i++)
            {
                information[dimension * i + i] = 1.0;
            }

            this.states = states;
            _dimension = dimension;

            InitializeEdge(information);
        }

        /// <summary>
        /// Create a new observation that represents a measurement.
        /// </summary>
        /// <param name="stdevs">The standard deviation of the measurement for each dimension.</param>
        /// <param name="states">All the states to which this observation relates.</param>
        /// <remarks>Add this observation to a graph to be used.</remarks>
        protected Observation(double[] stdevs, params BaseState[] states)
        {
            if (states == null)
            {
                throw new ArgumentNullException(nameof(states));
            }

            if (stdevs == null)
            {
                throw new ArgumentNullException(nameof(stdevs));
            }

            if (stdevs.Length < 1)
            {
                throw new ArgumentException("Standard deviation length should be greater than 1.", nameof(stdevs));
            }

            var dimension = stdevs.Length;
            var information = new double[dimension * dimension];
            for (int i = 0; i < dimension; i++)
            {
                information[dimension * i + i] = 1.0 / (stdevs[i] * stdevs[i]);
            }

            this.states = states;
            _dimension = dimension;

            InitializeEdge(information);
        }

        /// <summary>
        /// Create a new observation that represents a measurement.
        /// </summary>
        /// <param name="dimension">Number of dimensions of this observation.</param>
        /// <param name="precisionMatrix">The precision of the measurement. The precision matrix is row-wise flattened.</param>
        /// <param name="states">All the states to which this observation relates.</param>
        /// <remarks>Add this observation to a graph to be used.</remarks>
        protected Observation(int dimension, double[] precisionMatrix, params BaseState[] states)
        {
            if (states == null)
            {
                throw new ArgumentNullException(nameof(states));
            }

            if (precisionMatrix == null)
            {
                throw new ArgumentNullException(nameof(precisionMatrix));
            }

            if (dimension < 1)
            {
                throw new ArgumentException("Dimension should be greater than 1.", nameof(dimension));
            }

            if (dimension * dimension != precisionMatrix.Length)
            {
                throw new ArgumentException("Precision matrix should be a square", nameof(precisionMatrix));
            }

            this.states = states;
            _dimension = dimension;

            InitializeEdge((double[])precisionMatrix.Clone());
        }

        /// <summary>
        /// Releases the native edge owned by this observation.
        /// </summary>
        ~Observation()
        {
            if (edge != IntPtr.Zero)
            {
                NativeMethods.EdgeDestroy(edge);
                edge = IntPtr.Zero;
            }

            if (_selfHandle.IsAllocated)
            {
                _selfHandle.Free();
            }
        }

        /// <summary>
        /// Gets the calculated squared error of the last iteration.
        /// </summary>
        public double SquaredError
        {
            get
            {
                return NativeMethods.EdgeChi2(edge);
            }
        }

        /// <summary>
        /// Gets the states of this observation.
        /// </summary>
        public IReadOnlyList<BaseState> States
        {
            get
            {
                return new List<BaseState>(states);
            }
        }

        /// <summary>
        /// Calculate the new error based on the updated states.
        /// </summary>
        /// <returns>The calculated error</returns>
        /// <remarks>The length of the error should be equal to the dimension of this observation.</remarks>
        protected abstract double[] OnComputeError();

        /// <summary>
        /// Enable robust outlier filtering.
        /// </summary>
        protected virtual void OnRobustify(double squaredError, out double scaledError, out double firstDerivative)
        {
            scaledError = squaredError;
            firstDerivative = 1.0;
        }

        private void InitializeEdge(double[] information)
        {
            foreach (var state in states)
            {
                if (state == null)
                {
                    throw new ArgumentException("States should not contain null.", nameof(states));
                }
            }

            var vertices = new IntPtr[states.Length];
            for (int i = 0; i < states.Length; i++)
            {
                vertices[i] = states[i].vertex;
            }

            _selfHandle = GCHandle.Alloc(this, GCHandleType.Weak);
            edge = NativeMethods.EdgeCreate(_dimension, information, vertices.Length, vertices, GCHandle.ToIntPtr(_selfHandle), s_ComputeError, s_Robustify);
        }

        private static Observation FromUser(IntPtr user)
        {
            return (Observation)GCHandle.FromIntPtr(user).Target;
        }

        private static int OnNativeComputeError(IntPtr user, IntPtr error, int dimension)
        {
            var observation = FromUser(user);
            if (observation == null)
            {
                return 1;
            }

            try
            {
                var computedError = observation.OnComputeError();

                if (computedError == null || computedError.Length != dimension)
                {
                    throw new InvalidOperationException("Error function returned wrong length.");
                }

                Marshal.Copy(computedError, 0, error, dimension);

                return 0;
            }
            catch (Exception exception)
            {
                Graph.RecordCallbackException(exception);
                return 1;
            }
        }

        private static void OnNativeRobustify(IntPtr user, double squaredError, ref double rho0, ref double rho1)
        {
            var observation = FromUser(user);
            if (observation == null)
            {
                rho0 = squaredError;
                rho1 = 1.0;
                return;
            }

            try
            {
                observation.OnRobustify(squaredError, out rho0, out rho1);
            }
            catch (Exception exception)
            {
                Graph.RecordCallbackException(exception);
                rho0 = squaredError;
                rho1 = 1.0;
            }
        }
    }
}
