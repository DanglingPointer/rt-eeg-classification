using Processing.Double;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RTGui
{
    public enum NeuralNetworkType
    {
        BackPropagating, CascadeCorrelation
    }
    public class ClassifierAdapter
    {
        private bool _ready;
        private readonly Classifier _classifier;
        private object _inputDataLock = new object();
        private readonly List<double> _inputData;
        private volatile int _mode; // -1 = idle, -2 = classifying new data
        private volatile int _examplesCollected;

        /// <summary>
        /// Arg = mode with highest probability
        /// </summary>
        public event Action<int> SampleClassified;

        /// <summary>
        /// Number of modes to classify = output layer size in a NN
        /// </summary>
        /// <param name="type"></param>
        /// <param name="inputSize"></param>
        /// <param name="modeCount"></param>
        public ClassifierAdapter(int modeCount, int inputSize = 50)
        {
            InputSize = inputSize;
            ModeCount = modeCount;
            _inputData = new List<double>();
            _classifier = new Classifier();
            
            _mode = -1;
            _ready = false;
            _examplesCollected = 0;
            DataManager.Current.SampleAnalysed += OnSampleAnalysed;
        }
        public void SetupNetwork(NeuralNetworkType type)
        {
            NetworkType = type;
            if (type == NeuralNetworkType.BackPropagating)
                _classifier.CreateFixedSizeNetwork(InputSize * 8, InputSize * 4, ModeCount, 3); // 2 hidden layers + 1 output layer
            else if (type == NeuralNetworkType.CascadeCorrelation)
                _classifier.CreateCascadeNetwork(InputSize * 8, ModeCount);
            _ready = true;
        }
        public NeuralNetworkType NetworkType
        { get; private set; }
        public int ExamplesCollected
        {
            get => _examplesCollected / 3; // 2 for training, 1 for validation
        }
        /// <summary>
        /// Input vector size per channel
        /// </summary>
        public int InputSize
        { get; }
        /// <summary>
        /// Output vector size
        /// </summary>
        public int ModeCount
        { get; }
        public void StartCollecting(int mode)
        {
            if (!_ready)
                throw new InvalidOperationException("Neural network not setup");
            _examplesCollected = 0;
            lock (_inputDataLock) {
                _inputData.Clear();
            }
            _mode = mode;
        }
        public void StartClassifying()
        {
            if (!_ready)
                throw new InvalidOperationException("Neural network not setup");
            lock (_inputDataLock) {
                _inputData.Clear();
            }
            _mode = -2;
        }
        public void Stop()
        {
            _mode = -1;
        }
        public void Unsubscrube()
        {
            DataManager.Current.SampleAnalysed -= OnSampleAnalysed;
        }
        public async Task TrainAsync()
        {
            if (!_ready)
                throw new InvalidOperationException("Neural network not setup");
            else if (_examplesCollected < 3)
                throw new InvalidOperationException("Too few training data available");
            await _classifier.TrainAsync();
        }

        private void OnSampleAnalysed(IHilbertSpectrum hs, int channel)
        {
            if (_mode == -1) {
                return;
            }

            if (channel == 0) {
                lock (_inputDataLock) {
                    _inputData.Clear();
                }
            }
            double minFreq = hs.MinFrequency;
            double interval = (hs.MaxFrequency - minFreq) / InputSize;
            lock (_inputDataLock) {
                for (int i = 0; i < InputSize; ++i) {
                    double w = minFreq + i * interval;
                    _inputData.Add(hs.ComputeMarginalAt(w));
                }
            }

            if (channel == 7 && _inputData.Count == InputSize * 8) {
                double[] output = new double[ModeCount];

                if (_mode == -2) {
                    // classify unkonown data
                    double[] copy;
                    lock (_inputDataLock) {
                        copy = _inputData.ToArray();
                    }
                    _classifier.Classify(copy, output);

                    int maxIndex = 0;
                    double maxVal = output[0];
                    for (int i = 1; i < output.Length; ++i) {
                        if (output[i] > maxVal) {
                            maxVal = output[i];
                            maxIndex = i;
                        }
                    }
                    SampleClassified?.Invoke(maxIndex);
                }
                else {
                    // add training example
                    output[_mode] = 1.0;
                    lock (_inputDataLock) {
                        _classifier.AddExample(_inputData.ToArray(), output);
                    }
                    _examplesCollected++;
                }
            }
        }
    }
}
