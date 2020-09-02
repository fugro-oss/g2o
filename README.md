# General Graph Optimization in .Net
A .Net wrapper for the G2O (graph-based optimization) library

## Get Started
Clone this project and build it by using Visual Studio 2017. Reference Fugro.G2O.dll in your own project and you are ready to go. The repository includes unit tests which can be run by using NUnit.

### C# example
1. Create a class for each type of state you have. Derive from State<T> and implement the OnUpdate function. Provide an initial value and the dimension of the state to the base class. Next instantiate it for each state you want to estimate.
2. Create a class for each kind of measurement you have. Derive from Observation and implement the OnComputeError function. The base class needs the sd's of the measurement and all the states its dependent on. Next instantiate it for each observation you have.
3. Now instantiate a Graph, add all the obsevations and call Optimize(). This will optimize the graph and estimate the states.

```c#
var state = new State(0.0);
var measurement = new Measurement(3.0, state);

var graph = new Graph();
graph.AddObservation(measurement);
graph.Optimize(); 

// Should output 3.0
Console.WriteLine(state.Estimate);

class State : State<double>
{
    public State(double initialValue = 0.0) 
        : base(initialValue, 1)
    { }

    protected override double OnUpdate(double[] delta)
    {
        return Estimate + delta[0];
    }
}

class Measurement : Observation
{
    private readonly double m_Measurement;
    private readonly State m_State;

    public Measurement(double measurement, State state, double sd = 1.0)
        : base(new[] { sd }, state)
    {
        m_State = state;
        m_Measurement = measurement;
    }

    protected override double[] OnComputeError()
    {
        var error = m_Measurement - m_State;

        return new[] { error };
    }
}
```

## Externals
For easy compilation, we included necessary parts of [G2O](https://github.com/RainerKuemmerle/g2o), 
[Eigen](http://eigen.tuxfamily.org) and [CSPARSE](https://people.sc.fsu.edu/~jburkardt/c_src/csparse/csparse.html) 
in the repository. 

## License
A built version of Fugro.G2O contains components from the following sources:

* Fugro.G2O: MIT
* G2O: BSD 2-Clause
* Eigen: MPL 2, with parts LGPL-2.1
* csparse: LGPL-2.1+

See [LICENSE.txt] for details. 