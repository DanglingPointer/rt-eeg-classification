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
    /// A page showing Hilbert Marginal Spectrum
    /// </summary>
    public sealed partial class HilbertPage : Page
    {
        public HilbertPage()
        {
            this.InitializeComponent();
            DataContext = new HilbertPageViewModel();
        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            int? channel = e.Parameter as int?;
            var vm = DataContext as HilbertPageViewModel;
            if (vm != null && channel != null) {
                await vm.Initialize((int)channel);
            }
        }
        private void Back_OnClick(object sender, RoutedEventArgs e)
        {
            Frame.GoBack();
        }
    }
}
