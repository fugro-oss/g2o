#include "IterationActions.h"

using namespace Fugro::G2O;

PreIterationAction::PreIterationAction(Graph^ graph) :
_graph(graph)
{

}

g2o::HyperGraphAction* PreIterationAction::operator()(const g2o::HyperGraph*, Parameters* parameters)
{
  ParametersIteration* iterationParameters = static_cast<ParametersIteration*>(parameters);
  _graph->OnIterationStarting(iterationParameters->iteration);

  return this;
}

PostIterationAction::PostIterationAction(Graph^ graph) :
_graph(graph)
{

}

g2o::HyperGraphAction* PostIterationAction::operator()(const g2o::HyperGraph*, Parameters* parameters)
{
  ParametersIteration* iterationParameters = static_cast<ParametersIteration*>(parameters);
  _graph->OnIterationFinished(iterationParameters->iteration);

  return this;
}