using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Communication
{
    public class SerialManager
    {
        private readonly SerialPort _port;
        private MemoryStream _receiveBuffer;

        public event Action<BciData> BciDataReceived;
        public event Action<string> BciInfoReceived;

        public static SerialManager CreateAny()
        {
            string[] ports = SerialPort.GetPortNames();
            if (ports.Length != 1)
                throw new IOException();
            return new SerialManager(ports[0]);
        }
        public static SerialManager CreateFor(string port)
        {
            return new SerialManager(port);
        }
        public void OpenPort()
        {
            if (!_port.IsOpen) {
                _port.Open();
            }
        }
        public void ClosePort()
        {
            if (_port.IsOpen) {
                _port.Close();
            }
        }
        public void SendCommand(BciCommand cmd)
        {
            if (!_port.IsOpen)
                throw new InvalidOperationException();
            _port.Write(cmd.CommandBytes, 0, cmd.CommandBytes.Length);
        }
        private SerialManager(string portName)
        {
            _port = new SerialPort(portName, 115200); // throws IOException
            _receiveBuffer = new MemoryStream();

            _port.DataReceived += (sender, e) => {
                // append received bytes to the buffer
                byte[] tempBuffer = new byte[_port.BytesToRead];
                int bytesRead = _port.Read(tempBuffer, 0, tempBuffer.Length);
                _receiveBuffer.Write(tempBuffer, 0, bytesRead);
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
            };
        }
    }
}
