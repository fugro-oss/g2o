#pragma once

#include "fugro_g2o_api.h"
#include "vertex.h"

#include "g2o/core/optimizable_graph.h"

namespace fugro
{
  namespace g2o_native
  {
    using namespace Eigen;

    struct EdgeCallbacks
    {
      void* user;
      fugro_g2o_compute_error_cb computeError;
      fugro_g2o_robustify_cb robustify;
    };

    class Edge : public g2o::OptimizableGraph::Edge
    {
    private:
      typedef VectorXd ErrorVector;
      typedef MatrixXd InformationType;
      typedef MatrixXd::MapType JacobianType;
      typedef Map<MatrixXd, MatrixXd::Flags & PacketAccessBit ? Aligned : Unaligned> HessianBlockType;

      struct HessianHelper
      {
        Map<MatrixXd> matrix;     ///< the mapped memory
        bool transposed;          ///< the block has to be transposed
        HessianHelper() : matrix(0, 0, 0), transposed(false) {}
      };

      InformationType _information;
      ErrorVector _error;
      std::vector<HessianHelper> _hessian;
      std::vector<JacobianType, aligned_allocator<JacobianType> > _jacobianOplus;

      EdgeCallbacks _callbacks;

      inline size_t computeUpperTriangleIndex(size_t i, size_t j)
      {
        size_t elemsUpToCol = ((j - 1) * j) / 2;
        return elemsUpToCol + i;
      }

      InformationType robustInformation(const Vector3d& rho)
      {
        return rho[1] * _information;
      }

    public:
      Edge(const InformationType& information, const EdgeCallbacks& callbacks);

      double chi2() const;

      const ErrorVector& error() const
      {
        return _error;
      }

      ErrorVector& error()
      {
        return _error;
      }

      const double* errorData() const
      {
        return _error.data();
      }

      double* errorData()
      {
        return _error.data();
      }

      const double* informationData() const
      {
        return _information.data();
      }

      double* informationData()
      {
        return _information.data();
      }

      int rank() const
      {
        return _dimension;
      }

      void linearizeOplus(g2o::JacobianWorkspace& jacobianWorkspace);

      void linearizeOplus();

      void constructQuadraticForm();

      void computeQuadraticForm(const InformationType& omega, const ErrorVector& weightedError);

      void mapHessianMemory(double* d, int i, int j, bool rowMajor);

      void resize(size_t size);

      bool allVerticesFixed() const;

      void computeError();

      void initialEstimate(const g2o::OptimizableGraph::VertexSet&, g2o::OptimizableGraph::Vertex*)
      {
      }

      bool read(std::istream&)
      {
        return false;
      }

      bool write(std::ostream&) const
      {
        return false;
      }
    };
  }
}
