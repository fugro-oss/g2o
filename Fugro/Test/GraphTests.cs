using System;
using System.Linq;
using NUnit.Framework;

namespace Fugro.G2O.Test
{
    [TestFixture]
    internal sealed class GraphTests
    {
        [Test]
        public void WhenObservationIsAdded_StatesAndObservationsAreExposed()
        {
            var state = new State(-101.0);

            var measurement = new Measurement(state, 3.0);

            var graph = new Graph();
            graph.AddObservation(measurement);

            Assert.AreEqual(1, graph.Observations.OfType<Measurement>().Count());
            Assert.AreEqual(1, graph.States.OfType<State>().Count());
            Assert.AreEqual(1, measurement.States.OfType<State>().Count());
        }

        [Test]
        public void TestOptimizeSingleValue()
        {
            var state = new State(-101.0);

            var measurement = new Measurement(state, 3.0);

            var graph = new Graph();
            graph.AddObservation(measurement);
            var iteration = 0;
            graph.IterationStarting += (s, e) => iteration = e.Iteration;

            graph.Optimize();

            Assert.AreEqual(3.0, state, 0.0001);
            Assert.Greater(iteration, 0);
        }

        [Test]
        public void WhenTwoObservationsHaveDifferentSD_ResultIsWeighted()
        {
            var state = new State(-101.0);

            var measurement1 = new Measurement(state, 3.0, 1.0);
            var measurement2 = new Measurement(state, 6.0, 2.0);

            var graph = new Graph();
            graph.AddObservation(measurement1);
            graph.AddObservation(measurement2);
            var iteration = 0;
            graph.IterationStarting += (s, e) => iteration = e.Iteration;

            graph.Optimize();
            Assert.Greater(state, 3.5);
            Assert.Less(state, 4.0);
        }

        [Test]
        public void TestStopOptimization()
        {
            var state = new State(-101.0);

            var measurement = new Measurement(state, 3.0);

            var graph = new Graph();
            graph.AddObservation(measurement);

            var iteration = 10;
            graph.IterationStarting += (s, e) =>
            {
                e.Cancel = true;
                iteration = e.Iteration;
            };

            graph.Optimize();

            Assert.AreEqual(0, iteration);
        }

        [Test]
        public void TestSquaredError()
        {
            for (int _ = 0; _ < 1000; _++)
            {
                var state = new State(-101.0);

                var measurement = new Measurement(state, 3.0);

                var graph = new Graph();
                graph.AddObservation(measurement);

                graph.Optimize();

                // Squared error is accessing unmanaged memory. After the graph is collected, the observation shoud still be accessible.
                GC.Collect();

                Assert.AreEqual(0.0, measurement.SquaredError, 1.0e-5);
            }
        }

        [Test]
        public void SquaredErrorOfGraph_IsSumOfObservations()
        {
            var state = new State(-101.0);

            var measurement1 = new Measurement(state, 3.0, 1.0);
            var measurement2 = new Measurement(state, 6.0, 2.0);

            var graph = new Graph();
            graph.AddObservation(measurement1);
            graph.AddObservation(measurement2);
            graph.IterationStarting += (s, e) =>
            {
                var sum = measurement1.SquaredError + measurement2.SquaredError;

                Assert.AreEqual(sum, graph.SquaredError, 1.0e-10);
            };

            graph.Optimize();
        }

        [Test]
        public void WhenStateIsFixed_StateDoesNotChange()
        {
            var state = new State(-101.0)
            {
                Fixed = true
            };

            var measurement = new Measurement(state, 3.0);

            var graph = new Graph();
            graph.AddObservation(measurement);

            graph.Optimize();
            Assert.AreEqual(-101.0, state, 0.0001);
        }

        [Test]
        public void WhenGettingPrecisionMatrixFromFixedState_ItThrowsMeaningfulException()
        {
            var state = new State(-101.0)
            {
                Fixed = true
            };

            var measurement = new Measurement(state, 3.0);

            var graph = new Graph();
            graph.AddObservation(measurement);

            graph.Optimize();

            Assert.Throws<InvalidOperationException>(() =>
            {
                state.GetPrecisionMatrix();
            });
        }

        private class State : State<double>
        {
            public State(double initialValue)
                : base(initialValue, 1)
            {
            }

            protected override double OnUpdate(double[] delta)
            {
                return Estimate + delta[0];
            }
        }

        private class Measurement : Observation
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
    }
}
