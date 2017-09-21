#include "Stdafx.h"
#include "State.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace Fugro::G2O;

generic<class T>
State<T>::State(T estimate, int dimension) : _estimate(estimate)
{
  if (estimate == nullptr)
  {
    throw gcnew ArgumentNullException("estimate");
  }

  static int count = 0;
  
  vertex = new Vertex(dimension, this);
  vertex->setId(++count);
  vertex->updateCache();
  
  _backup = gcnew Stack<T>(1);
}

generic<class T>
State<T>::!State()
{
    delete vertex;
}

generic<class T>
T State<T>::Estimate::get()
{
  return _estimate;
}

generic<class T>
void State<T>::Oplus(const double* update)
{
  array<double>^ delta = gcnew array<double>(vertex->dimension());
  Marshal::Copy(IntPtr((void*)update), delta, 0, delta->Length);

  _estimate = OnUpdate(delta);
}

generic<class T>
void State<T>::AddOplusDelta(int index)
{
  _estimate = OnPositiveJacobianUpdate(index);
}

generic<class T>
void State<T>::SubtractOplusDelta(int index)
{
  _estimate = OnNegativeJacobianUpdate(index);
}

generic<class T>
T State<T>::OnPositiveJacobianUpdate(int index)
{
  array<double>^ deltas = gcnew array<double>(vertex->dimension());
  deltas[index] = Vertex::jacobianDelta();

  return OnUpdate(deltas);
}

generic<class T>
T State<T>::OnNegativeJacobianUpdate(int index)
{
  array<double>^ deltas = gcnew array<double>(vertex->dimension());
  deltas[index] = -Vertex::jacobianDelta();

  return OnUpdate(deltas);
}

generic<class T>
void State<T>::Push()
{
  _backup->Push(_estimate);
}

generic<class T>
void State<T>::Pop()
{
  _estimate = _backup->Pop();
}

generic<class T>
void State<T>::DiscardTop()
{
  _backup->Pop();
}

generic<class T>
int State<T>::GetStackSize()
{
  return _backup->Count;
}

generic<class T>
array<double>^ State<T>::GetPrecisionMatrix()
{
	if (vertex->fixed())
	{
		throw gcnew InvalidOperationException("Fixed state does not have a precision matrix.");
	}

	array<double>^ precisionVector = gcnew array<double>(vertex->dimension() * vertex->dimension());
	Marshal::Copy(IntPtr((void*)vertex->hessianData()), precisionVector, 0, precisionVector->Length);
	
  return precisionVector;
}