using Communication;
using Processing;
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
        public static ConcurrentDictionary<int, ChartPageViewModel> ChartPagesStates
        { get; } = new ConcurrentDictionary<int, ChartPageViewModel>();

        public static DataManager Current
        { get; } = new DataManager();


        public event Action<IHilbertSpectrumDouble, int> SampleAnalysed;

        private int _sampleSize;
        private int _ensembleCount;
        private readonly object _sampleLock = new object();
        private List<BciData> _sample;

        private readonly ConcurrentQueue<List<BciData>> _queue;
        private volatile bool _queueStopped;

        private DataManager()
        {
            _sample = new List<BciData>();
            _sampleSize = 500;
            _ensembleCount = 100;

            _queue = new ConcurrentQueue<List<BciData>>();
            _queueStopped = false;
        }
        public void Start()
        {
            Task.Run(async () => {
                bool queueEmpty = false;
                int queueLastLength = 0;
                int queueLengthSlope = 0;

                while (!(queueEmpty && _queueStopped)) {
                    List<BciData> sample;
                    queueEmpty = !_queue.TryDequeue(out sample);

                    if (!queueEmpty) {
                        // Update error and d_error
                        queueLengthSlope = _queue.Count - queueLastLength;
                        queueLastLength = _queue.Count;
                        AdjustParameters(queueLastLength, queueLengthSlope);

                        // Analyse data and fire events
                        double[] xValues = new double[sample.Count];
                        for (int i = 0; i < sample.Count; ++i)
                            xValues[i] = i;

                        Parallel.For(0, 8, async (channel) => {
                            double[] yValues = new double[sample.Count];
                            for (int i = 0; i < sample.Count; ++i)
                                yValues[i] = sample[i].ChannelData[channel];

                            var decomp = await Emd.EnsembleDecomposeAsync(xValues, yValues, 0.05, _ensembleCount);
                            var spectrum = await Hsa.GetHilbertSpectrumAsync(decomp, 1.0);

                            SampleAnalysed?.Invoke(spectrum, channel);
                        });
                    }
                    else {
                        queueLengthSlope = 0;
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
            List<BciData> sample = null;
            lock (_sampleLock) {
                _sample.Add(data);
                if (_sample.Count >= _sampleSize) {
                    sample = _sample;
                    _sample = new List<BciData>();
                }
            }
            if (sample != null)
                _queue.Enqueue(sample);
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
            int u = 10 * e + 20 * de; // PD-controller

            Debug.WriteLine($"PD-controller output = {u}");

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
