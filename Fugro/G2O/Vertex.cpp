#include "Stdafx.h"

#pragma unmanaged
#include <Eigen/Cholesky>
#include <Eigen/LU>
#pragma managed

#include "State.h"

using namespace g2o;
using namespace Fugro::G2O;

Vertex::Vertex(int dimension, BaseState^ state) :
_state(state),
_hessian(nullptr, dimension, dimension),
_b(dimension)
{
  _dimension = dimension;
}

const double& Vertex::hessian(int i, int j) const
{
  assert(i < _dimension && j < _dimension);
  return _hessian(i, j);
}

double& Vertex::hessian(int i, int j)
{
  assert(i < _dimension && j < _dimension);
  return _hessian(i, j);
}

double Vertex::hessianDeterminant() const
{
  return _hessian.determinant();
}

double* Vertex::hessianData()
{
  return const_cast<double*>(_hessian.data());
}

void Vertex::mapHessianMemory(double* d)
{
  new (&_hessian) HessianBlockType(d, _dimension, _dimension);
}

int Vertex::copyB(double* b_) const
{
  memcpy(b_, _b.data(), _dimension * sizeof(double));

  return _dimension;
}

const double& Vertex::b(int i) const
{
  assert(i < _dimension);
  return _b(i);
}

double& Vertex::b(int i)
{
  assert(i < _dimension);
  return _b(i);
}

double* Vertex::bData()
{
  return _b.data();
}

void Vertex::clearQuadraticForm()
{
  _b.setZero();
}

double Vertex::solveDirect(double lambda)
{
  MatrixXd tempA = _hessian + MatrixXd::Identity(_dimension, _dimension) * lambda;
  double det = tempA.determinant();

  if (g2o_isnan(det) || det < std::numeric_limits<double>::epsilon())
  {
    return det;
  }

  VectorXd dx = tempA.llt().solve(_b);
  oplus(&dx[0]);

  return det;
}

VectorXd& Vertex::b()
{
  return _b;
}

const VectorXd& Vertex::b() const
{
  return _b;
}

Vertex::HessianBlockType& Vertex::A()
{
  return _hessian;
}

const Vertex::HessianBlockType& Vertex::A() const
{
  return _hessian;
}

void Vertex::push()
{
  _state->Push();
}

void Vertex::pop()
{
  _state->Pop();
  updateCache();
}

void Vertex::discardTop()
{
  _state->DiscardTop();
}

int Vertex::stackSize() const
{
  return _state->GetStackSize();
}

void Vertex::oplusImpl(const double* update)
{
  _state->Oplus(update);
}

void Vertex::addOplusDelta(int index)
{
  _state->AddOplusDelta(index);
  updateCache();
}

void Vertex::subtractOplusDelta(int index)
{
  _state->SubtractOplusDelta(index);
  updateCache();
}

void Vertex::setToOriginImpl()
{
}

bool Vertex::read(std::istream& is)
{
  return true;
}

bool Vertex::write(std::ostream& os) const
{
  return os.good();
}