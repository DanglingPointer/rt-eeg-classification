using Communication;
using Processing;
using Processing.Double;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RTGui
{
    public class DataManager
    {
        public static ConcurrentDictionary<int, RTAnalysisViewModel> ChartPagesStates
        { get; } = new ConcurrentDictionary<int, RTAnalysisViewModel>();
        public static string[] ActionNames
        { get; set; }

        public static DataManager Current
        { get; } = new DataManager();

        public const double ScaleFactor = 4.5d / 24.0d / 8388607.0d;


        public event Action<IHilbertSpectrum, int> SampleAnalysed;

        private int _sampleSize;
        private int _ensembleCount;
        private readonly object _sampleLock = new object();
        private List<BciData> _sample;
        private List<BciData> _lastSample;

        private readonly ConcurrentQueue<List<BciData>> _queue;
        private volatile bool _queueStopped;

        private DataManager()
        {
            _sample = new List<BciData>();
            _lastSample = null;
            _sampleSize = 500;
            _ensembleCount = 500;

            _queue = new ConcurrentQueue<List<BciData>>();
            _queueStopped = true;
        }
        public void Start()
        {
            _queueStopped = false;
            Task.Run(async () => {
                bool queueEmpty = false;
                int queueLastLength = 0;
                int queueLengthSlope = 0;

                while (!(queueEmpty && _queueStopped)) {
                    List<BciData> sample;
                    queueEmpty = !_queue.TryDequeue(out sample);

                    if (!queueEmpty) {
                        Debug.WriteLine("Processing queue...");
                        // Update error and d_error
                        queueLengthSlope = _queue.Count - queueLastLength;
                        queueLastLength = _queue.Count;
                        AdjustParameters(queueLastLength, queueLengthSlope);

                        // Analyse data and fire events
                        double[] xValues = new double[sample.Count];
                        for (int i = 0; i < sample.Count; ++i)
                            xValues[i] = i;

                        for (int channel = 0; channel < 8; ++channel) {
                            double[] yValues = new double[sample.Count];
                            for (int i = 0; i < sample.Count; ++i)
                                yValues[i] = sample[i].ChannelData[channel] * ScaleFactor;

                            var decomp = await Emd.EnsembleDecomposeAsync(xValues, yValues, 0.05, _ensembleCount);
                            var spectrum = await Hsa.GetHilbertSpectrumAsync(decomp, 1.0);
                            
                            SampleAnalysed?.Invoke(spectrum, channel);
                        }
                    }
                    else {
                        await Task.Delay(100);
                    }
                }
            });
        }
        public void Stop()
        {
            _queueStopped = true;
        }
        public void EnqueueData(BciData data)
        {
            if (_queueStopped)
                return;
            lock (_sampleLock) {
                _sample.Add(data);
                if (_sample.Count >= _sampleSize) {
                    _lastSample = _sample;
                    _sample = new List<BciData>();
                }
            }
            if (_sample.Count == 0) {
                _queue.Enqueue(_lastSample);
                Debug.WriteLine("Sample enqueued");
            }
        }
        public ClassifierAdapter Classifier
        { get; set; }
        /// <summary>
        /// Copy of previous sample, might be used for static analysis
        /// </summary>
        public BciData[] LastSample
        {
            get => _lastSample?.ToArray();
        }
        public int SampleSize
        {
            get => _sampleSize;
        }
        public int EnsembleCount
        {
            get => _ensembleCount;
        }
        private void AdjustParameters(int e, int de)
        {
            int u = 20 * e + 20 * de; // PD-controller

            Debug.WriteLine($"--- P-term = {10 * e}, D-term = {20 * de} ---");
            Debug.WriteLine($"PD-controller output = {u}\nEnsemble count = {_ensembleCount}\nSample size = {_sampleSize}");

            _ensembleCount -= u;
            if (_ensembleCount < 10) {
                _ensembleCount = 10;

                _sampleSize -= u;
                if (_sampleSize < 100)
                    Stop();
            }            
        }
    }
}
