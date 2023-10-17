# General Graph Optimization in .Net
A .Net wrapper for the G2O (graph-based optimization) library

## Get Started
Reference the [Fugro.G2O](https://www.nuget.org/packages/Fugro.G2O) NuGet package

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

## Building from scratch
Clone this project and build it by using Visual Studio 2017. The repository includes unit tests which can be run by using NUnit.

For easy compilation, we included necessary parts of [G2O](https://github.com/RainerKuemmerle/g2o), 
[Eigen](http://eigen.tuxfamily.org) and [CSPARSE](https://people.sc.fsu.edu/~jburkardt/c_src/csparse/csparse.html) 
in the repository. 


## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our process for submitting pull requests to us, and please ensure
you follow the [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).

## Authors

See the list of [contributors](CONTRIBUTORS) who participated in this project.

## License
A built version of Fugro.G2O contains components from the following sources:

* Fugro.G2O: MIT
* G2O: BSD 2-Clause
* Eigen: MPL 2, with parts LGPL-2.1
* csparse: LGPL-2.1+

See [LICENSE] for details. 

# Packaging
GitHub builds a package for .NetFramework 4.8, .NetCoreApp 3.1 and .Net 6.0. The solution is built 3 times, for 3 configurations: `net48-release`, `netcoreapp31-release` and `net6-release`. Only for `net6-release` the test project is built and run as it seems you can't set TargetFramework to be configuration-specific, and multitarget doesn't work as G2O is built in 3 separate builds.

### Ijwhost.dll
This dll is copied into the package, so it is distributed when another package references not this package, but yet another package that does... Be warned this may cause problems when a conflicting version of Ijwhost.dll is needed by other of your dependencies.