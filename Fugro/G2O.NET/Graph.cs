using System;
using System.Collections.Generic;
using System.Runtime.ExceptionServices;
using System.Runtime.InteropServices;

namespace Fugro.G2O
{
    /// <summary>
    /// A graph of states and observations that can be optimized using g2o
    /// (Levenberg-Marquardt with a sparse variable block size solver).
    /// </summary>
    public class Graph
    {
        private static readonly NativeMethods.IterationCallback s_PreIteration = OnNativePreIteration;
        private static readonly NativeMethods.IterationCallback s_PostIteration = OnNativePostIteration;

        // The graph whose Optimize call is currently on this thread's stack.
        // Native callbacks report exceptions from user code here, because managed
        // exceptions must not unwind through the native g2o frames.
        [ThreadStatic]
        private static Graph t_current;

        private GCHandle _selfHandle;
        private IntPtr _handle;

        private readonly HashSet<BaseState> _states = new HashSet<BaseState>();
        private readonly HashSet<Observation> _observations = new HashSet<Observation>();

        private Exception _callbackException;

        /// <summary>
        /// Create a new graph which can be optimized after states and observations are added.
        /// </summary>
        public Graph()
        {
            _selfHandle = GCHandle.Alloc(this, GCHandleType.Weak);
            _handle = NativeMethods.GraphCreate(GCHandle.ToIntPtr(_selfHandle), s_PreIteration, s_PostIteration);
        }

        /// <summary>
        /// Releases the native optimizer owned by this graph.
        /// </summary>
        ~Graph()
        {
            if (_handle != IntPtr.Zero)
            {
                NativeMethods.GraphDestroy(_handle);
                _handle = IntPtr.Zero;
            }

            if (_selfHandle.IsAllocated)
            {
                _selfHandle.Free();
            }
        }

        /// <summary>
        /// Occurs when a new iteration is starting.
        /// </summary>
        public event EventHandler<IterationEventArgs> IterationStarting;

        /// <summary>
        /// Occurs when an iteration has finished.
        /// </summary>
        public event EventHandler<IterationEventArgs> IterationFinished;

        /// <summary>
        /// Gets the weighted and robustified squared error of the last iteration.
        /// </summary>
        public double SquaredError
        {
            get
            {
                var squaredError = NativeMethods.GraphChi2(_handle);
                GC.KeepAlive(this);

                return squaredError;
            }
        }

        /// <summary>
        /// Gets all observations in this graph.
        /// </summary>
        public IReadOnlyList<Observation> Observations
        {
            get
            {
                return new List<Observation>(_observations);
            }
        }

        /// <summary>
        /// Gets all states in this graph.
        /// </summary>
        public IReadOnlyList<BaseState> States
        {
            get
            {
                return new List<BaseState>(_states);
            }
        }

        /// <summary>
        /// Add a new observation to the graph.
        /// </summary>
        /// <param name="observation">The observation to add.</param>
        public void AddObservation(Observation observation)
        {
            if (observation == null)
            {
                throw new ArgumentNullException(nameof(observation));
            }

            // Keep managed observations alive, in case the client code doesn't.
            _observations.Add(observation);

            foreach (var state in observation.states)
            {
                if (_states.Add(state))
                {
                    AddState(state);
                }
            }

            if (NativeMethods.GraphAddEdge(_handle, observation.edge) != 0)
            {
                throw new InvalidOperationException("Unable to add this observation to the graph.");
            }

            GC.KeepAlive(this);
        }

        /// <summary>
        /// Add multiple new observations to the graph.
        /// </summary>
        /// <param name="observations">The observations to add.</param>
        public void AddObservation(IEnumerable<Observation> observations)
        {
            if (observations == null)
            {
                throw new ArgumentNullException(nameof(observations));
            }

            foreach (var observation in observations)
            {
                AddObservation(observation);
            }
        }

        /// <summary>
        /// Start the optimization of this graph.
        /// </summary>
        public void Optimize()
        {
            Optimize(100);
        }

        /// <summary>
        /// Start the optimization of this graph.
        /// </summary>
        /// <param name="maxIterations">The maximum number of iterations after which the optimization will stop. The default maximum is 100</param>
        public void Optimize(int maxIterations)
        {
            _callbackException = null;

            var previous = t_current;
            t_current = this;

            try
            {
                NativeMethods.GraphOptimize(_handle, maxIterations);
            }
            finally
            {
                t_current = previous;
                GC.KeepAlive(this);
            }

            if (_callbackException != null)
            {
                var exception = _callbackException;
                _callbackException = null;

                ExceptionDispatchInfo.Capture(exception).Throw();
            }
        }

        internal static void RecordCallbackException(Exception exception)
        {
            var current = t_current;
            if (current == null)
            {
                return;
            }

            if (current._callbackException == null)
            {
                current._callbackException = exception;
            }

            NativeMethods.GraphRequestStop(current._handle);
        }

        private void AddState(BaseState state)
        {
            var result = NativeMethods.GraphAddVertex(_handle, state.vertex);

            if (result == 1)
            {
                throw new InvalidOperationException("This state is already part of another graph.");
            }

            if (result != 0)
            {
                throw new InvalidOperationException("Unable to add state to graph.");
            }
        }

        private static Graph FromUser(IntPtr user)
        {
            return (Graph)GCHandle.FromIntPtr(user).Target;
        }

        private static int OnNativePreIteration(IntPtr user, int iteration)
        {
            var graph = FromUser(user);
            if (graph == null)
            {
                return 0;
            }

            return graph.RaiseIterationEvent(graph.IterationStarting, iteration);
        }

        private static int OnNativePostIteration(IntPtr user, int iteration)
        {
            var graph = FromUser(user);
            if (graph == null)
            {
                return 0;
            }

            return graph.RaiseIterationEvent(graph.IterationFinished, iteration);
        }

        private int RaiseIterationEvent(EventHandler<IterationEventArgs> handler, int iteration)
        {
            // g2o also invokes the actions with iteration -1 (e.g. after termination).
            if (iteration == -1 || handler == null)
            {
                return 0;
            }

            try
            {
                var args = new IterationEventArgs(iteration);

                handler(this, args);

                return args.Cancel ? 1 : 0;
            }
            catch (Exception exception)
            {
                RecordCallbackException(exception);
                return 1;
            }
        }
    }
}
