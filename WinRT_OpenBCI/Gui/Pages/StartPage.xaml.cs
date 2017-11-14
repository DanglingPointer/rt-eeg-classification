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

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace Gui
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class StartPage : Page
    {
        public StartPage()
        {
            this.InitializeComponent();
        }

        private void StartStream_OnClick(object sender, RoutedEventArgs e)
        {

        }

        private void Connect_OnClick(object sender, RoutedEventArgs e)
        {

        }

        private void StopStream_OnClick(object sender, RoutedEventArgs e)
        {

        }

        private void Reset_OnClick(object sender, RoutedEventArgs e)
        {

        }

        private void StartSampling_OnClick(object sender, RoutedEventArgs e)
        {

        }

        private void StopSampling_OnClick(object sender, RoutedEventArgs e)
        {

        }

        private void ViewData_OnClick(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(DataPage));
        }
    }
}
