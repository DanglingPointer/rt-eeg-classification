using Communication;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
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

namespace Gui
{
    /// <summary>
    /// Start page
    /// </summary>
    public sealed partial class StartPage : Page
    {
        private BciSerialAdapter _serial;
        private volatile int _sampleCounter; // -1 indicates not currently sampling

        public StartPage()
        {
            this.InitializeComponent();
            _sampleCounter = -1;
            _serial = null;
        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            _serial?.ClosePort();
            _serial = null;
            base.OnNavigatedFrom(e);
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            int sampleLength = DataManager.Current.Sample.Count;
            txtSampleCount.Text = (sampleLength != 0) ? sampleLength.ToString() : "";
        }

        private async void StartStream_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.START_STREAM));
            });
        }
        private async void StopStream_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.STOP_STREAM));
            });
        }
        private async void Reset_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                await _serial.SendCommandAsync(BciCommand.Simple(BciCommand.General.RESET));
            });
        } 

        private async void Connect_OnClick(object sender, RoutedEventArgs ea)
        {
            await PopupIfThrowsAsync(async () => {
                if (_serial != null) {
                    _serial.ClosePort();
                }
                _serial = await BciSerialAdapter.CreateAny();

                _serial.BciDataReceived += (data) => {
                    if (_sampleCounter != -1) {
                        DataManager.Current.Sample.Enqueue(data);
                        _sampleCounter++;
                        txtSampleCount.Text = _sampleCounter.ToString();
                    }
                };
                _serial.BciInfoReceived += (info) => {
                    txtInfo.Text += $"{info}\n";
                };

                _serial.OpenPort();
            });
        }
        private void StartSampling_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                DataManager.Current.Reset();
                _sampleCounter = 0;
            });
        }
        private void StopSampling_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (_serial == null)
                    throw new InvalidOperationException("Not connected");
                _sampleCounter = -1;
            });
        }
        private void ViewData_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (DataManager.Current.Sample.Count == 0)
                    throw new InvalidOperationException("No data sampled");
                Frame.Navigate(typeof(DataPage));
            });
        }

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
}
