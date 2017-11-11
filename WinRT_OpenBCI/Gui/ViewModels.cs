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
        private const int DATA_LENGTH = 1000;
        private IList<KeyValuePair<double, double>> _dataSeries;
        private String _chartName;

        RelayCommand _onNextChartPressed;
        RelayCommand _onPrevChartPressed;

        private IList<double> _xData, _yData;    // data to show
        private IList<double> _yOrigData;        // original data range
        private IImfDecompositionDouble _decomposition;
        private int _currentIndex;          // index of current decomposition on screen
        private volatile bool _busy;


        public event PropertyChangedEventHandler PropertyChanged;

        public MainPageViewModel(LinearAxis yAxis)
        {
            yAxis.Minimum = -10;
            yAxis.Maximum = 10;

            _dataSeries = null; // set in UpdateDataSeries()
            _chartName = "Original data";

            _busy = false;
            _currentIndex = 0;
            _decomposition = null;

            _xData = new double[DATA_LENGTH];
            _yOrigData = new double[DATA_LENGTH];

            GenerateOrigData();
            _yData = _yOrigData;
            UpdateDataSeries();

            _onPrevChartPressed = new RelayCommand((o) => {
                _currentIndex--;
                _onPrevChartPressed.ExecuteChanged(this, null);
                if (_currentIndex >= 0) {
                    // show previous imf
                    _yData = _decomposition.ImfFunctions[_currentIndex];
                    ChartName = $"Intrinsic mode function {_currentIndex}";
                }
                else { // _currentIndex < 0
                        // show original
                    _yData = _yOrigData;
                    ChartName = "Original data";
                }
                UpdateDataSeries();
            }, (o) => !_busy && _decomposition != null && _currentIndex > -1);

            _onNextChartPressed = new RelayCommand(async (o) => {
                if (_decomposition == null) {
                    _currentIndex = 0;
                    ChartName = "Decomposing...";
                    SetBusy(true);
                    _decomposition = await Emd.DecomposeAsync(_xData.ToArray(), _yOrigData.ToArray());
                    ChartName = "Intrinsic mode function 0";
                    _yData = _decomposition.ImfFunctions[_currentIndex];
                    SetBusy(false);
                }
                else {
                    _currentIndex++;
                    if (_currentIndex < _decomposition.ImfFunctions.Count) {
                        // show next imf
                        _yData = _decomposition.ImfFunctions[_currentIndex];
                        ChartName = $"Intrinsic mode function {_currentIndex}";
                    }
                    else if (_currentIndex == _decomposition.ImfFunctions.Count && _decomposition.ResidueFunction != null) {
                        // show residue
                        _yData = _decomposition.ResidueFunction;
                        ChartName = "Residue";
                    }
                    else {
                        // reset
                        _decomposition = null;
                        _currentIndex = 0;
                        GenerateOrigData();
                        _yData = _yOrigData;
                        ChartName = "Original data";
                    }
                    _onPrevChartPressed.ExecuteChanged(this, null);
                }
                UpdateDataSeries();
            }, (o) => !_busy);
        }
        public IList<KeyValuePair<double, double>> DataSeries
        { get => _dataSeries; }
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
            set {
                _chartName = value;
                OnPropertyChanged(nameof(ChartName));
            }
        }
        private void GenerateOrigData()
        {
            Random r = new Random();
            for (int i = 1; i < DATA_LENGTH; ++i) {
                _yOrigData[i] = _yOrigData[i - 1] + 1 * (r.NextDouble() - 0.5);
                _xData[i] = i;
            }
        }
        private void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
        /// <summary>
        /// Updates data to show with _yData and _xData
        /// </summary>
        private void UpdateDataSeries()
        {
            var newSeries = new List<KeyValuePair<double, double>>();
            for (int i = 0; i < DATA_LENGTH; ++i) {
                newSeries.Add(new KeyValuePair<double, double>(_xData[i], _yData[i]));
            }
            _dataSeries = newSeries;
            OnPropertyChanged(nameof(DataSeries));
        }
        private void SetBusy(bool busy)
        {
            _busy = busy;
            _onNextChartPressed.ExecuteChanged(this, null);
            _onPrevChartPressed.ExecuteChanged(this, null);
        }
    }

}
