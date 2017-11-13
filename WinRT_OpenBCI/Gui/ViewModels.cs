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
using WinRTXamlToolkit.Controls.DataVisualization.Charting;

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

    public class MainPageViewModel : INotifyPropertyChanged
    {
        private const int DATA_LENGTH = 500;
        private IList<KeyValuePair<float, float>> _dataSeries;
        private String _chartName;
        private string _status;

        private IList<KeyValuePair<float, float>> _amplitudes;
        private IList<KeyValuePair<float, float>> _phases;
        private IList<KeyValuePair<float, float>> _frequencies;

        RelayCommand _onNextChartPressed;
        RelayCommand _onPrevChartPressed;

        private IList<float> _xData, _yData;    // data to show
        private IList<float> _yOrigData;        // original data range
        private IImfDecompositionSingle _decomposition;
        private int _currentIndex;          // index of current decomposition on screen
        private volatile bool _busy;


        public event PropertyChangedEventHandler PropertyChanged;
        
        public MainPageViewModel()
        {
            _chartName = "Original data";

            _busy = false;
            _currentIndex = 0;
            _decomposition = null;

            _xData = new float[DATA_LENGTH];
            _yOrigData = new float[DATA_LENGTH];

            GenerateData();
            _yData = _yOrigData;
            
            UpdateDataSeries(null);

            _onNextChartPressed = new RelayCommand(async (o) => {
                if (_decomposition == null) {
                    _currentIndex = 0;

                    Status = "Decomposing...";
                    SetBusy(true);
                    _decomposition = await Emd.EnsembleDecomposeAsync(_xData.ToArray(), _yOrigData.ToArray(), 1.0f);
                    SetBusy(false);
                    Status = null;

                    _chartName = "Intrinsic mode function 0";
                    _yData = _decomposition.ImfFunctions[_currentIndex];                    
                }
                else {
                    _currentIndex++;
                    if (_currentIndex < _decomposition.ImfFunctions.Count) {
                        // show next imf
                        _yData = _decomposition.ImfFunctions[_currentIndex];
                        _chartName = $"Intrinsic mode function {_currentIndex}";
                    }
                    else if (_currentIndex == _decomposition.ImfFunctions.Count && _decomposition.ResidueFunction != null) {
                        // show residue
                        _yData = _decomposition.ResidueFunction;
                        _chartName = "Residue";
                    }
                    else {
                        // reset
                        _decomposition = null;
                        _currentIndex = 0;
                        GenerateData();
                        _yData = _yOrigData;
                        _chartName = "Original data";
                    }
                    _onPrevChartPressed.ExecuteChanged(this, null);
                }
                var analysis = await GetAnalysis();
                UpdateDataSeries(analysis);
                OnPropertyChanged(nameof(ChartName));
            }, (o) => !_busy);

            _onPrevChartPressed = new RelayCommand(async (o) => {
                _currentIndex--;
                _onPrevChartPressed.ExecuteChanged(this, null);
                if (_currentIndex >= 0) {
                    // show previous imf
                    _yData = _decomposition.ImfFunctions[_currentIndex];
                    _chartName = $"Intrinsic mode function {_currentIndex}";
                }
                else { // _currentIndex < 0
                       // show original
                    _yData = _yOrigData;
                    _chartName = "Original data";
                }
                var analysis = await GetAnalysis();
                UpdateDataSeries(analysis);
                OnPropertyChanged(nameof(ChartName));
            }, (o) => !_busy && _decomposition != null && _currentIndex > -1);
        }
        public IList<KeyValuePair<float, float>> DataSeries
        { get => _dataSeries; }
        public IList<KeyValuePair<float, float>> Amplitudes
        { get => _amplitudes; }
        public IList<KeyValuePair<float, float>> Phases
        { get => _phases; }
        public IList<KeyValuePair<float, float>> Frequencies
        { get => _frequencies; }
        /// <summary>
        /// Button handler
        /// </summary>
        public ICommand OnNextChartPressed
        { get => _onNextChartPressed; }
        /// <summary>
        /// Button handler
        /// </summary>
        public ICommand OnPrevChartPressed
        { get => _onPrevChartPressed; }
        /// <summary>
        /// Chart title
        /// </summary>
        public string ChartName
        {
            get => _chartName;
        }
        public string Status
        {
            get => _status;
            set {
                _status = value;
                OnPropertyChanged(nameof(Status));
            }
        }
        private void GenerateData()
        {
            Random r = new Random();
            for (int i = 1; i < DATA_LENGTH; ++i) {
                _yOrigData[i] = _yOrigData[i - 1] + 1 * (float)(r.NextDouble() - 0.5);
                _xData[i] = i;
            }
        }
        private async Task<ISpectralAnalysisSingle> GetAnalysis()
        {
            Status = "Analyzing...";
            SetBusy(true);
            var analysis = await Hsa.AnalyseAsync(_yData.ToArray(), 1.0f);
            SetBusy(false);
            Status = null;
            return analysis;
        }
        /// <summary>
        /// Updates data to show with _yData and _xData
        /// </summary>
        private void UpdateDataSeries(ISpectralAnalysisSingle analysis)
        {
            _dataSeries = new List<KeyValuePair<float, float>>();
            for (int i = 0; i < DATA_LENGTH; ++i) {
                _dataSeries.Add(new KeyValuePair<float, float>(_xData[i], _yData[i]));
            }
            OnPropertyChanged(nameof(DataSeries));

            if (analysis != null) {
                _amplitudes = new List<KeyValuePair<float, float>>();
                for (int i = 0; i < analysis.InstAmplitudes.Length; ++i) {
                    _amplitudes.Add(new KeyValuePair<float, float>(i, analysis.InstAmplitudes[i]));
                }
                OnPropertyChanged(nameof(Amplitudes));

                _phases = new List<KeyValuePair<float, float>>();
                for (int i = 0; i < analysis.InstPhases.Length; ++i) {
                    _phases.Add(new KeyValuePair<float, float>(i, analysis.InstPhases[i]));
                }
                OnPropertyChanged(nameof(Phases));

                _frequencies = new List<KeyValuePair<float, float>>();
                for (int i = 0; i < analysis.InstFrequencies.Length; ++i) {
                    _frequencies.Add(new KeyValuePair<float, float>(i, analysis.InstFrequencies[i]));
                }
                OnPropertyChanged(nameof(Frequencies));
            }
        }
        private void SetBusy(bool busy)
        {
            _busy = busy;
            _onNextChartPressed?.ExecuteChanged(this, null);
            _onPrevChartPressed?.ExecuteChanged(this, null);
        }
        private void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

}
