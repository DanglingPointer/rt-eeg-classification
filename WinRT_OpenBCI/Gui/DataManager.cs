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
        /// <summary>
        /// Singleton instance
        /// </summary>
        public static DataManager Current { get; } = new DataManager();
        /// <summary>
        /// Current sample
        /// </summary>
        public ConcurrentQueue<BciData> Sample { get; private set; } = new ConcurrentQueue<BciData>();
        /// <summary>
        /// EMDs for each channel data
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
