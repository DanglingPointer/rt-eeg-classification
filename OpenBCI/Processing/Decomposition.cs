//
//   Copyright 2017 Mikhail Vasilyev
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Spline;

/// <summary>
/// Implementation of Empirical Mode Decomposition, making use of parallel processing
/// </summary>
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

            maxY.Add(yValues.Last());
            maxX.Add(xValues.Last());
            minY.Add(yValues.Last());
            minX.Add(xValues.Last());

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

    public interface IImfDecomposition
    {
        IList<double[]> ImfFunctions { get; }

        double[] ResidueFunction { get; }
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
        /// <summary>
        /// Ensemble empirical mode decomposition. No residual function
        /// </summary>
        /// <param name="xValues"></param>
        /// <param name="yValues"></param>
        /// <param name="ensembleCount">Number of generated white noise ranges</param>
        /// <param name="wnAmplitude">Max white noise amplitude (must be positive)</param>
        public EemdDecomposer(double[] xValues, double[] yValues, int ensembleCount, double wnAmplitude)
        {
            double[][] yValuesEnsembles = new double[ensembleCount][];

            // Add white noise 

            Random r = new Random(Guid.NewGuid().GetHashCode());

            for (int i = 0; i < ensembleCount; ++i) {
                yValuesEnsembles[i] = new double[yValues.Length];
                for (int x = 0; x < yValues.Length; ++x) {
                    yValuesEnsembles[i][x] = yValues[x] + (r.NextDouble() - 0.5) * (2 * wnAmplitude);
                }
            }

            // Decompose the ensembles in parallell

            IList<double[]>[] imfEnsembles = new IList<double[]>[ensembleCount];

            Parallel.For(0, ensembleCount, (i) => {
                var myImfEnsembles = imfEnsembles;

                var decomposer = new EmdDecomposer(xValues, yValuesEnsembles[i]);
                myImfEnsembles[i] = decomposer.ImfFunctions;
            });

            int maxImfCount = 0;
            foreach (IList<double[]> imfFunctions in imfEnsembles) {
                if (imfFunctions.Count > maxImfCount)
                    maxImfCount = imfFunctions.Count;
            }

            // Compute average in parallell

            ImfFunctions = new double[maxImfCount][];

            Parallel.For(0, maxImfCount, (imfIndex) => {
                var myImfFunctions = ImfFunctions;

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
                myImfFunctions[imfIndex] = resultingImf;
            });

            ResidueFunction = null;
        }

        public IList<double[]> ImfFunctions
        { get; private set; }

        public double[] ResidueFunction
        { get; private set; }
    }

    public static class Emd
    {
        /// <summary>
        /// Decompose yValues into IMFs using Empirical Mode Decomposition
        /// </summary>
        /// <param name="xValues"></param>
        /// <param name="yValues"></param>
        /// <returns></returns>
        public static IImfDecomposition Decompose(double[] xValues, double[] yValues)
        {
            return new EmdDecomposer(xValues, yValues);
        }
        /// <summary>
        /// Decompose yValues into IMFs using Ensemble Empirical Mode Decomposition
        /// </summary>
        /// <param name="xValues"></param>
        /// <param name="yValues"></param>
        /// <param name="ensembleCount">Number of generated white noise ranges</param>
        /// <param name="noiseAmplitude">Max white noise amplitude (must be positive)</param>
        /// <returns></returns>
        public static IImfDecomposition EnsembleDecompose(double[] xValues, double[] yValues, 
            int ensembleCount = 100, double noiseAmplitude = 0.5)
        {
            return new EemdDecomposer(xValues, yValues, ensembleCount, noiseAmplitude);
        }
    }
}
