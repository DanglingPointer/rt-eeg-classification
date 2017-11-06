using Communication;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Shapes;
using System.Windows.Threading;

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
        private const int MAX_DATA_LENGTH = 500;
        private const double SCALE_FACTOR = 4.5d / 24 / 8388607.0d;

        private long _currentX;
        private ConcurrentQueue<BciData> _dataQueue;
        
        private readonly DispatcherTimer _timer;
        private readonly SerialManager _serial;


        public event PropertyChangedEventHandler PropertyChanged;

        public MainViewModel()
        {
            _dataQueue = new ConcurrentQueue<BciData>();

            DataSeries = new ObservableQueue<KeyValuePair<long, double>>(MAX_DATA_LENGTH);
            _currentX = 0;

            _timer = new DispatcherTimer(DispatcherPriority.DataBind) {
                Interval = new TimeSpan(0, 0, 0, 0, 30)
            };
            _timer.Tick += (sender, e) => {
                if (_dataQueue.Count > MAX_DATA_LENGTH / 50) {
                    while (_dataQueue.Count > 0) {
                        BciData data;
                        _dataQueue.TryDequeue(out data);
                        DataSeries.Enqueue(new KeyValuePair<long, double>(_currentX++, data.ChannelData[4] * SCALE_FACTOR));
                    }
                    while (DataSeries.Count > MAX_DATA_LENGTH) {
                        DataSeries.Dequeue();
                    }
                    DataSeries.NotifyCollectionChanged();
                }            
            };

            try {
                _serial = SerialManager.CreateAny();
                _serial.BciDataReceived += (data) => {                
                    _dataQueue.Enqueue(data);
                };
                _serial.BciInfoReceived += (info) => {
                    InfoText += (info + '\n');
                    OnPropertyChanged(nameof(InfoText));
                };
                _serial.OpenPort();
            }
            catch (Exception e) {
                MessageBox.Show(e.ToString());
            }
            
            OnStartButtonPressed = new RelayCommand((o) => {
                _timer.Start();
                _serial?.SendCommand(BciCommand.Simple(BciCommand.General.START_STREAM));
            });
            OnStopButtonPressed = new RelayCommand(async (o) => {
                _serial?.SendCommand(BciCommand.Simple(BciCommand.General.STOP_STREAM));
                _timer.Stop();
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
