using System;
using System.Runtime.InteropServices;

namespace Fugro.G2O
{
    internal static class NativeMethods
    {
        private const string LibraryName = "fugro_g2o";

        internal const int StackPush = 0;
        internal const int StackPop = 1;
        internal const int StackDiscardTop = 2;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OplusCallback(IntPtr user, IntPtr update);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void JacobianStepCallback(IntPtr user, int index, int sign);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void StackOpCallback(IntPtr user, int op);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate int StackSizeCallback(IntPtr user);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate int ComputeErrorCallback(IntPtr user, IntPtr error, int dimension);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void RobustifyCallback(IntPtr user, double squaredError, ref double rho0, ref double rho1);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate int IterationCallback(IntPtr user, int iteration);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_graph_create", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr GraphCreate(IntPtr user, IterationCallback preIteration, IterationCallback postIteration);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_graph_destroy", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void GraphDestroy(IntPtr graph);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_graph_add_vertex", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GraphAddVertex(IntPtr graph, IntPtr vertex);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_graph_add_edge", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GraphAddEdge(IntPtr graph, IntPtr edge);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_graph_optimize", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GraphOptimize(IntPtr graph, int maxIterations);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_graph_request_stop", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void GraphRequestStop(IntPtr graph);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_graph_chi2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern double GraphChi2(IntPtr graph);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_vertex_create", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr VertexCreate(
            int dimension,
            IntPtr user,
            OplusCallback oplus,
            JacobianStepCallback jacobianStep,
            StackOpCallback stackOp,
            StackSizeCallback stackSize);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_vertex_destroy", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void VertexDestroy(IntPtr vertex);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_vertex_get_fixed", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int VertexGetFixed(IntPtr vertex);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_vertex_set_fixed", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void VertexSetFixed(IntPtr vertex, int isFixed);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_vertex_get_graph", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr VertexGetGraph(IntPtr vertex);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_vertex_copy_hessian", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int VertexCopyHessian(IntPtr vertex, [Out] double[] destination);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_edge_create", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr EdgeCreate(
            int dimension,
            double[] informationRowMajor,
            int vertexCount,
            IntPtr[] vertices,
            IntPtr user,
            ComputeErrorCallback computeError,
            RobustifyCallback robustify);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_edge_destroy", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EdgeDestroy(IntPtr edge);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_edge_chi2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern double EdgeChi2(IntPtr edge);

        [DllImport(LibraryName, EntryPoint = "fugro_g2o_jacobian_delta", CallingConvention = CallingConvention.Cdecl)]
        internal static extern double JacobianDelta();
    }
}
