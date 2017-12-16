using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using LiveCharts;
using LiveCharts.Configurations;
using LiveCharts.Uwp;
using LiveCharts.Defaults;
using System.Threading.Tasks;
using Windows.UI.Core;
using Processing.Double;
using System.Diagnostics;
using Telerik.UI.Xaml.Controls.Chart;
using Windows.UI.Composition;
using System.Collections.ObjectModel;
using Telerik.Charting;
using System.Numerics;
using System.Collections.Specialized;

namespace RTGui
{
    /// <summary>
    /// Chart page showing variations of Marginal Hilbert Spectrum for one channel in real time 
    /// </summary>
    public sealed partial class RTAnalysisPage : Page
    {
        private RTAnalysisViewModel _vm;

        public RTAnalysisPage()
        {
            this.InitializeComponent();
            _vm = null;
        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            if (_vm != null) {
                DataManager.Current.SampleAnalysed -= OnSampleAnalysed;
                DataManager.ChartPagesStates.AddOrUpdate(_vm.Channel, _vm, (channel, prevVm) => _vm);
                _vm = null;
            }
            base.OnNavigatedFrom(e);
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            int? channel = e.Parameter as int?;
            if (channel != null) {
                DataManager.ChartPagesStates.TryRemove((int)channel, out _vm);
                if (_vm == null)
                    _vm = new RTAnalysisViewModel((int)channel);

                DataContext = _vm;
                DataManager.Current.SampleAnalysed += OnSampleAnalysed;
            }
        }
        private void OnSampleAnalysed(IHilbertSpectrum spectrum, int channel)
        {
            if (_vm != null && channel == _vm.Channel) {
                double maxFreq = spectrum.MaxFrequency;
                double minFreq = spectrum.MinFrequency;
                double step = (maxFreq - minFreq) / 1000;

                double avgSpectrum = 0.0;
                double maxSpectrum = spectrum.ComputeMarginalAt(minFreq);
                int i = 0;
                for (double w = minFreq; w <= maxFreq; w += step) {
                    double value = spectrum.ComputeMarginalAt(w);
                    avgSpectrum += value;
                    if (value > maxSpectrum)
                        maxSpectrum = value;
                    ++i;
                }
                avgSpectrum /= i;

                _vm.UpdateContent(maxFreq, minFreq, maxSpectrum, avgSpectrum, Dispatcher);
            }
        }
        private void NextChannel_OnClick(object sender, RoutedEventArgs e)
        {
            if (_vm != null) {
                Frame.Navigate(typeof(RTAnalysisPage), (_vm.Channel + 1) % 8);
            }
        }
        private void PrevChannel_OnClick(object sender, RoutedEventArgs e)
        {
            if (_vm != null) {
                Frame.Navigate(typeof(RTAnalysisPage), (_vm.Channel + 7) % 8);
            }
        }
    }

    public class ObservableList<T> : List<T>, INotifyCollectionChanged
    {
        public event NotifyCollectionChangedEventHandler CollectionChanged;
        public void NotifyCollectionReset()
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }
    }
    public class Point
    {
        public Point(double x, double y) { XValue = x; YValue = y; }
        public double XValue { get; set; }
        public double YValue { get; set; }
    }

    public class RTAnalysisViewModel : ViewModelBase
    {
        public const int X_MAX_LENGTH = 10;
        
        private readonly int _channel;

        public RTAnalysisViewModel(int channel)
        {
            _channel = channel;
            MaxFreqValues = new ObservableList<Point>();
            MinFreqValues = new ObservableList<Point>();
            MaxSpectrumValues = new ObservableList<Point>();
            AvgSpectrumValues = new ObservableList<Point>();
            Title = $"Channel {channel + 1}";
        }
        public async void UpdateContent(double maxFreq, double minFreq, double maxSpectrum, double avgSpectrum, CoreDispatcher uiDisp)
        {
            await uiDisp.RunAsync(CoreDispatcherPriority.Normal, () => {
                MaxFreqText = $"Max inst frequency = {maxFreq}";
                MinFreqText = $"Min inst frequency = {minFreq}";
                MaxSpectrumText = $"Max Hilbert spectrum = {maxSpectrum}";
                AvgSpectrumText = $"Avg Hilbert spectrum = {avgSpectrum}";
                
                AppendDataSeries(MaxFreqValues, maxFreq);
                AppendDataSeries(MinFreqValues, minFreq);

                AppendDataSeries(AvgSpectrumValues, avgSpectrum);
                AppendDataSeries(MaxSpectrumValues, maxSpectrum);
            });
        }
        private static void AppendDataSeries(ObservableList<Point> series, double newValue)
        {
            if (series.Count > X_MAX_LENGTH) {
                series.RemoveAt(0);
                foreach (Point p in series)
                    p.XValue--;
            }
            series.Add(new Point(series.Count, newValue));
            series.NotifyCollectionReset();
        }

        public ObservableList<Point> MaxFreqValues
        { get; }
        public ObservableList<Point> MinFreqValues
        { get; }
        public ObservableList<Point> MaxSpectrumValues
        { get; }
        public ObservableList<Point> AvgSpectrumValues
        { get; }

        public int Channel
        { get => _channel; }
        public double MaxXValue
        {
            get => X_MAX_LENGTH;
        }

        private string _title;
        public string Title
        {
            get => _title;
            set {
                _title = value;
                OnPropertyChanged(nameof(Title));
            }
        }
        private string _maxFreqText;
        public string MaxFreqText
        {
            get => _maxFreqText;
            set {
                _maxFreqText = value;
                OnPropertyChanged(nameof(MaxFreqText));
            }
        }
        private string _minFreqText;
        public string MinFreqText
        {
            get => _minFreqText;
            set {
                _minFreqText = value;
                OnPropertyChanged(nameof(MinFreqText));
            }
        }
        private string _avgSpectrumText;
        public string AvgSpectrumText
        {
            get => _avgSpectrumText;
            set {
                _avgSpectrumText = value;
                OnPropertyChanged(nameof(AvgSpectrumText));
            }
        }
        private string _maxSpectrumText;
        public string MaxSpectrumText
        {
            get => _maxSpectrumText;
            set {
                _maxSpectrumText = value;
                OnPropertyChanged(nameof(MaxSpectrumText));
            }
        }
    }
}
