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
      static IAsyncOperation<ISpectralAnalysisDouble^>^ AnalyseAsync(const Array<double>^ yValues, const Array<double>^ xValues);

      static IAsyncOperation<ISpectralAnalysisDouble^>^ AnalyseAsync(const Array<double>^ yValues, double timeStep);


      static IAsyncOperation<ISpectralAnalysisSingle^>^ AnalyseAsync(const Array<float>^ yValues, const Array<float>^ xValues);

      static IAsyncOperation<ISpectralAnalysisSingle^>^ AnalyseAsync(const Array<float>^ yValues, float timeStep);
   };
}

