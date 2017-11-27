using Communication;
using LiveCharts;
using LiveCharts.Configurations;
using LiveCharts.Uwp;
using Processing;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
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
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class SpectrumPage : Page
    {
        public SpectrumPage()
        {
            this.InitializeComponent();
            DataContext = new SpectrumViewModel();
        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            int? channel = e.Parameter as int?;
            var vm = DataContext as SpectrumViewModel;
            if (vm != null && channel != null) {
                await vm.Initialize((int)channel);
            }
        }
    }

    public class SpectrumViewModel : ViewModelBase
    {
        private const int XLength = 500;

        private SeriesCollection _marginalSpectrum;
        private string _status;
        private string _info;

        private double _xStep;

        public async Task Initialize(int channelIndex)
        {
            BciData[] sampleData = DataManager.Current.LastSample;

            var channelData = new double[sampleData.Length];
            for (int i = 0; i < sampleData.Length; ++i) {
                channelData[i] = sampleData[i].ChannelData[channelIndex] * DataManager.ScaleFactor;
            }
            
            double[] xValues = new double[channelData.Length];
            for (int i = 0; i < channelData.Length; ++i)
                xValues[i] = i;

            Status = "Decomposing...";
            IImfDecompositionDouble decomp = await Emd.EnsembleDecomposeAsync(xValues, channelData, 0.05, 100);

            Status = "Analysing...";
            IHilbertSpectrumDouble hs = await Hsa.GetHilbertSpectrumAsync(decomp, 1.0);
            Status = null;

            _xStep = (hs.MaxFrequency - hs.MinFrequency) / XLength;

            var marginalData = new ChartValues<double>();
            for (double w = hs.MinFrequency; w <= hs.MaxFrequency; w += _xStep) {
                marginalData.Add(hs.ComputeMarginalAt(w));
            }

            double marginalMin = marginalData[0], marginalMax = marginalData[0], marginalMean = 0.0;
            foreach (double val in marginalData) {
                marginalMean += val;
                if (val < marginalMin)
                    marginalMin = val;
                else if (val > marginalMax)
                    marginalMax = val;
            }
            marginalMean /= marginalData.Count;

            Info = $"Channel = {channelIndex+1}\nMin value = {marginalMin}\nMax value = {marginalMax}\nAverage value = {marginalMean}";

            var mapper = new CartesianMapper<double>()
                .X((value, index) => hs.MinFrequency + index * _xStep)
                .Y((value, index) => value);
            _marginalSpectrum = new SeriesCollection(mapper) {
                new LineSeries {
                    Title = "Marginal Hilbert Spectrum",
                    Values = marginalData,
                    PointGeometry = DefaultGeometries.None,
                    StrokeThickness = 0.5,
                    Stroke = new SolidColorBrush(Colors.Red)
                }
            };
            OnPropertyChanged(nameof(MarginalSpectrum));
        }
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
        public SeriesCollection MarginalSpectrum
        { get => _marginalSpectrum; }
        public string Status
        {
            get => _status;
            set {
                _status = value;
                OnPropertyChanged(nameof(Status));
            }
        }
        public string Info
        {
            get => _info;
            set {
                _info = value;
                OnPropertyChanged(nameof(Info));
            }
        }
    }
}
