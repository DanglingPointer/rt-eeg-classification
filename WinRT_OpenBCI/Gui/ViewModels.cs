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
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Popups;
using Windows.UI.Xaml.Media;

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
        private SeriesCollection _dataSeries;
        private SeriesCollection _amplitudes;
        private SeriesCollection _phases;
        private SeriesCollection _frequencies;

        private readonly int _channelIndex;
        private int _chartIndex;
        private string _chartName;
        private double[] _channelData;


        public event PropertyChangedEventHandler PropertyChanged;
        
        public AnalysisPageViewModel(int channelNo)
        {
            _channelIndex = channelNo;
            _chartIndex = -1;
            _chartName = $"Channel {channelNo + 1} data";

            BciData[] sampleData = DataManager.Current.Sample.ToArray();

            _channelData = new double[sampleData.Length];
            for (int i = 0; i < sampleData.Length; ++i) {
                _channelData[i] = sampleData[i].ChannelData[_channelIndex] * DataManager.ScaleFactor;
            }
        }

        public SeriesCollection DataSeries
        { get => _dataSeries; }
        public SeriesCollection Amplitudes
        { get => _amplitudes; }
        public SeriesCollection Phases
        { get => _phases; }
        public SeriesCollection Frequencies
        { get => _frequencies; }


        private string _status;
        public string Status
        {
            get => _status;
            set {
                _status = value;
                OnPropertyChanged(nameof(Status));
            }
        }

        public Func<double, string> YLabelFormatter
        {
            get {
                return (value) => value.ToString("F");
            }
        }
        public Func<double, string> XLabelFormatter
        {
            get {
                return (value) => ((int)(value * 2)).ToString();
            }
        }
        public Func<double, string> DataXLabelFormatter
        {
            get {
                return (value) => value.ToString();
            }
        }

        public void Initialize() // to be called in OnNavigatedTo
        {
            UpdateDataSeries(_channelData);
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

            if (_chartIndex < decomp.ImfFunctions.Count) {
                dataToShow = decomp.ImfFunctions[_chartIndex].ToArray();
                _chartName = $"IMF {_chartIndex}";
            }
            else {
                dataToShow = decomp.ResidueFunction;
                _chartName = "Residue";
            }
            UpdateDataSeries(dataToShow);
        }
        public void OnPrevChartPressed()
        {
            if (_chartIndex == -1)
                return;

            IImfDecompositionDouble decomp = DataManager.Current.Emds[_channelIndex];
            _chartIndex--;
            double[] dataToShow;

            if (_chartIndex > -1) {
                dataToShow = decomp.ImfFunctions[_chartIndex].ToArray();
                _chartName = $"IMF {_chartIndex}";
            }
            else {
                dataToShow = _channelData;
                _chartName = $"Channel {_channelIndex + 1} data";
            }
            UpdateDataSeries(dataToShow);
        }
        
        private void UpdateDataSeries(double[] data)
        {
            if (data.Length == 0) {
                Status = "No data to show";
                return;
            }
            var analysis = Hsa.Analyse(data, 1.0);

            var dataValues = new ChartValues<double>();
            for (int i = 0; i < data.Length; ++i) {
                dataValues.Add(data[i]);
            }
            _dataSeries = new SeriesCollection {
                new LineSeries {
                    Title = _chartName,
                    Values = dataValues,
                    PointGeometry = DefaultGeometries.None,
                    StrokeThickness = 0.5,
                    Stroke = new SolidColorBrush(Colors.Red)
                }
            };
            OnPropertyChanged(nameof(DataSeries));

            var ampValues = new ChartValues<double>();
            double[] amplitudes = analysis.InstAmplitudes;
            for (int i = 0; i < amplitudes.Length; i += 2) {
                ampValues.Add(amplitudes[i]);
            }
            _amplitudes = new SeriesCollection {
                new LineSeries {
                    Title = "Instantaneous amplitudes",
                    Values = ampValues,
                    PointGeometry = DefaultGeometries.None,
                    StrokeThickness = 0.5,
                    Stroke = new SolidColorBrush(Colors.Red)
                }
            };
            OnPropertyChanged(nameof(Amplitudes));

            var phasValues = new ChartValues<double>();
            double[] phases = analysis.InstPhases;
            for (int i = 0; i < phases.Length; i += 2) {
                phasValues.Add(phases[i]);
            }
            _phases = new SeriesCollection {
                new LineSeries {
                    Title = "Instantaneous phases",
                    Values = phasValues,
                    PointGeometry = DefaultGeometries.None,
                    StrokeThickness = 0.5,
                    Stroke = new SolidColorBrush(Colors.Red)
                }
            };
            OnPropertyChanged(nameof(Phases));

            var freqValues = new ChartValues<double>();
            double[] frequencies = analysis.InstFrequencies;
            for (int i = 0; i < frequencies.Length; i += 2) {
                freqValues.Add(frequencies[i]);
            }
            _frequencies = new SeriesCollection {
                new LineSeries {
                    Title = "Instantaneous frequencies",
                    Values = freqValues,
                    PointGeometry = DefaultGeometries.None,
                    StrokeThickness = 0.5,
                    Stroke = new SolidColorBrush(Colors.Red)
                }
            };
            OnPropertyChanged(nameof(Frequencies));
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
            ChannelData = new SeriesCollection[8];
            ChartTitles = new string[8];

            BciData[] sampleCopy = DataManager.Current.Sample.ToArray();

            for (int channel = 0; channel < 8; ++channel) {
                ChartValues<double> ydata = new ChartValues<double>();

                for (int sample = 0; sample < sampleCopy.Length; sample += 2) {
                    double value = sampleCopy[sample].ChannelData[channel] * DataManager.ScaleFactor;
                    ydata.Add(value);
                }
                ChannelData[channel] = new SeriesCollection {
                    new LineSeries {
                        Title = null,
                        Values = ydata,
                        PointGeometry = DefaultGeometries.None,
                        StrokeThickness = 0.5,
                        Stroke = new SolidColorBrush(Colors.Red)
                    }
                };
                ChartTitles[channel] = $"Channel {channel + 1}";
            }
        }
        public string[] ChartTitles { get; }
        /// <summary>
        /// Data binded to the charts
        /// </summary>
        public SeriesCollection[] ChannelData { get; }
        /// <summary>
        /// Axes labels formatter binded to the charts
        /// </summary>
        public Func<double, string> YLabelFormatter
        {
            get {
                return (value) => value.ToString("F");
            }
        }
        public Func<double, string> XLabelFormatter
        {
            get {
                return (value) => ((int)(value * 2)).ToString();
            }
        }
    }

}
