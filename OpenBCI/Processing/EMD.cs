using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Spline;

namespace Processing
{
    class MonotonicFunctionException : Exception
    { }

    class EnvelopeFinder
    {
        /// <summary>
        /// Uses cubic spline interpolation to calculate upper and lower envelopes at xValues
        /// </summary>
        /// <param name="xValues"></param>
        /// <param name="yValues"></param>
        public EnvelopeFinder(double[] xValues, double[] yValues)
        {
            if (xValues.Length != yValues.Length)
                throw new ArgumentException();
            
            // Find max and min points

            List<double> maxX = new List<double>(xValues.Length);
            List<double> maxY = new List<double>(xValues.Length);

            List<double> minX = new List<double>(xValues.Length);
            List<double> minY = new List<double>(xValues.Length);

            ZeroCrossingCount = 0;

            maxY.Add(yValues[0]);
            maxX.Add(xValues[0]);
            minY.Add(yValues[0]);
            minX.Add(xValues[0]);

            maxY.Add(yValues.Last());
            maxX.Add(xValues.Last());
            minY.Add(yValues.Last());
            minX.Add(xValues.Last());

            for (int i = 1; i < xValues.Length - 1; ++i) {
                if (yValues[i] > yValues[i - 1] && yValues[i] > yValues[i + 1]) {
                    maxY.Add(yValues[i]);
                    maxX.Add(xValues[i]);
                }
                else if (yValues[i] < yValues[i - 1] && yValues[i] < yValues[i + 1]) {
                    minY.Add(yValues[i]);
                    minX.Add(xValues[i]);
                }
                if ((yValues[i] < 0 && yValues[i + 1] >= 0)
                    || (yValues[i] > 0 && yValues[i + 1] <= 0)) {
                    ZeroCrossingCount++;
                }
            }
            if (yValues[0] == 0 || yValues[1] == 0) {
                ZeroCrossingCount++;
            }

            UpperExtremaCount = maxX.Count;
            LowerExtremaCount = minX.Count;

            if (UpperExtremaCount == 2 && LowerExtremaCount == 2)
                throw new MonotonicFunctionException();

            // Compute interpolations in parallell

            Parallel.Invoke(
                () => UpperEnvelope = CubicSpline.Compute(maxX.ToArray(), maxY.ToArray(), xValues),
                () => LowerEnvelope = CubicSpline.Compute(minX.ToArray(), minY.ToArray(), xValues)
            );
        }
        public double[] UpperEnvelope
        { get; private set; }
        public double[] LowerEnvelope
        { get; private set; }

        public int UpperExtremaCount
        { get; private set; }
        public int LowerExtremaCount
        { get; private set; }

        public int ZeroCrossingCount
        { get; private set; }
    }

    delegate bool SiftingStopCriterionDelegate(double[] lastYValues, double[] nextLastYValues, int zeroCrossingCount, int extremaCount);

    class Sifter
    {
        private readonly int _length;
        private readonly double[] _xValues;
        private readonly SiftingStopCriterionDelegate _stopCondition;

        private double[] _prevH;
        private double[] _newH;

        /// <summary>
        /// Decomposes yValues info IMF and Residue. If no IMF can be extracted, sets Imf to null
        /// </summary>
        /// <param name="xValues"></param>
        /// <param name="yValues"></param>
        /// <param name="stopCondition"></param>
        public Sifter(double[] xValues, double[] yValues, SiftingStopCriterionDelegate stopCondition)
        {
            _length = _xValues.Length;
            _xValues = xValues;
            _stopCondition = stopCondition;          

            _prevH = new double[_length];
            _newH = new double[_length];

            Array.Copy(yValues, _prevH, _length);

            try {
                SiftRecursively(); // returns when _newH is IMF

                Residue = _prevH;   // reuse _prevH to avoid allocating mem
                Imf = _newH;
                for (int i = 0; i < _length; ++i) {
                    Residue[i] = yValues[i] - _newH[i]; 
                }
            }
            catch (MonotonicFunctionException) {
                Residue = _prevH;
                Imf = null;
            }
        }
        private void SiftRecursively()
        {
            var envelopes = new EnvelopeFinder(_xValues, _prevH);
            for (int i = 0; i < _length; ++i) {
                double meanEnvelope = 0.5 * (envelopes.UpperEnvelope[i] + envelopes.LowerEnvelope[i]);
                _newH[i] = _prevH[i] - meanEnvelope;
            }

            if (!_stopCondition(_newH, _prevH, envelopes.ZeroCrossingCount, envelopes.UpperExtremaCount + envelopes.LowerExtremaCount)) {
                Array.Copy(_newH, _prevH, _length);
                SiftRecursively();
            }
        }

        public double[] Imf
        { get; private set; }

        public double[] Residue
        { get; private set; }
    }

    class EmdDecomposer : IImfDecomposition
    {
        /// <summary>
        /// Decomposes yValues into several IMF functions + 1 monotonic residue function
        /// </summary>
        /// <param name="xValues"></param>
        /// <param name="yValues"></param>
        public EmdDecomposer(double[] xValues, double[] yValues)
        {
            ImfFunctions = new List<double[]>();
            ResidueFunction = yValues;

            Sifter s;
            do {
                s = new Sifter(xValues, ResidueFunction, IsSiftingFinished);
                if (s.Imf != null)
                    ImfFunctions.Add(s.Imf);
                ResidueFunction = s.Residue;

            } while (s.Imf != null);
        }

        public IList<double[]> ImfFunctions
        { get; private set; }

        public double[] ResidueFunction
        { get; private set; }

        protected virtual bool IsSiftingFinished(double[] lastYValues, double[] nextLastYValues, int zeroCrossingCount, int extremaCount)
        {
            // standard deviation
            double sd = 0.0;
            for (int i = 0; i < lastYValues.Length; ++i) {
                double diff = nextLastYValues[i] - lastYValues[i];
                sd += (diff * diff) / (nextLastYValues[i] * nextLastYValues[i]);
            }
            return sd < 0.3;
        }
    }

    class EemdDecomposer : IImfDecomposition
    {
        public EemdDecomposer(double[] xValues, double[] yValues, int ensembleCount, double wnAmplitude = 0.5)
        {
            double[][] yValuesEnsembles = new double[ensembleCount][];

            Random r = new Random(Guid.NewGuid().GetHashCode());

            for (int i = 0; i < ensembleCount; ++i) {
                yValuesEnsembles[i] = new double[yValues.Length];
                for (int x = 0; x < yValues.Length; ++x) {
                    yValuesEnsembles[i][x] = yValues[x] + (r.NextDouble() - 0.5) * (2 * wnAmplitude);
                }
            }

            IList<double[]>[] imfEnsembles = new IList<double[]>[ensembleCount];

            Parallel.For(0, ensembleCount, (i) => {
                var decomposer = new EmdDecomposer(xValues, yValuesEnsembles[i]);
                imfEnsembles[i] = decomposer.ImfFunctions;
            });

            int maxImfCount = 0;
            foreach (IList<double[]> imfFunctions in imfEnsembles) {
                if (imfFunctions.Count > maxImfCount)
                    maxImfCount = imfFunctions.Count;
            }

            ImfFunctions = new double[maxImfCount][];

            Parallel.For(0, maxImfCount, (imfIndex) => {
                double[] resultingImf = new double[yValues.Length];
                int actualEnsembleCount = 0;

                foreach (IList<double[]> imfFunctions in imfEnsembles) {
                    if (imfIndex < imfFunctions.Count) {
                        actualEnsembleCount++;

                        int xIndex = 0;
                        foreach (double value in imfFunctions[imfIndex]) {
                            resultingImf[xIndex++] += value;
                        }

                    }
                }
                for (int i = 0; i < resultingImf.Length; ++i) {
                    resultingImf[i] /= actualEnsembleCount;
                }
                ImfFunctions[imfIndex] = resultingImf;
            });

            ResidueFunction = null;
        }

        public IList<double[]> ImfFunctions
        { get; private set; }

        public double[] ResidueFunction
        { get; private set; }
    }

    public interface IImfDecomposition
    {
        IList<double[]> ImfFunctions { get; }
        double[] ResidueFunction { get; }
    }

    public static class Emd
    {
        public static IImfDecomposition ComputeDecomposition(double[] xValues, double[] yValues)
        {
            return new EmdDecomposer(xValues, yValues);
        }
        public static IImfDecomposition ComputeEnsembleDecomposition(double[] xValues, double[] yValues, int ensembleCount = 10)
        {
            return new EemdDecomposer(xValues, yValues, ensembleCount);
        }
    }
}
