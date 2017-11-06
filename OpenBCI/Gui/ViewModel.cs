using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls.DataVisualization;
using System.Windows.Controls.DataVisualization.Charting;
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

    /// <summary>
    /// DataContext for Main window
    /// </summary>
    public class MainViewModel : INotifyPropertyChanged
    {
        private const int DATA_SERIES_LENGTH = 100;
        private long _currentX;
        private readonly Random _rand;

        public event PropertyChangedEventHandler PropertyChanged;

        public MainViewModel()
        {
            _rand = new Random();
            _currentX = 0;
            DataSeries = new ObservableQueue<KeyValuePair<long, double>>(DATA_SERIES_LENGTH);

            for (int i = 0; i < DATA_SERIES_LENGTH; ++i) {
                DataSeries.Enqueue(new KeyValuePair<long, double>(_currentX++, _rand.NextDouble()));
            }
            OnStartButtonPressed = new RelayCommand((o) => {
                DataSeries.Dequeue();
                DataSeries.Enqueue(new KeyValuePair<long, double>(_currentX++, _rand.NextDouble()));
                OnPropertyChanged(nameof(DataSeries)); // unnecessary
            });
            OnStopButtonPressed = new RelayCommand((o) => {
                InfoText += "yo";
                OnPropertyChanged(nameof(InfoText));
            });
        }
        public ICommand OnStartButtonPressed
        { get; }
        public ICommand OnStopButtonPressed
        { get; }
        public ObservableQueue<KeyValuePair<long, double>> DataSeries
        { get; }
        public string InfoText
        { get; private set; }
        private void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
