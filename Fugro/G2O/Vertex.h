#pragma once

#include "gcroot_weak.h"

#pragma unmanaged
#include "g2o/core/optimizable_graph.h"
#pragma managed

namespace Fugro
{
  namespace G2O
  {
    using namespace g2o;
    using namespace Eigen;

    ref class BaseState;

    class Vertex :public OptimizableGraph::Vertex
    {

    private:
      typedef Map<MatrixXd, MatrixXd::Flags & PacketAccessBit ? Aligned : Unaligned > HessianBlockType;

      Vertex::HessianBlockType _hessian;
      VectorXd _b;

      gcroot_weak<BaseState^> _state;

    public:

      static double jacobianDelta()
      {
        return  1e-9;
      }

      Vertex(int, BaseState^);

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

      Vertex::HessianBlockType& A();

      const Vertex::HessianBlockType& A() const;

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
