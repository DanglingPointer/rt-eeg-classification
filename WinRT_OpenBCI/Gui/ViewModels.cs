using Communication;
using LiveCharts;
using LiveCharts.Uwp;
using Processing;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using Windows.Foundation;
using Windows.UI.Core;
using Windows.UI.Popups;

namespace Gui
{
    /// <summary>
    /// Helper class for MVVM
    /// </summary>
    class RelayCommand : ICommand
    {
        Action<object> _execute;
        Func<object, bool> _canExecute;

        public event EventHandler CanExecuteChanged;
        public RelayCommand(Action<object> execute, Func<object, bool> canExecute)
        {
            _execute = execute;
            _canExecute = canExecute;
        }
        public RelayCommand(Action<object> execute)
        {
            _execute = execute;
            _canExecute = (o) => true;
        }
        public void Execute(object o)
        {
            _execute(o);
        }
        public bool CanExecute(object o)
        {
            return _canExecute(o);
        }
        public void ExecuteChanged(object sender, EventArgs e)
        {
            CanExecuteChanged?.Invoke(sender, e);
        }
    }

    public class AnalysisPageViewModel : INotifyPropertyChanged
    {
        private IList<KeyValuePair<int, double>> _dataSeries;
        private IList<KeyValuePair<int, double>> _amplitudes;
        private IList<KeyValuePair<int, double>> _phases;
        private IList<KeyValuePair<int, double>> _frequencies;

        private readonly int _channelIndex;
        private int _chartIndex;
        private CoreDispatcher _dispatcher;
        private double[] _channelData;


        public event PropertyChangedEventHandler PropertyChanged;
        
        public AnalysisPageViewModel(CoreDispatcher dispatcher, int channelNo)
        {
            _channelIndex = channelNo;
            _chartIndex = -1;
            _dispatcher = dispatcher;
            _chartName = $"Channel {channelNo + 1} data";

            BciData[] sampleData = DataManager.Current.Sample.ToArray();

            _channelData = new double[sampleData.Length];
            for (int i = 0; i < sampleData.Length; ++i) {
                _channelData[i] = sampleData[i].ChannelData[_channelIndex] * DataManager.ScaleFactor;
            }
        }

        public IList<KeyValuePair<int, double>> DataSeries
        { get => _dataSeries; }
        public IList<KeyValuePair<int, double>> Amplitudes
        { get => _amplitudes; }
        public IList<KeyValuePair<int, double>> Phases
        { get => _phases; }
        public IList<KeyValuePair<int, double>> Frequencies
        { get => _frequencies; }

        private string _chartName;
        public string ChartName
        {
            get => _chartName;
            set {
                _chartName = value;
                OnPropertyChanged(nameof(ChartName));
            }
        }

        private string _status;
        public string Status
        {
            get => _status;
            set {
                _status = value;
                OnPropertyChanged(nameof(Status));
            }
        }

        public async void Initialize() // to be called in OnNavigatedTo
        {
            await UpdateDataSeries(_channelData);
        }
        public async Task OnNextChartPressed(Action<bool> setButtonsEnabled)
        {
            if (_channelData.Length == 0) {
                return;
            }
            IImfDecompositionDouble decomp = DataManager.Current.Emds[_channelIndex];
            if (decomp == null) {
                Status = "Decomposing ...";
                setButtonsEnabled?.Invoke(false);

                double[] xValues = new double[_channelData.Length];
                for (int i = 0; i < _channelData.Length; ++i)
                    xValues[i] = i;

                decomp = await Emd.EnsembleDecomposeAsync(xValues, _channelData, 1.0, 50);
                DataManager.Current.Emds[_channelIndex] = decomp;

                Status = null;
                setButtonsEnabled?.Invoke(true);
            }

            if (_chartIndex == decomp.ImfFunctions.Count - 1 && decomp.ResidueFunction == null) // currently last imf and no residue
                return;
            if (_chartIndex == decomp.ImfFunctions.Count) // currently residue
                return;

            _chartIndex++;
            double[] dataToShow;
            string chartName = null;

            if (_chartIndex < decomp.ImfFunctions.Count) {
                dataToShow = decomp.ImfFunctions[_chartIndex].ToArray();
                chartName = $"IMF {_chartIndex}";
            }
            else {
                dataToShow = decomp.ResidueFunction;
                chartName = "Residue";
            }
            await UpdateDataSeries(dataToShow);
            ChartName = chartName;
        }
        public async Task OnPrevChartPressed()
        {
            if (_chartIndex == -1)
                return;

            IImfDecompositionDouble decomp = DataManager.Current.Emds[_channelIndex];
            _chartIndex--;
            double[] dataToShow;
            string chartName = null;

            if (_chartIndex > -1) {
                dataToShow = decomp.ImfFunctions[_chartIndex].ToArray();
                chartName = $"IMF {_chartIndex}";
            }
            else {
                dataToShow = _channelData;
                chartName = $"Channel {_channelIndex + 1} data";
            }
            await UpdateDataSeries(dataToShow);
            ChartName = chartName;
        }
        
        private async Task UpdateDataSeries(double[] data)
        {
            if (data.Length == 0) {
                Status = "No data to show";
                return;
            }

            Status = "Drawing the charts...";
            var analysis = Hsa.Analyse(data, 1.0);

            await _dispatcher.RunAsync(CoreDispatcherPriority.High, () => {

                _dataSeries = new List<KeyValuePair<int, double>>();
                for (int i = 0; i < data.Length; ++i) {
                    _dataSeries.Add(new KeyValuePair<int, double>(i, data[i]));
                }
                OnPropertyChanged(nameof(DataSeries));

                _amplitudes = new List<KeyValuePair<int, double>>();
                for (int i = 0; i < analysis.InstAmplitudes.Length; ++i) {
                    _amplitudes.Add(new KeyValuePair<int, double>(i, analysis.InstAmplitudes[i]));
                }
                OnPropertyChanged(nameof(Amplitudes));

                _phases = new List<KeyValuePair<int, double>>();
                for (int i = 0; i < analysis.InstPhases.Length; ++i) {
                    _phases.Add(new KeyValuePair<int, double>(i, analysis.InstPhases[i]));
                }
                OnPropertyChanged(nameof(Phases));

                _frequencies = new List<KeyValuePair<int, double>>();
                for (int i = 0; i < analysis.InstFrequencies.Length; ++i) {
                    _frequencies.Add(new KeyValuePair<int, double>(i, analysis.InstFrequencies[i]));
                }
                OnPropertyChanged(nameof(Frequencies));
            });
            Status = null;
        }
        private void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    /// <summary>
    /// Shows sampled data from all the 8 channels
    /// </summary>
    public class DataPageViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public DataPageViewModel()
        {
            LabelFormatter = (value) => value.ToString();
            ChannelData = new SeriesCollection[8];
        }
        public void Initialize()
        {
            BciData[] sampleCopy = DataManager.Current.Sample.ToArray();

            for (int channel = 0; channel < 8; ++channel) {
                ChartValues<double> ydata = new ChartValues<double>();

                for (int sample = 0; sample < sampleCopy.Length; ++sample) {
                    double value = sampleCopy[sample].ChannelData[channel] * DataManager.ScaleFactor;
                    ydata.Add(value);
                }
                ChannelData[channel] = new SeriesCollection
                {
                    new LineSeries
                    {
                        Title = null,
                        Values = ydata,
                        PointGeometry = DefaultGeometries.None,
                        StrokeThickness = 1.0
                    }
                };
            }
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ChannelData)));

        }

        public SeriesCollection[] ChannelData { get; }

        public Func<double, string> LabelFormatter { get; }
    }

}
