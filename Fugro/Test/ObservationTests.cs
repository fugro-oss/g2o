using System;
using NUnit.Framework;

namespace Fugro.G2O.Test
{
    [TestFixture]
    internal sealed class ObservationTests
    {
        [Test]
        public void WhenObservationHasDimensionZero_ArgumentOutOfRangeExceptionsIsThrown()
        {
            var state = new State();

            Assert.Throws<ArgumentException>(() =>
            {
                var _ = new ObservationWithDimensionZero(state);
            });
        }

        [Test]
        public void WhenPrecisionMatrixIsNotSquare_ArgumentExceptionIsThrown()
        {
            var state = new State();

            Assert.Throws<ArgumentException>(() =>
            {
                var _ = new ObservationWithNonSquarePrecisionMatrix(state);
            });
        }

        [Test]
        public void WhenObservationHasWrongErrorLengt_InvalidOperationExceptionIsThrown()
        {
            var state = new State();

            var measurement = new ObservationWithWrongErrorLength(state);

            var graph = new Graph();
            graph.AddObservation(measurement);

            Assert.Throws<InvalidOperationException>(() =>
            {
                graph.Optimize();
            });
        }

        private class State : State<double>
        {
            public State()
                : base(0.0, 1)
            {
            }

            protected override double OnUpdate(double[] delta)
            {
                return Estimate + delta[0];
            }
        }

        private class ObservationWithDimensionZero : Observation
        {
            public ObservationWithDimensionZero(State state)
                : base(0, state)
            {
            }

            protected override double[] OnComputeError()
            {
                return new double[1];
            }
        }

        private class ObservationWithNonSquarePrecisionMatrix : Observation
        {
            public ObservationWithNonSquarePrecisionMatrix(State state)
                : base(1, new[] { 1.0, 0.0 }, state)
            {
            }

            protected override double[] OnComputeError()
            {
                return new double[1];
            }
        }

        private class ObservationWithWrongErrorLength : Observation
        {
            public ObservationWithWrongErrorLength(State state)
                : base(1, state)
            {
            }

            protected override double[] OnComputeError()
            {
                return new double[2];
            }
        }
    }
}