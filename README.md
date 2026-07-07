# General Graph Optimization in .NET

A .NET wrapper for the [g2o](https://github.com/RainerKuemmerle/g2o) (graph-based optimization) library.
Define your states and observations in plain C#, add them to a graph and optimize.

Runs on **Windows and Linux** (e.g. Docker, fly.io, AWS): the g2o C++ code is compiled into a small
native library (`fugro_g2o`) with a plain C interface, and the managed `Fugro.G2O` assembly talks to it
through P/Invoke. There is no C++/CLI involved, so the same package works on any .NET 8+ runtime
(the managed assembly targets `netstandard2.0`).

## Get started

Reference the `Fugro.G2O` NuGet package and you are ready to go. The package contains the native
library for `win-x64` and `linux-x64`; the right one is picked automatically at runtime.

Releases are published by pushing a version tag (e.g. `git tag v3.0.0 && git push origin v3.0.0`),
which triggers the release workflow: it builds and tests on both platforms, packs the `.nupkg`
with the tag version and attaches it to a GitHub Release for direct download.

To consume it, download the `.nupkg` into a folder and register that folder as a package source:

```bash
dotnet nuget add source ./packages -n local
dotnet add package Fugro.G2O
```

Regular pushes to `master` additionally upload the package as a short-lived Actions artifact
(`package`), which is handy for trying out unreleased builds.

### C# example

1. Create a class for each type of state you have. Derive from `State<T>` and implement the `OnUpdate` function. Provide an initial value and the dimension of the state to the base class. Next instantiate it for each state you want to estimate.
2. Create a class for each kind of measurement you have. Derive from `Observation` and implement the `OnComputeError` function. The base class needs the sd's of the measurement and all the states it depends on. Next instantiate it for each observation you have.
3. Now instantiate a `Graph`, add all the observations and call `Optimize()`. This will optimize the graph and estimate the states.

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

## Building from source

Two build steps: the native library (CMake) and the managed library (dotnet).

### Windows

Requires Visual Studio 2022 (C++ workload), CMake and the .NET 10 SDK.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
dotnet test Fugro/Test/Fugro.G2O.Test.csproj -c Release
```

### Linux

Requires g++, make, CMake and the .NET 10 SDK.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
dotnet test Fugro/Test/Fugro.G2O.Test.csproj -c Release
```

### Docker

The repository `Dockerfile` builds everything and runs the test suite on Linux:

```bash
docker build -t fugro-g2o .
docker run --rm fugro-g2o
```

To extract just the Linux native library (e.g. for packaging):

```bash
docker build --target native --output type=local,dest=build/bin .
```

The native binary always ends up in `build/bin`; the .NET projects copy it to their output
directory from there.

## Repository layout

* `External/` — bundled sources of [g2o](https://github.com/RainerKuemmerle/g2o), [Eigen](http://eigen.tuxfamily.org) and [CSparse](https://people.sc.fsu.edu/~jburkardt/c_src/csparse/csparse.html)
* `Native/` — a thin C++ shim exposing a C API over g2o; managed state/observation logic is reached through callbacks
* `Fugro/G2O.NET/` — the managed `Fugro.G2O` library (pure C#, `netstandard2.0`)
* `Fugro/Test/` — NUnit test suite (`net10.0`)
* `CMakeLists.txt` — builds `External` + `Native` into the `fugro_g2o` shared library

## Packaging

GitHub Actions builds and tests on Linux and Windows, then packs a single `Fugro.G2O` NuGet package
containing the managed assembly plus `runtimes/win-x64/native/fugro_g2o.dll` and
`runtimes/linux-x64/native/libfugro_g2o.so`. .NET Core / .NET 5+ consumers get the correct native
library automatically. (Classic .NET Framework projects would need to copy `fugro_g2o.dll` next to
their executable manually.)

## License

A built version of Fugro.G2O contains components from the following sources:

* Fugro.G2O: MIT
* G2O: BSD 2-Clause
* Eigen: MPL 2, with parts LGPL-2.1
* csparse: LGPL-2.1+

See [LICENSE.txt](LICENSE.txt) for details.
