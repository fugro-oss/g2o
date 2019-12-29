using System;
using NUnit.Framework;

namespace Fugro.G2O.Test
{
    [TestFixture]
    internal sealed class PrecisionMatrixTest
    {
        [Test]
        public void OneRayPointPrecisionMatrixTest()
        {
            var center = new Point(1000, 0);

            var angleStandardDeviation = Math.Atan(0.25 / 1000);
            const double expectedYVariance = 0.25 * 0.25;
            const double expectedXVariance = 1.0;

            var graph = new Graph();
            var point2DState = new Point2DState(new Point(1001.7, 0.1));
            graph.AddObservation(new VectorFromOriginObservation(new Point(0, 0), 0, point2DState, angleStandardDeviation));
            graph.AddObservation(new Point2DObservation(center, new[] { 1.0, 0.0, 0.0, .001 }, point2DState));

            graph.Optimize(20);

            var precisionMatrix = point2DState.GetPrecisionMatrix();

            Assert.AreEqual(point2DState.Estimate.X, center.X, 1e-12);
            Assert.AreEqual(point2DState.Estimate.Y, center.Y, 1e-12);

            Assert.AreEqual(0, precisionMatrix[1], 1e-12);
            Assert.AreEqual(0, precisionMatrix[2], 1e-12);

            Assert.AreEqual(1 / expectedXVariance, precisionMatrix[0], 1e-4);
            Assert.AreEqual(1 / expectedYVariance + 0.001, precisionMatrix[3], 1e-6);
        }

        [Test]
        public void TwoRaysPrecisionMatrixTest()
        {
            var center = new Point(1000, 1000);

            var angleStandardDeviation = Math.Atan(0.25 / 1000);
            const double expectedVariance = 0.25 * 0.25;

            var graph = new Graph();
            var point2DState = new Point2DState(center + new Vector(2.3, -3.4));

            graph.AddObservation(new VectorFromOriginObservation(new Point(1000, 0), 0.5 * Math.PI, point2DState, angleStandardDeviation));
            graph.AddObservation(new VectorFromOriginObservation(new Point(0, 1000), 0, point2DState, angleStandardDeviation));

            graph.Optimize(20);

            Assert.AreEqual(point2DState.Estimate.X, center.X, 1e-12);
            Assert.AreEqual(point2DState.Estimate.Y, center.Y, 1e-12);

            var precisionMatrix = point2DState.GetPrecisionMatrix();

            Assert.AreEqual(0, precisionMatrix[1], 1e-12);
            Assert.AreEqual(0, precisionMatrix[2], 1e-12);

            Assert.AreEqual(1 / expectedVariance, precisionMatrix[0], 1e-3);
            Assert.AreEqual(1 / expectedVariance, precisionMatrix[3], 1e-3);
        }

        [Test]
        public void ThreeRayPrecisionMatrixTest()
        {
            var center = new Point(0, 0);

            var angleStandardDeviation = Math.Atan(0.25 / 1000);
            const double expectedVariance = 0.25 * 0.25;

            var graph = new Graph();
            var point2DState = new Point2DState(center + new Vector(2.3, -3.4));

            graph.AddObservation(new VectorFromOriginObservation(new Point(1000 / Math.Sqrt(2), 1000 / Math.Sqrt(2)), -0.75 * Math.PI, point2DState, angleStandardDeviation));
            graph.AddObservation(new VectorFromOriginObservation(new Point(-1000 / Math.Sqrt(2), 1000 / Math.Sqrt(2)), -0.25 * Math.PI, point2DState, angleStandardDeviation));
            graph.AddObservation(new VectorFromOriginObservation(new Point(-1000 / Math.Sqrt(2), -1000 / Math.Sqrt(2)), 0.25 * Math.PI, point2DState, angleStandardDeviation));

            graph.Optimize(20);

            var precisionMatrix = point2DState.GetPrecisionMatrix();

            var rotationAngle = 0.25 * Math.PI;
            double cosA = Math.Cos(rotationAngle);
            double sinA = Math.Sin(rotationAngle);
            var r = new[] { cosA, -sinA, cosA, sinA };

            var covariance = Inverse2By2(precisionMatrix);
            var rotatedCovariance = Multiply2By2(Multiply2By2(r, covariance), Transpose2By2(r));
            var rotatedPrecisionMatrix = Inverse2By2(rotatedCovariance);

            Assert.AreEqual(point2DState.Estimate.X, center.X, 1e-12);
            Assert.AreEqual(point2DState.Estimate.Y, center.Y, 1e-12);

            Assert.AreEqual(0, rotatedPrecisionMatrix[1], 1e-3);
            Assert.AreEqual(0, rotatedPrecisionMatrix[2], 1e-3);

            Assert.AreEqual(2 / expectedVariance, rotatedPrecisionMatrix[0], 1e-2);
            Assert.AreEqual(1 / expectedVariance, rotatedPrecisionMatrix[3], 1e-2);
        }

        private static double[] Inverse2By2(double[] data)
        {
            double determinant = data[0] * data[3] - data[1] * data[2];

            double inverseDeterminant = 1.0 / determinant;

            return new[]
            {
                data[3] * inverseDeterminant,
                -data[1] * inverseDeterminant,
                -data[2] * inverseDeterminant,
                data[0] * inverseDeterminant
            };
        }

        private static double[] Transpose2By2(double[] data)
        {
            return new[]
            {
                data[0],
                data[2],
                data[1],
                data[3]
            };
        }

        private static double[] Multiply2By2(double[] data1, double[] data2)
        {
            return new[]
            {
                data1[0] * data2[0] + data1[1] * data2[2],
                data1[0] * data2[1] + data1[1] * data2[3],
                data1[2] * data2[0] + data1[3] * data2[2],
                data1[2] * data2[1] + data1[3] * data2[3],
            };
        }

        private class VectorFromOriginObservation : Observation
        {
            private readonly Point m_Origin;
            private readonly double m_Angle;
            private readonly Point2DState m_Point;

            public VectorFromOriginObservation(Point origin, double angleInRadians, Point2DState point, double standardDeviationInRadians)
                : base(new[] { standardDeviationInRadians }, point)
            {
                m_Origin = origin;
                m_Angle = angleInRadians;
                m_Point = point;
            }

            protected override double[] OnComputeError()
            {
                var vector = m_Point.Estimate - m_Origin;

                double angle = Math.Atan2(vector.Y, vector.X);

                var error = (angle - m_Angle);

                return new[] { error };
            }
        }

        private class Point2DObservation : Observation
        {
            private readonly Point m_Location;
            private readonly Point2DState m_Point2DState;

            public Point2DObservation(Point location, double[] stdevs, Point2DState point2DState)
                : base(2, stdevs, point2DState)
            {
                m_Location = location;
                m_Point2DState = point2DState;
            }

            protected override double[] OnComputeError()
            {
                var error = m_Point2DState.Estimate - m_Location;

                return new[] { error.X, error.Y };
            }
        }

        private class Point2DState : State<Point>
        {
            public Point2DState(Point initial)
                : base(initial, 2)
            {
            }

            protected override Point OnUpdate(double[] delta)
            {
                return Estimate + new Vector(delta[0], delta[1]);
            }
        }

        private struct Point
        {
            public double X { get; private set; }
            public double Y { get; private set; }

            public Point(double x, double y)
            {
                X = x;
                Y = y;
            }

            public static Vector operator -(Point point1, Point point2)
            {
                return new Vector(point1.X - point2.X, point1.Y - point2.Y);
            }

            public static Point operator +(Point point, Vector vector)
            {
                return new Point(point.X + vector.X, point.Y + vector.Y);
            }
        }

        private struct Vector
        {
            public double X { get; private set; }
            public double Y { get; private set; }

            public Vector(double x, double y)
            {
                X = x;
                Y = y;
            }
        }
    }
}