using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.ViewManagement;
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
    /// Shows channel data and its IMFs, and corresponding Hilbert spectral analysis
    /// </summary>
    public sealed partial class AnalysisPage : Page
    {
        public AnalysisPage()
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
            
            int? channelNo = e.Parameter as int?;
            if (channelNo != null) {
                var viewModel = new AnalysisPageViewModel(Dispatcher, (int)channelNo);
                DataContext = viewModel;
                viewModel.Initialize();
            }
        }

        private async void PrevChart_OnClick(object sender, RoutedEventArgs e)
        {
            AnalysisPageViewModel vm = DataContext as AnalysisPageViewModel;
            if (vm != null) {
                await vm.OnPrevChartPressed();
            }
        }
        private async void NextChart_OnClick(object sender, RoutedEventArgs e)
        {
            AnalysisPageViewModel vm = DataContext as AnalysisPageViewModel;
            if (vm != null) {
                await vm.OnNextChartPressed((enabled) => {
                    btnNextChart.IsEnabled = enabled;
                    btnPrevChart.IsEnabled = enabled;
                    btnBack.IsEnabled = enabled;
                });
            }
        }
        private void Back_OnClick(object sender, RoutedEventArgs e)
        {
            Frame.GoBack();
        }
    }
}
