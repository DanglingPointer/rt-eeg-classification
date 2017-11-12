#pragma once
#include "ISpectralAnalysis.h"

using namespace Windows::Foundation;
using namespace Platform;

namespace Processing
{
   // Hilbert spectral analysis
   public ref class Hsa sealed
   {
      Hsa();

   public:
      static IAsyncOperation<ISpectralAnalysisDouble^>^ AnalyseAsync(const Array<double>^ xValues, const Array<double>^ yValues);

      static IAsyncOperation<ISpectralAnalysisSingle^>^ AnalyseAsync(const Array<float>^ xValues, const Array<float>^ yValues);
   };
}

