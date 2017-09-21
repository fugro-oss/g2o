# General Graph Optimization in .Net
A .Net wrapper for the G2O (graph-based optimization) library

## Get Started
This project can be build by using Visual Studio 2013 (or later). Included in the repository are unit tests that can serve as examples.

### C# example
```c#
var state = new State(0.0);
var measurement = new Measurement(state, 3.0);

var graph = new Graph();
graph.AddObservation(measurement);
graph.Optimize(); 

Console.WriteLine(state.Estimate)   // Should output 3.0

class State : State<double>
{
    public State(double initialValue) 
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

    public Measurement(State state, double measurement, double sd = 1.0)
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
For easy compilation, we included necessary parts of [G2O](https://github.com/RainerKuemmerle/g2o) and [Eigen](http://eigen.tuxfamily.org) in the repository.

## License
This library is licensed under the MIT License