#include "vertex.h"

#include <Eigen/Cholesky>
#include <Eigen/LU>

#include <cstring>
#include <limits>

using namespace g2o;
using namespace fugro::g2o_native;

Vertex::Vertex(int dimension, const VertexCallbacks& callbacks) :
  _hessian(nullptr, dimension, dimension),
  _b(dimension),
  _callbacks(callbacks)
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
  _callbacks.stackOp(_callbacks.user, FUGRO_G2O_STACK_PUSH);
}

void Vertex::pop()
{
  _callbacks.stackOp(_callbacks.user, FUGRO_G2O_STACK_POP);
  updateCache();
}

void Vertex::discardTop()
{
  _callbacks.stackOp(_callbacks.user, FUGRO_G2O_STACK_DISCARD_TOP);
}

int Vertex::stackSize() const
{
  return _callbacks.stackSize(_callbacks.user);
}

void Vertex::oplusImpl(const double* update)
{
  _callbacks.oplus(_callbacks.user, update);
}

void Vertex::addOplusDelta(int index)
{
  _callbacks.jacobianStep(_callbacks.user, index, 1);
  updateCache();
}

void Vertex::subtractOplusDelta(int index)
{
  _callbacks.jacobianStep(_callbacks.user, index, -1);
  updateCache();
}

void Vertex::setToOriginImpl()
{
}

bool Vertex::read(std::istream&)
{
  return true;
}

bool Vertex::write(std::ostream& os) const
{
  return os.good();
}
