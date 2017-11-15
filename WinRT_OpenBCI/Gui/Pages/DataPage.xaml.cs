using System;
using System.Collections.Generic;
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

namespace Gui
{
    /// <summary>
    /// Page showing data from all the 8 channels
    /// </summary>
    public sealed partial class DataPage : Page
    {
        public DataPage()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            DataContext = new DataPageViewModel();
            base.OnNavigatedTo(e);
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
                    Frame.Navigate(typeof(AnalysisPage), channelIndex);
                }
            }
        }

        private void Back_OnClick(object sender, RoutedEventArgs e)
        {
            Frame.GoBack();
        }
    }
}
