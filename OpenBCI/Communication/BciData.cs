using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Communication
{
    public class BciData
    {
        public static BciData FromBytes(byte[] rawData)
        {
            return new BciData(rawData);
        }
        public Byte SampleNo { get; }
        public Int32[] ChannelData { get; } = new Int32[8];
        public Int16[] AcclXYZ { get; } = new Int16[3];
        public UInt32 Timestamp { get; }
        public bool TimestampSet { get; }

        private BciData(byte[] rawData)
        {
            if (rawData == null || rawData.Length != 33 || rawData[0] != 0xA0) {
                throw new ArgumentException();
            }
            SampleNo = rawData[1];
            
            for (int rawIndex = 2, channelIndex = 0; rawIndex <=23; rawIndex += 3) {
                byte[] value = new byte[3];
                Array.Copy(rawData, rawIndex, value, 0, value.Length);
                ChannelData[channelIndex++] = Int24ToInt32(value);
            }

            byte stopByte = rawData[32];
            if (stopByte == 0xC0) {
                AcclXYZ[0] = (Int16)(rawData[26] << 8 | rawData[27]);      
                AcclXYZ[1] = (Int16)(rawData[28] << 8 | rawData[29]);
                AcclXYZ[2] = (Int16)(rawData[30] << 8 | rawData[31]);
            }
            else if (stopByte >= 0xC3 && stopByte <= 0xC6) {
                Timestamp = (UInt32)(
                    (rawData[28] << 24) |
                    (rawData[29] << 16) |
                    (rawData[30] << 8)  |
                    (rawData[31])
                );
            }
            TimestampSet = stopByte == 0xC3 || stopByte == 0xC5;
        }
        private static Int32 Int24ToInt32(byte[] int24)
        {
            UInt32 result = (UInt32)(int24[0] << 16 | int24[1] << 8 | int24[2]);
            if ((result & 0x00800000) > 0)
                result |= 0xFF000000;
            else
                result &= 0x00FFFFFF;
            return (Int32)result;
        }
    }
}