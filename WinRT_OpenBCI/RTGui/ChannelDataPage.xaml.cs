using Communication;
using LiveCharts;
using LiveCharts.Uwp;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
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
    /// Page showing the last sample data from all the 8 channels
    /// </summary>
    public sealed partial class ChannelDataPage : Page
    {
        public ChannelDataPage()
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
            if (DataContext == null)
                DataContext = new ChannelDataViewModel();
        }
        private void Analyse_OnClick(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;
            if (btn != null) {
                int? channelIndex = null;

                if (btn == btnAnalyse0) {
                    channelIndex = 0;
                }
                else if (btn == btnAnalyse1) {
                    channelIndex = 1;
                }
                else if (btn == btnAnalyse2) {
                    channelIndex = 2;
                }
                else if (btn == btnAnalyse3) {
                    channelIndex = 3;
                }
                else if (btn == btnAnalyse4) {
                    channelIndex = 4;
                }
                else if (btn == btnAnalyse5) {
                    channelIndex = 5;
                }
                else if (btn == btnAnalyse6) {
                    channelIndex = 6;
                }
                else if (btn == btnAnalyse7) {
                    channelIndex = 7;
                }

                if (channelIndex != null) {
                    Frame.Navigate(typeof(EmdPage), channelIndex);
                }
            }
        }
        private void Spectrum_OnClick(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;
            if (btn != null) {
                int? channelIndex = null;

                if (btn == btnSpectrum0) {
                    channelIndex = 0;
                }
                else if (btn == btnSpectrum1) {
                    channelIndex = 1;
                }
                else if (btn == btnSpectrum2) {
                    channelIndex = 2;
                }
                else if (btn == btnSpectrum3) {
                    channelIndex = 3;
                }
                else if (btn == btnSpectrum4) {
                    channelIndex = 4;
                }
                else if (btn == btnSpectrum5) {
                    channelIndex = 5;
                }
                else if (btn == btnSpectrum6) {
                    channelIndex = 6;
                }
                else if (btn == btnSpectrum7) {
                    channelIndex = 7;
                }

                if (channelIndex != null) {
                    Frame.Navigate(typeof(SpectrumPage), channelIndex);
                }
            }
        }
    }

    /// <summary>
    /// Shows sampled data from all the 8 channels
    /// </summary>
    public class ChannelDataViewModel : ViewModelBase
    {
        public ChannelDataViewModel()
        {
            ChannelData = new SeriesCollection[8];
            ChartTitles = new string[8];

            BciData[] sampleCopy = DataManager.Current.LastSample;

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
                        Stroke = new SolidColorBrush(Colors.Red),
                        Fill = new SolidColorBrush(Colors.Transparent)
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
                return (value) => value.ToString("#.000");
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
