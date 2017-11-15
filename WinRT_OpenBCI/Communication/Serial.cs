//
//   Copyright 2017 Mikhail Vasilyev
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;
using Windows.Storage.Streams;

namespace Communication
{
    /// <summary>
    /// Event-based wrapper for SerialDevice
    /// </summary>
    class SerialWrapper : IDisposable
    {
        private readonly SerialDevice _port;
        private CancellationTokenSource _closeTokenSrc;
        private readonly Object _mutex;

        public event Action<byte[]> DataReceived;

        public SerialWrapper(SerialDevice device)
        {
            _port = device;
            _port.BaudRate = 115200;

            _mutex = new object();
            _closeTokenSrc = null;
        }
        public async void StartReceiving()
        {
            lock (_mutex) {
                // start only if never been started or has been cancelled
                if (_closeTokenSrc != null && !_closeTokenSrc.IsCancellationRequested)
                    return;
                _closeTokenSrc = new CancellationTokenSource();
            }
            DataReader reader = null;
            try {
                reader = new DataReader(_port.InputStream) {
                    InputStreamOptions = InputStreamOptions.Partial
                };
                for (; ; ) {
                    Task<UInt32> loadAsyncTask;
                    lock (_mutex) {
                        _closeTokenSrc.Token.ThrowIfCancellationRequested();
                        loadAsyncTask = reader.LoadAsync(512).AsTask(_closeTokenSrc.Token);
                    }
                    UInt32 bytesRead = await loadAsyncTask;
                    if (bytesRead > 0) {
                        byte[] data = new byte[bytesRead];
                        reader.ReadBytes(data);
                        DataReceived?.Invoke(data);
                    }
                }
            }
            catch (OperationCanceledException) {
                // port stopped/closed
            }
            finally {
                reader?.DetachStream();
                reader?.Dispose();
            }
        }
        public void Stop()
        {
            lock (_mutex) {
                if (_closeTokenSrc != null && !_closeTokenSrc.IsCancellationRequested) {
                    _closeTokenSrc.Cancel();
                }
            }
        }
        public async Task<bool> Send(byte[] data)
        {
            DataWriter writer = null;
            try {
                writer = new DataWriter(_port.OutputStream);
                writer.WriteBytes(data);

                Task<UInt32> storeAsyncTask;
                lock (_mutex) {
                    if (_closeTokenSrc == null || _closeTokenSrc.IsCancellationRequested) {
                        throw new OperationCanceledException();
                    }
                    storeAsyncTask = writer.StoreAsync().AsTask(_closeTokenSrc.Token);
                }
                uint bytesWritten = await storeAsyncTask;
                return bytesWritten == data.Length;
            }
            catch (OperationCanceledException) {
                return false;
            }
            finally {
                writer?.DetachStream();
                writer?.Dispose();
            }
        }
        public void Dispose()
        {
            Stop();
            _closeTokenSrc.Dispose();
            _port.Dispose();
        }
    }

    public class BciSerialAdapter
    {
        private SerialWrapper _port;
        private MemoryStream _receiveBuffer;

        public event Action<BciData> BciDataReceived;
        public event Action<string> BciInfoReceived;

        public static async Task<BciSerialAdapter> CreateAny()
        {
            var devices = await DeviceInformation.FindAllAsync();
            if (devices.Count == 0) {
                throw new InvalidOperationException("No serial devices found");
            }
            var port = await SerialDevice.FromIdAsync(devices[0].Id);
            return new BciSerialAdapter(port);
        }

        public BciSerialAdapter(SerialDevice device)
        {
            _port = new SerialWrapper(device);
            _receiveBuffer = new MemoryStream();

            _port.DataReceived += OnDataReceived;
        }
        public void OpenPort()
        {
            if (_port == null)
                throw new InvalidOperationException("Can't reopen a disposed serial port");
            _port.StartReceiving();
        }
        public void ClosePort()
        {
            if (_port == null)
                return;
            _port.DataReceived -= OnDataReceived;
            _port.Dispose();
            _port = null;
        }
        public async Task SendCommandAsync(BciCommand cmd)
        {
            if (_port == null)
                throw new InvalidOperationException("Serial port disposed");
            bool success = await _port.Send(cmd.CommandBytes);
            if (!success) {
                throw new IOException("Command transmitting unsuccessfull");
            }
        }
        protected virtual void OnDataReceived(byte[] data)
        {
            // append received bytes to the buffer
            _receiveBuffer.Write(data, 0, data.Length);
            _receiveBuffer.Position = 0;

            // loop through the buffer length
            long lastPosition = _receiveBuffer.Position;
            while (_receiveBuffer.Position + 2 < _receiveBuffer.Length) {
                int nextByte = _receiveBuffer.ReadByte();

                if (nextByte == '$'
                    && _receiveBuffer.ReadByte() == '$'
                    && _receiveBuffer.ReadByte() == '$') {
                    // extract bytes up to "$$$" and send as Info

                    byte[] infoBytes = new byte[_receiveBuffer.Position - lastPosition];
                    _receiveBuffer.Position = lastPosition;
                    _receiveBuffer.Read(infoBytes, 0, infoBytes.Length);

                    BciInfoReceived?.Invoke(Encoding.ASCII.GetString(infoBytes));

                    lastPosition = _receiveBuffer.Position;
                }
                else if (nextByte == 0xA0 && _receiveBuffer.Length - (_receiveBuffer.Position - 1) >= 33) {
                    // extract the next 32 bytes after "0xA0" and send as Data

                    byte[] dataBytes = new byte[33];
                    dataBytes[0] = 0xA0;
                    _receiveBuffer.Read(dataBytes, 1, dataBytes.Length - 1);

                    BciDataReceived?.Invoke(BciData.FromBytes(dataBytes));

                    lastPosition = _receiveBuffer.Position;
                }
            }

            // rewrite the buffer with the rest data
            byte[] restData = _receiveBuffer.ToArray().Skip((int)lastPosition).ToArray();
            _receiveBuffer = new MemoryStream();
            _receiveBuffer.Write(restData, 0, restData.Length);
            _receiveBuffer.Position = 0;
        }
    }
}
