#include "graph.h"

#include "g2o/core/block_solver.h"
#include "g2o/core/optimization_algorithm_levenberg.h"
#include "g2o/solvers/csparse/linear_solver_csparse.h"
#include "g2o/stuff/misc.h"

using namespace g2o;
using namespace fugro::g2o_native;

HyperGraphAction* PreIterationAction::operator()(const HyperGraph*, Parameters* parameters)
{
  ParametersIteration* iterationParameters = static_cast<ParametersIteration*>(parameters);
  _graph->onIterationStarting(iterationParameters->iteration);

  return this;
}

HyperGraphAction* PostIterationAction::operator()(const HyperGraph*, Parameters* parameters)
{
  ParametersIteration* iterationParameters = static_cast<ParametersIteration*>(parameters);
  _graph->onIterationFinished(iterationParameters->iteration);

  return this;
}

Graph::Graph(void* user, fugro_g2o_iteration_cb preIteration, fugro_g2o_iteration_cb postIteration) :
  _stopFlag(false),
  _user(user),
  _preIteration(preIteration),
  _postIteration(postIteration)
{
  // Equivalent of the "lm_var" algorithm from the g2o factory: Levenberg-Marquardt
  // with a variable block size CSparse linear solver. Constructed directly to avoid
  // relying on static factory registration, which the linker may strip.
  auto linearSolver = g2o::make_unique<LinearSolverCSparse<BlockSolverX::PoseMatrixType>>();
  linearSolver->setBlockOrdering(false);
  _ownedAlgorithm = new OptimizationAlgorithmLevenberg(g2o::make_unique<BlockSolverX>(std::move(linearSolver)));

  setAlgorithm(_ownedAlgorithm);
  setForceStopFlag(&_stopFlag);

  _preAction = new PreIterationAction(this);
  _postAction = new PostIterationAction(this);

  addPreIterationAction(_preAction);
  addPostIterationAction(_postAction);
}

Graph::~Graph()
{
  // SparseOptimizer's destructor does not delete the algorithm because the
  // graph is compiled without implicit ownership of objects.
  delete _ownedAlgorithm;
  _ownedAlgorithm = nullptr;

  delete _preAction;
  delete _postAction;
}

void Graph::onIterationStarting(int iteration)
{
  if (_preIteration != nullptr && _preIteration(_user, iteration) != 0)
  {
    requestStop();
  }
}

void Graph::onIterationFinished(int iteration)
{
  if (_postIteration != nullptr && _postIteration(_user, iteration) != 0)
  {
    requestStop();
  }
}

int Graph::optimizeGraph(int maxIterations)
{
  _stopFlag = false;

  initializeOptimization();

  return optimize(maxIterations);
}
