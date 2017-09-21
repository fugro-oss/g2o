#include "Stdafx.h"
#include "IterationEventArgs.h"

using namespace Fugro::G2O;

IterationEventArgs::IterationEventArgs(int iteration)
{
	this->iteration = iteration;
}

int IterationEventArgs::Iteration::get()
{
	return this->iteration;
}

bool IterationEventArgs::Cancel::get()
{
  return this->cancel;
}

void IterationEventArgs::Cancel::set(bool value)
{
  this->cancel = value;
}