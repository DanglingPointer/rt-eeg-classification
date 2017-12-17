using System;
using System.Collections.Generic;
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

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace RTGui
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class ClassifierPage : Page
    {
        private string[] _actionNames;
        private int _currentAction;
        private bool _classifying, _collecting;

        public ClassifierPage()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

            DataManager.Current.Classifier.Stop();
            DataManager.Current.Classifier.SampleClassified -= OnSampleClassified;
            DataManager.ActionNames = _actionNames;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            _currentAction = 0;
            _classifying = _collecting = false;
            int modeCount = (int)e.Parameter;
            NeuralNetworkType nntype;
            if (modeCount > 100) {
                nntype = NeuralNetworkType.CascadeCorrelation;
                modeCount -= 100;
            }
            else {
                nntype = NeuralNetworkType.BackPropagating;
            }
            if (DataManager.Current.Classifier == null 
                || DataManager.Current.Classifier.NetworkType != nntype
                || DataManager.Current.Classifier.ModeCount != modeCount) {

                DataManager.Current.Classifier?.Unsubscrube();
                DataManager.Current.Classifier = new ClassifierAdapter(modeCount);
                DataManager.Current.Classifier.SetupNetwork(nntype);

                txtNetworkType.Text = nntype.ToString();
                _actionNames = new string[modeCount];
                for (int i = 0; i < modeCount; ++i)
                    _actionNames[i] = $"Action {i + 1}";
            }
            else {
                _actionNames = DataManager.ActionNames;
            }
            DataManager.Current.Classifier.SampleClassified += OnSampleClassified;

            txtActionNo.Text = $"{_currentAction + 1}";
            txtActionName.Text = _actionNames[_currentAction];

            btnCollect.Content = "Start collecting";
            btnCollect.IsEnabled = true;
            btnClassify.Content = "Start classifying";
            btnClassify.IsEnabled = true;
        }

        private async void CollectButton_OnClick(object sender, RoutedEventArgs e)
        {
            await PopupIfThrowsAsync(async () => {
                if (_collecting) {
                    // stop collecting
                    if (DataManager.Current.Classifier.ExamplesCollected < 1)
                        throw new Exception("Not enough examples collected");

                    DataManager.Current.Classifier.Stop();
                    btnClassify.IsEnabled = true;
                    _collecting = false;

                    _currentAction++;
                    if (_currentAction == DataManager.Current.Classifier.ModeCount) {
                        txtActionName.IsReadOnly = true;
                        btnCollect.Content = "Train";

                        txtActionName.Text = "N/A";
                        txtActionNo.Text = "N/A";
                    }
                    else {
                        txtActionName.IsReadOnly = false;
                        btnCollect.Content = "Start collecting";

                        txtActionName.Text = _actionNames[_currentAction];
                        txtActionNo.Text = $"{_currentAction + 1}";
                    }
                }
                else {
                    if (_currentAction == DataManager.Current.Classifier.ModeCount) {
                        // start training
                        btnCollect.IsEnabled = false;
                        btnClassify.IsEnabled = false;
                        await DataManager.Current.Classifier.TrainAsync();
                        btnCollect.IsEnabled = true;
                        btnClassify.IsEnabled = true;

                        _currentAction = 0;
                        txtActionName.Text = _actionNames[_currentAction];
                        txtActionName.IsReadOnly = false;
                        txtActionNo.Text = $"{_currentAction + 1}";
                        btnCollect.Content = "Start collecting";
                    }
                    else {
                        // start collecting
                        DataManager.Current.Classifier.StartCollecting(_currentAction);
                        btnClassify.IsEnabled = false;
                        _collecting = true;
                        txtActionName.IsReadOnly = true;
                        _actionNames[_currentAction] = txtActionName.Text;

                        btnCollect.Content = "Stop collecting";
                    }
                }
            });
        }
        private void ClassifyButton_OnClick(object sender, RoutedEventArgs e)
        {
            PopupIfThrows(() => {
                if (_classifying) {
                    // stop classifying
                    btnCollect.IsEnabled = true;
                    txtActionName.IsReadOnly = false;
                    btnClassify.Content = "Start classifying";
                    _classifying = false;

                    DataManager.Current.Classifier.Stop();
                }
                else {
                    // start classifying
                    btnCollect.IsEnabled = false;
                    txtActionName.IsReadOnly = true;
                    btnClassify.Content = "Stop classifying";
                    _classifying = true;

                    DataManager.Current.Classifier.StartClassifying();
                }
            });
        }
        private async void OnSampleClassified(int action)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                txtActionNo.Text = $"{action + 1}";
                txtActionName.Text = _actionNames[action];
            });
        }
        
        // --------------------------------------------------------------------------------------------------------------------
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
