#pragma once

#include "fugro_g2o_api.h"

#include "g2o/core/optimizable_graph.h"

namespace fugro
{
  namespace g2o_native
  {
    using namespace g2o;
    using namespace Eigen;

    struct VertexCallbacks
    {
      void* user;
      fugro_g2o_oplus_cb oplus;
      fugro_g2o_jacobian_step_cb jacobianStep;
      fugro_g2o_stack_op_cb stackOp;
      fugro_g2o_stack_size_cb stackSize;
    };

    class Vertex : public OptimizableGraph::Vertex
    {
    private:
      typedef Map<MatrixXd, MatrixXd::Flags & PacketAccessBit ? Aligned : Unaligned> HessianBlockType;

      HessianBlockType _hessian;
      VectorXd _b;

      VertexCallbacks _callbacks;

    public:
      static double jacobianDelta()
      {
        return 1e-9;
      }

      Vertex(int dimension, const VertexCallbacks& callbacks);

      const double& hessian(int i, int j) const;

      double& hessian(int i, int j);

      double hessianDeterminant() const;

      double* hessianData();

      void mapHessianMemory(double* d);

      int copyB(double* b_) const;

      const double& b(int i) const;

      double& b(int i);

      double* bData();

      void clearQuadraticForm();

      double solveDirect(double lambda);

      VectorXd& b();

      const VectorXd& b() const;

      HessianBlockType& A();

      const HessianBlockType& A() const;

      void push();

      void pop();

      void discardTop();

      int stackSize() const;

      void oplusImpl(const double* update);

      void addOplusDelta(int index);
      void subtractOplusDelta(int index);

      void setToOriginImpl();

      bool read(std::istream& is);

      bool write(std::ostream& os) const;
    };
  }
}
