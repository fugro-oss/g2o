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

            Assert.That(graph.Observations.OfType<Measurement>().Count(), Is.EqualTo(1));
            Assert.That(graph.States.OfType<State>().Count(), Is.EqualTo(1));
            Assert.That(measurement.States.OfType<State>().Count(), Is.EqualTo(1));
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

            Assert.That((double)state, Is.EqualTo(3.0).Within(0.0001));
            Assert.That(iteration, Is.GreaterThan(0));
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
            Assert.That((double)state, Is.GreaterThan(3.5));
            Assert.That((double)state, Is.LessThan(4.0));
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

            Assert.That(iteration, Is.EqualTo(0));
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

                Assert.That(measurement.SquaredError, Is.EqualTo(0.0).Within(1.0e-5));
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

                Assert.That(graph.SquaredError, Is.EqualTo(sum).Within(1.0e-10));
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
            Assert.That((double)state, Is.EqualTo(-101.0).Within(0.0001));
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
