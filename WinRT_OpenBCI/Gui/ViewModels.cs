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
        private const int DATA_LENGTH = 100;
        private IList<KeyValuePair<double, double>> _dataSeries;
        private String _chartName;

        private IList<double> _xData, _yData;    // data to show
        private IList<double> _yOrigData;        // original data range
        private IImfDecomposition _decomposition;
        private int _currentIndex;          // index of current decomposition on screen
        private bool _busy;


        public event PropertyChangedEventHandler PropertyChanged;

        public MainPageViewModel()
        {
            _dataSeries = null; // set in UpdateDataSeries()
            _chartName = "Original data";

            _busy = false;
            _currentIndex = 0;
            _decomposition = null;

            _xData = new double[DATA_LENGTH];
            _yOrigData = new double[DATA_LENGTH];
            
            Random r = new Random(4);            
            for (int i = 1; i < DATA_LENGTH; ++i) {
                _yOrigData[i] = _yOrigData[i-1] + 2 * (r.NextDouble() - 0.5);
                _xData[i] = i;
            }
            _yData = _yOrigData;
            UpdateDataSeries();

            OnNextChartPressed = new RelayCommand(async (o) => {
                try {
                    if (_decomposition == null) {
                        _currentIndex = 0;
                        ChartName = "Decomposing...";
                        _busy = true;
                        await Task.Run(() => {
                            _decomposition = Emd.DoubleDecompose(_xData.ToArray(), _yOrigData.ToArray());
                        });
                        _busy = false;
                        ChartName = "Imf 0";
                        _yData = _decomposition.ImfFunctions[_currentIndex];
                    }
                    else {
                        _currentIndex++;
                        if (_currentIndex < _decomposition.ImfFunctions.Count) {
                            // show next imf
                            _yData = _decomposition.ImfFunctions[_currentIndex];
                            ChartName = $"Imf {_currentIndex}";
                        }
                        else if (_currentIndex == _decomposition.ImfFunctions.Count) {
                            // show residue
                            _yData = _decomposition.ResidueFunction;
                            ChartName = "Residue";
                        }
                        else {
                            // reset
                            _currentIndex = 0;
                            _decomposition = null;
                            _yData = _yOrigData;
                            ChartName = "Original data";
                        }
                    }
                    UpdateDataSeries();
                }
                catch (Exception e) {
                    Debug.WriteLine(e.Message);
                    Debug.WriteLine(e.Source);
                    Debug.WriteLine(e.StackTrace);
                }
            }, (o) => !_busy);
        }
        public IList<KeyValuePair<double, double>> DataSeries
        { get => _dataSeries; }
        /// <summary>
        /// Button handler
        /// </summary>
        public ICommand OnNextChartPressed
        { get; private set; }
        /// <summary>
        /// Button name
        /// </summary>
        public string ChartName
        {
            get => _chartName;
            set {
                _chartName = value;
                OnPropertyChanged(nameof(ChartName));
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
                Debug.WriteLine($"--- x = {_xData[i]}, y = {_yData[i]} ---");
            }
            _dataSeries = newSeries;
            OnPropertyChanged(nameof(DataSeries));
        }
    }

}
