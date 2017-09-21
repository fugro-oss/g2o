#include "Stdafx.h"
#include "Observation.h"
#include "RobustKernel.h"

using namespace System;
using namespace Fugro::G2O;

Observation::Observation(int dimension, ... array<BaseState^>^ states) : _states(states)
{
  if (states == nullptr)
  {
    throw gcnew ArgumentNullException("states");
  }

  if (dimension < 1)
  {
    throw gcnew ArgumentException("Dimension should be greater than 1.", "dimension");
  }

  MatrixXd information = MatrixXd::Identity(dimension, dimension);

  InitializeEge(information);
}

Observation::Observation(array<double>^ stdevs, ... array<BaseState^>^ states) : _states(states)
{
  if (states == nullptr)
  {
    throw gcnew ArgumentNullException("states");
  }

  if (stdevs == nullptr)
  {
    throw gcnew ArgumentNullException("stdevs");
  }

  if (stdevs->Length < 1)
  {
    throw gcnew ArgumentException("Standard deviation length should be greater than 1.", "stdevs");
  }

  VectorXd informationVector(stdevs->Length);
  for (int i = 0; i < informationVector.size(); i++)
  {
    informationVector(i) = 1 / (stdevs[i] * stdevs[i]);
  }

  InitializeEge(informationVector.asDiagonal());
}

Observation::Observation(int dimension, array<double>^ precisionMatrix, ... array<BaseState^>^ states) : _states(states)
{
  if (states == nullptr)
  {
    throw gcnew ArgumentNullException("states");
  }

  if (precisionMatrix == nullptr)
  {
    throw gcnew ArgumentNullException("precisionMatrix");
  }

  if (dimension < 1)
  {
    throw gcnew ArgumentException("Dimension should be greater than 1.", "dimension");
  }

  if (dimension * dimension != precisionMatrix->Length)
  {
    throw gcnew ArgumentException("Precision matrix should be a square", "precisionMatrix");
  }

  MatrixXd precision(dimension, dimension);
  for (int i = 0; i < dimension; i++)
  {
    for (int j = 0; j < dimension; j++)
    {
      precision(i, j) = precisionMatrix[dimension * i + j];
    }
  }

  InitializeEge(precision);
}

void Observation::InitializeEge(MatrixXd information)
{
  edge = new Edge(information, this);
  edge->resize(_states->Length);

  _robustKernel = new RobustKernel(this);
  edge->setRobustKernel(_robustKernel);

  for (int i = 0; i < _states->Length; i++)
  {
    edge->setVertex(i, _states[i]->vertex);
  }
}

Observation::!Observation()
{
  delete edge;
  delete _robustKernel;
}

double Observation::SquaredError::get()
{
  return edge->chi2();
}

IReadOnlyList<BaseState^>^ Observation::States::get()
{
  return gcnew List<BaseState^>(_states);
}

array<double>^ Observation::ComputeError()
{
  return OnComputeError();
}