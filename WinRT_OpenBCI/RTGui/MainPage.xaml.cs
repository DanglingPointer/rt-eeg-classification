using Communication;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Popups;
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
    /// Main page controlling OpenBCI
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private BciSerialAdapter _serial;
        private int? _channel;

        public MainPage()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            _serial?.ClosePort();
            _serial = null;
            DataManager.Current.Stop();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
        }

        // --------------------------------------------------------------------------------------------------------------------
        private async void Connect_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial != null) {
                    _serial.ClosePort();
                }

                _serial = await BciSerialAdapter.CreateAny();
                _serial.BciDataReceived += (data) => {
                    DataManager.Current.EnqueueData(data);
                };
                _serial.BciInfoReceived += (info) => {
                    txtInfo.Text += $"{info}\n";
                };
                _serial.OpenPort();

                txtStatus.Text = "Serial port opened";
            });
        }
        private async void Reset_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.RESET));
                txtStatus.Text = "Device reset";
                txtInfo.Text = "";
                DataManager.Current.Stop();
            });
        }
        private async void Start_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.START_STREAM));
                txtStatus.Text = "Streaming started";
                DataManager.Current.Start();
            });
        }
        private async void Stop_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.STOP_STREAM));
                txtStatus.Text = "Streaming stopped";
                DataManager.Current.Stop();
            });
        }

        private void PrevChannel_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                if (_channel == null)
                    throw new InvalidOperationException("No way back, only forward");
                _channel = (_channel == 0) ? 7 : (_channel - 1);
                frmContent.Navigate(typeof(ChartPage), _channel);
            });
        }
        private void NextChannel_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                if (_channel == null) {
                    _channel = -1;
                }
                _channel++;
                _channel %= 8;
                frmContent.Navigate(typeof(ChartPage), _channel);
            });
        }
        // --------------------------------------------------------------------------------------------------------------------
        private async Task PopupIfThrowsAsync(Func<Task> operation)
        {
            try {
                await operation();
            }
            catch (Exception e) {
                var dialog = new MessageDialog(e.Message);
                await dialog.ShowAsync();
            }
        }
        private async void PopupIfThrows(Action operation)
        {
            try {
                operation();
            }
            catch (Exception e) {
                var dialog = new MessageDialog(e.Message);
                await dialog.ShowAsync();
            }
        }
    }

    public class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public Func<double, string> YLabelFormatter
        {
            get {
                return (value) => value.ToString("#.000");
            }
        }
        public Func<double, string> XLabelFormatter
        {
            get {
                return (value) => value.ToString();
            }
        }
        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
