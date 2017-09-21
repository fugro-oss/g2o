#pragma once

#include "gcroot_weak.h"

#pragma unmanaged
#include "g2o/core/robust_kernel.h"
#pragma managed

namespace Fugro
{
  namespace G2O
  {
    using namespace Eigen;

    ref class Observation;

    class RobustKernel :public g2o::RobustKernel
    {
    private:
      gcroot_weak<Observation^> _observation;

    public:
      RobustKernel(Observation^ observation) :
        _observation(observation)
      {

      }

      void robustify(double squaredError, Vector3d& rho) const
      {
        rho[2] = 0.0;
        _observation->Robustify(squaredError, rho[0], rho[1]);
      }
    };
  }
}