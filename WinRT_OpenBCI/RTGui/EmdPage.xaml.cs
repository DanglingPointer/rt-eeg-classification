using Communication;
using LiveCharts;
using LiveCharts.Uwp;
using Processing;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace RTGui
{
    /// <summary>
    /// Page showing EMD decomposition of the last sample
    /// </summary>
    public sealed partial class EmdPage : Page
    {
        public EmdPage()
        {
            this.InitializeComponent();
        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            int? channelNo = e.Parameter as int?;
            if (channelNo != null) {
                var viewModel = new EmdViewModel((int)channelNo);
                DataContext = viewModel;
                viewModel.Initialize();
            }
        }

        private void PrevChart_OnClick(object sender, RoutedEventArgs e)
        {
            EmdViewModel vm = DataContext as EmdViewModel;
            if (vm != null) {
                vm.OnPrevChartPressed();
            }
        }
        private async void NextChart_OnClick(object sender, RoutedEventArgs e)
        {
            EmdViewModel vm = DataContext as EmdViewModel;
            if (vm != null) {
                await vm.OnNextChartPressed((enabled) => {
                    btnNextChart.IsEnabled = enabled;
                    btnPrevChart.IsEnabled = enabled;
                });
            }
        }
    }

    public class EmdViewModel : ViewModelBase
    {
        private SeriesCollection _dataSeries;
        private SeriesCollection _amplitudes;
        private SeriesCollection _phases;
        private SeriesCollection _frequencies;

        private readonly int _channelIndex;
        private int _chartIndex;
        private string _chartName;
        private double[] _channelData;
        private IImfDecompositionDouble _decomp;

        public EmdViewModel(int channelNo)
        {
            _channelIndex = channelNo;
            _chartIndex = -1;
            _chartName = $"Channel {channelNo + 1} data";

            BciData[] sampleData = DataManager.Current.LastSample;

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
                return (value) => value.ToString("#.000");
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
            if (_decomp == null) {
                Status = "Decomposing ...";
                setButtonsEnabled?.Invoke(false);

                double[] xValues = new double[_channelData.Length];
                for (int i = 0; i < _channelData.Length; ++i)
                    xValues[i] = i;

                _decomp = await Emd.EnsembleDecomposeAsync(xValues, _channelData, 0.05, 100);

                Status = null;
                setButtonsEnabled?.Invoke(true);
            }

            if (_chartIndex == _decomp.ImfFunctions.Count - 1 && _decomp.ResidueFunction == null) // currently last imf and no residue
                return;
            if (_chartIndex == _decomp.ImfFunctions.Count) // currently residue
                return;

            _chartIndex++;
            double[] dataToShow;

            if (_chartIndex < _decomp.ImfFunctions.Count) {
                dataToShow = _decomp.ImfFunctions[_chartIndex].ToArray();
                _chartName = $"IMF {_chartIndex}";
            }
            else {
                dataToShow = _decomp.ResidueFunction;
                _chartName = "Residue";
            }
            UpdateDataSeries(dataToShow);
        }
        public void OnPrevChartPressed()
        {
            if (_chartIndex == -1)
                return;
            
            _chartIndex--;
            double[] dataToShow;

            if (_chartIndex > -1) {
                dataToShow = _decomp.ImfFunctions[_chartIndex].ToArray();
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
    }
}
