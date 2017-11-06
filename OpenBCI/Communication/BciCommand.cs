using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Communication
{
    public class BciCommand
    {
        public enum General
        {
            START_STREAM = 'b',
            STOP_STREAM = 's',
            RESET = 'v',
            USE_8CHANS = 'c',
            USE_16CHANS = 'C',
            CHAN_SETTINGS_DEFAULT = 'd',
            CHAN_SETTINGS_QUERY = 'D',
            REGS_SETTINGS_QUERY = '?'
        }
        public enum SDCard
        {
            LOG_5MIN = 'A',
            LOG_15MIN = 'S',
            LOG_30MIN = 'F',
            LOG_1HR = 'G',
            LOG_2HR = 'H',
            LOG_4HR = 'J',
            LOG_12HR = 'K',
            LOG_24HR = 'L',
            LOG_TEST = 'a',
            LOG_CLOSE = 'j'
        }
        public enum Channels
        {
            CHAN1_OFF = '1',
            CHAN2_OFF = '2',
            CHAN3_OFF = '3',
            CHAN4_OFF = '4',
            CHAN5_OFF = '5',
            CHAN6_OFF = '6',
            CHAN7_OFF = '7',
            CHAN8_OFF = '8',

            CHAN9_OFF = 'q',
            CHAN10_OFF = 'w',
            CHAN11_OFF = 'e',
            CHAN12_OFF = 'r',
            CHAN13_OFF = 't',
            CHAN14_OFF = 'y',
            CHAN15_OFF = 'u',
            CHAN16_OFF = 'i',

            CHAN1_ON = '!',
            CHAN2_ON = '@',
            CHAN3_ON = '#',
            CHAN4_ON = '$',
            CHAN5_ON = '%',
            CHAN6_ON = '^',
            CHAN7_ON = '%',
            CHAN8_ON = '*',

            CHAN9_ON = 'Q',
            CHAN10_ON = 'W',
            CHAN11_ON = 'E',
            CHAN12_ON = 'R',
            CHAN13_ON = 'T',
            CHAN14_ON = 'Y',
            CHAN15_ON = 'U',
            CHAN16_ON = 'I'
        }
        public enum SignalTests
        {
            TEST_GND = '0',
            TEST_1AMP_SLOW = '-',
            TEST_1AMP_FAST = '=',
            TEST_DC = 'p',
            TEST_2AMP_SLOW = '[',
            TEST_2AMP_FAST = ']'
        }
        public enum AdsInputType
        {
            NORMAL = '0',
            SHORTED = '1',
            BIAS_MEAS = '2',
            MVDD = '3',
            TEMP = '4',
            TESTSIG = '5',
            BIAS_DRP = '6',
            BIAS_DRN = '7'
        }
        public enum FirmwareV20
        {
            TIMESTAMP_ON = '<',
            TIMESTAMP_OFF = '>',
            GET_CHAN = 0x00,
            SET_CHAN = 0x01,
            SET_CHAN_OVERRIDE = 0x02,
            GET_POLL_TIME = 0x03,
            SET_POLL_TIME = 0x04,
            SET_BAUD_DEFAULT = 0x05,
            SET_BAUD_HIGH = 0x06,
            SET_BAUD_HYPER = 0x0A,
            GET_STATUS = 0x07
        }
        public enum ChannelNo
        {
            CHAN1 = '1',
            CHAN2 = '2',
            CHAN3 = '3',
            CHAN4 = '4',
            CHAN5 = '5',
            CHAN6 = '6',
            CHAN7 = '7',
            CHAN8 = '8',
            CHAN9 = 'Q',
            CHAN10 = 'W',
            CHAN11 = 'E',
            CHAN12 = 'R',
            CHAN13 = 'T',
            CHAN14 = 'Y',
            CHAN15 = 'U',
            CHAN16 = 'I'
        }
        public enum Gain
        {
            GAIN1 = '0',
            GAIN2 = '1',
            GAIN4 = '2',
            GAIN6 = '3',
            GAIN8 = '4',
            GAIN12 = '5',
            GAIN24 = '6'
        }
        //-----------------------------------------------------------------------------------------------------

        public static BciCommand Simple(General cmd)
        {
            return new BciCommand((char)cmd);
        }
        public static BciCommand Simple(Channels cmd)
        {
            return new BciCommand((char)cmd);
        }
        public static BciCommand Simple(SignalTests cmd)
        {
            return new BciCommand((char)cmd);
        }
        public static BciCommand Simple(AdsInputType cmd)
        {
            return new BciCommand((char)cmd);
        }
        public static BciCommand Simple(SDCard cmd)
        {
            return new BciCommand((char)cmd);
        }
        public static BciCommand LeadoffImpedance(ChannelNo channel, bool pchan, bool nchan)
        {
            return new BciCommand('z')
                .AddSymbol((char)channel)
                .AddSymbol(pchan ? '1' : '0')
                .AddSymbol(nchan ? '1' : '0')
                .AddSymbol('Z');
        }
        public static BciCommand ChannelSettings(ChannelNo channel, bool powerOn, Gain gainSet, AdsInputType ads, 
            bool bias, bool srb2, bool srb1)
        {
            return new BciCommand('x')
                .AddSymbol((char)channel)
                .AddSymbol(powerOn ? '0' : '1')
                .AddSymbol((char)gainSet)
                .AddSymbol((char)ads)
                .AddSymbol(bias ? '1' : '0')
                .AddSymbol(srb2 ? '1' : '0')
                .AddSymbol(srb1 ? '1' : '0')
                .AddSymbol('X');
        }
        public static BciCommand Firmware(FirmwareV20 cmd)
        {
            return new BciCommand((char)0x0F)
                .AddSymbol((char)cmd);
        }

        private String _data;

        private BciCommand(char cmd)
        {
            _data = "" + cmd;
        }
        private BciCommand AddSymbol(char cmd)
        {
            _data += cmd;
            return this;
        }
        public String CommandString
        {
            get { return _data; }
        }
        public byte[] CommandBytes
        {
            get { return Encoding.ASCII.GetBytes(_data); }
        }
    }
}
