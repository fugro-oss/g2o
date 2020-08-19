#include "Observation.h"

#pragma unmanaged
#include "g2o/core/robust_kernel.h"
#pragma managed

using namespace g2o;
using namespace System;
using namespace Fugro::G2O;

Edge::Edge(InformationType information, Observation^ observation) :
_observation(observation),
_error(ErrorVector::Zero(information.rows())),
_information(information)
{
  _dimension = (int)information.rows();
}

void Edge::computeError()
{
  auto error = _observation->ComputeError();

  if (_dimension != error->Length)
  {
    throw gcnew InvalidOperationException("Error function returned wrong length.");
  }

  for (int i = 0; i < _dimension; i++)
  {
    _error[i] = error[i];
  }
}

double Edge::chi2() const
{
  return _error.dot(_information * _error);
}

void Edge::linearizeOplus(JacobianWorkspace& jacobianWorkspace)
{
  for (size_t i = 0; i < _vertices.size(); ++i)
  {
    OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(_vertices[i]);
    assert(v->dimension() >= 0);
    new (&_jacobianOplus[i]) JacobianType(jacobianWorkspace.workspaceForVertex((int)i), _dimension, v->dimension());
  }

  linearizeOplus();
}

void Edge::linearizeOplus()
{
  const double scalar = 1.0 / (2 * Vertex::jacobianDelta());

  ErrorVector errorBak;
  ErrorVector errorBeforeNumeric = _error;

  for (size_t i = 0; i < _vertices.size(); ++i)
  {
    auto vi = static_cast<Vertex*>(_vertices[i]);

    if (vi->fixed())
    {
      continue;
    }

    const int vi_dim = vi->dimension();

    assert(_jacobianOplus[i].rows() == _dimension && _jacobianOplus[i].cols() == vi_dim && "jacobian cache dimension does not match");
    _jacobianOplus[i].resize(_dimension, vi_dim);
   
    // add small step along the unit vector in each dimension
    for (int d = 0; d < vi_dim; ++d)
    {
      vi->push();
      vi->addOplusDelta(d);
      computeError();
      errorBak = _error;
      vi->pop();
      vi->push();
      vi->subtractOplusDelta(d);
      computeError();
      errorBak -= _error;
      vi->pop();

      _jacobianOplus[i].col(d) = scalar * errorBak;
    } // end dimension
  }

  _error = errorBeforeNumeric;
}

void Edge::constructQuadraticForm()
{
  if (this->robustKernel())
  {
    double error = this->chi2();
    Vector3d rho;
    this->robustKernel()->robustify(error, rho);
    VectorXd omega_r = -_information * _error;
    omega_r *= rho[1];
    computeQuadraticForm(this->robustInformation(rho), omega_r);
  }
  else
  {
    computeQuadraticForm(_information, -_information * _error);
  }
}

void Edge::computeQuadraticForm(const InformationType& omega, const ErrorVector& weightedError)
{
  for (size_t i = 0; i < _vertices.size(); ++i)
  {
    OptimizableGraph::Vertex* from = static_cast<OptimizableGraph::Vertex*>(_vertices[i]);
    bool istatus = !(from->fixed());

    if (istatus)
    {
      const MatrixXd& A = _jacobianOplus[i];

      MatrixXd AtO = A.transpose() * omega;
      int fromDim = from->dimension();
      assert(fromDim >= 0);
      Map<MatrixXd> fromMap(from->hessianData(), fromDim, fromDim);
      Map<VectorXd> fromB(from->bData(), fromDim);

      // ii block in the hessian
      fromMap.noalias() += AtO * A;
      fromB.noalias() += A.transpose() * weightedError;

      // compute the off-diagonal blocks ij for all j
      for (size_t j = i + 1; j < _vertices.size(); ++j)
      {
        OptimizableGraph::Vertex* to = static_cast<OptimizableGraph::Vertex*>(_vertices[j]);

        bool jstatus = !(to->fixed());
        if (jstatus)
        {
          const MatrixXd& B = _jacobianOplus[j];
          size_t idx = computeUpperTriangleIndex(i, j);
          assert(idx < _hessian.size());
          HessianHelper& hhelper = _hessian[idx];
          if (hhelper.transposed)
          { // we have to write to the block as transposed
            hhelper.matrix.noalias() += B.transpose() * AtO.transpose();
          }
          else
          {
            hhelper.matrix.noalias() += AtO * B;
          }
        }
      }
    }
  }
}

void Edge::mapHessianMemory(double* d, int i, int j, bool rowMajor)
{
  size_t idx = computeUpperTriangleIndex(i, j);
  assert(idx < _hessian.size());
  OptimizableGraph::Vertex* vi = static_cast<OptimizableGraph::Vertex*>(HyperGraph::Edge::vertex(i));
  OptimizableGraph::Vertex* vj = static_cast<OptimizableGraph::Vertex*>(HyperGraph::Edge::vertex(j));
  assert(vi->dimension() >= 0);
  assert(vj->dimension() >= 0);
  HessianHelper& h = _hessian[idx];
  if (rowMajor)
  {
    if (h.matrix.data() != d || h.transposed != rowMajor)
    {
      new (&h.matrix) Edge::HessianBlockType(d, vj->dimension(), vi->dimension());
    }
  }
  else
  {
    if (h.matrix.data() != d || h.transposed != rowMajor)
    {
      new (&h.matrix)  Edge::HessianBlockType(d, vi->dimension(), vj->dimension());
    }
  }

  h.transposed = rowMajor;
}

void Edge::resize(size_t size)
{
  OptimizableGraph::Edge::resize(size);
  int n = (int)_vertices.size();
  int maxIdx = (n * (n - 1)) / 2;
  assert(maxIdx >= 0);
  _hessian.resize(maxIdx);
  _jacobianOplus.resize(size, JacobianType(0, 0, 0));
}

bool Edge::allVerticesFixed() const
{
  for (size_t i = 0; i < _vertices.size(); ++i)
  {
    if (!static_cast<const OptimizableGraph::Vertex*> (_vertices[i])->fixed())
    {
      return false;
    }
  }

  return true;
}
