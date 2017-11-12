#include "pch.h"
#include "Analysis.h"
#include "Hsa.h"

using namespace Processing;
using namespace Platform;

IAsyncOperation<ISpectralAnalysisDouble^>^ Hsa::AnalyseAsync(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return concurrency::create_async([=]() {
      return static_cast<ISpectralAnalysisDouble^>(ref new SpectralAnalyzer<double>());
   });
}

IAsyncOperation<ISpectralAnalysisSingle^>^ Hsa::AnalyseAsync(const Array<float>^ xValues, const Array<float>^ yValues)
{
   return concurrency::create_async([=]() {
      return static_cast<ISpectralAnalysisSingle^>(ref new SpectralAnalyzer<float>());
   });
}
