using System;
using System.Runtime.InteropServices;

namespace Fugro.G2O
{
    /// <summary>
    /// Base class for all states that can be estimated during graph optimization.
    /// Derive from <see cref="State{T}"/> instead of this class.
    /// </summary>
    public abstract class BaseState
    {
        // Static delegate instances passed to native code. Static fields keep
        // them alive for the process lifetime; the native side never outlives them.
        private static readonly NativeMethods.OplusCallback s_Oplus = OnNativeOplus;
        private static readonly NativeMethods.JacobianStepCallback s_JacobianStep = OnNativeJacobianStep;
        private static readonly NativeMethods.StackOpCallback s_StackOp = OnNativeStackOp;
        private static readonly NativeMethods.StackSizeCallback s_StackSize = OnNativeStackSize;

        // A weak handle to this state, handed to native code as callback context.
        // Weak (like the original gcroot_weak) so the state can still be collected;
        // callbacks only occur while a Graph strongly references this state.
        private GCHandle _selfHandle;

        internal IntPtr vertex;
        internal readonly int dimension;

        private protected BaseState(int dimension)
        {
            this.dimension = dimension;

            _selfHandle = GCHandle.Alloc(this, GCHandleType.Weak);
            vertex = NativeMethods.VertexCreate(dimension, GCHandle.ToIntPtr(_selfHandle), s_Oplus, s_JacobianStep, s_StackOp, s_StackSize);
        }

        /// <summary>
        /// Releases the native vertex owned by this state.
        /// </summary>
        ~BaseState()
        {
            if (vertex != IntPtr.Zero)
            {
                NativeMethods.VertexDestroy(vertex);
                vertex = IntPtr.Zero;
            }

            if (_selfHandle.IsAllocated)
            {
                _selfHandle.Free();
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this state should be estimated.
        /// </summary>
        public bool Fixed
        {
            get
            {
                return NativeMethods.VertexGetFixed(vertex) != 0;
            }

            set
            {
                NativeMethods.VertexSetFixed(vertex, value ? 1 : 0);
            }
        }

        /// <summary>
        /// Gets the small delta used to calculate the Jacobian.
        /// </summary>
        public static double JacobianDelta
        {
            get
            {
                return NativeMethods.JacobianDelta();
            }
        }

        internal abstract void Push();
        internal abstract void Pop();
        internal abstract void DiscardTop();
        internal abstract int GetStackSize();
        internal abstract void Oplus(double[] delta);
        internal abstract void AddOplusDelta(int index);
        internal abstract void SubtractOplusDelta(int index);

        private static BaseState FromUser(IntPtr user)
        {
            return (BaseState)GCHandle.FromIntPtr(user).Target;
        }

        private static void OnNativeOplus(IntPtr user, IntPtr update)
        {
            var state = FromUser(user);
            if (state == null)
            {
                return;
            }

            try
            {
                var delta = new double[state.dimension];
                Marshal.Copy(update, delta, 0, delta.Length);

                state.Oplus(delta);
            }
            catch (Exception exception)
            {
                Graph.RecordCallbackException(exception);
            }
        }

        private static void OnNativeJacobianStep(IntPtr user, int index, int sign)
        {
            var state = FromUser(user);
            if (state == null)
            {
                return;
            }

            try
            {
                if (sign > 0)
                {
                    state.AddOplusDelta(index);
                }
                else
                {
                    state.SubtractOplusDelta(index);
                }
            }
            catch (Exception exception)
            {
                Graph.RecordCallbackException(exception);
            }
        }

        private static void OnNativeStackOp(IntPtr user, int op)
        {
            var state = FromUser(user);
            if (state == null)
            {
                return;
            }

            try
            {
                switch (op)
                {
                    case NativeMethods.StackPush:
                        state.Push();
                        break;
                    case NativeMethods.StackPop:
                        state.Pop();
                        break;
                    case NativeMethods.StackDiscardTop:
                        state.DiscardTop();
                        break;
                }
            }
            catch (Exception exception)
            {
                Graph.RecordCallbackException(exception);
            }
        }

        private static int OnNativeStackSize(IntPtr user)
        {
            var state = FromUser(user);
            if (state == null)
            {
                return 0;
            }

            try
            {
                return state.GetStackSize();
            }
            catch (Exception exception)
            {
                Graph.RecordCallbackException(exception);
                return 0;
            }
        }
    }
}
