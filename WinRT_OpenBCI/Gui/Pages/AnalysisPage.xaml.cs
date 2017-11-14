using System;
using System.Collections.Generic;
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

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace Gui
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
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
                await vm.OnNextChartPressed();
            }
        }

        private void Back_OnClick(object sender, RoutedEventArgs e)
        {
            Frame.GoBack();
        }
    }
}
