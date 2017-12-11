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

        public MainPage()
        {
            this.InitializeComponent();
            txtInfo.Text = "Welcome to RT Classifier!\n";
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

                txtInfo.Text += "Serial port opened\n";
            });
        }
        private async void Reset_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.RESET));
                txtInfo.Text += "Device reset\n";
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
                txtInfo.Text += "Streaming started\n";
                DataManager.Current.Start();
            });
        }
        private async void Stop_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.STOP_STREAM));
                txtInfo.Text += "Streaming stopped\n";
                DataManager.Current.Stop();
            });
        }
        private void ShowCharts_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                frmContent.Navigate(typeof(RTAnalysisPage), 0);
            });
        }
        private void ShowSample_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (_serial == null || DataManager.Current.LastSample == null)
                    throw new InvalidOperationException("Operation unavailable");
                Stop_OnClick(null, null);
                frmContent.Navigate(typeof(ChannelDataPage));
            });
        }
        // --------------------------------------------------------------------------------------------------------------------
        private async Task PopupIfThrowsAsync(Func<Task> operation)
        {
            try {
                await operation();
            }
            catch (Exception e) {
                txtInfo.Text += $"Error: {e.Message}\n";
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
                txtInfo.Text += $"Error: {e.Message}\n";
                var dialog = new MessageDialog(e.Message);
                await dialog.ShowAsync();
            }
        }
    }

    public class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
