using Communication;
using Processing;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Gui
{
    public class DataManager
    {
        public const double ScaleFactor = 4.5d / 24.0d / 8388607.0d;
        /// <summary>
        /// Singleton instance
        /// </summary>
        public static DataManager Current { get; } = new DataManager();
        /// <summary>
        /// Current sample, unscaled
        /// </summary>
        public ConcurrentQueue<BciData> Sample { get; private set; } = new ConcurrentQueue<BciData>();
        /// <summary>
        /// EMDs for each channel data, scaled
        /// </summary>
        public IImfDecompositionDouble[] Emds { get; } = new IImfDecompositionDouble[8];
        /// <summary>
        /// Clear sample and Emds
        /// </summary>
        public void Reset()
        {
            Sample = new ConcurrentQueue<BciData>();
            for(int i = 0; i < 8; ++i) {
                Emds[i] = null;
            }
        }
        private DataManager()
        { }
    }
}
