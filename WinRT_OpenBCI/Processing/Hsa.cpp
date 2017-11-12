#include "pch.h"
#include "Analysis.h"
#include "Hsa.h"

using namespace Processing;
using namespace Platform;

[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
IAsyncOperation<ISpectralAnalysisDouble^>^ Processing::Hsa::AnalyseAsync(const Array<double>^ yValues, double timeStep)
{
   return concurrency::create_async([=]() {
      return static_cast<ISpectralAnalysisDouble^>(ref new SpectralAnalyzer<double>(yValues, timeStep));
   });
}

IAsyncOperation<ISpectralAnalysisDouble^>^ Hsa::AnalyseAsync(const Array<double>^ yValues, const Array<double>^ xValues)
{
   double dt = 0.0;
   for (uint32 i = 0; i < xValues->Length - 1; ++i) {
      dt += xValues[i + 1] - xValues[i];
   }
   dt /= (xValues->Length - 1.0);
   return AnalyseAsync(yValues, dt);
}


IAsyncOperation<ISpectralAnalysisSingle^>^ Processing::Hsa::AnalyseAsync(const Array<float>^ yValues, float timeStep)
{
   return concurrency::create_async([=]() {
      return static_cast<ISpectralAnalysisSingle^>(ref new SpectralAnalyzer<float>(yValues, timeStep));
   });
}
IAsyncOperation<ISpectralAnalysisSingle^>^ Hsa::AnalyseAsync(const Array<float>^ yValues, const Array<float>^ xValues)
{
   float dt = 0.0;
   for (uint32 i = 0; i < xValues->Length - 1; ++i) {
      dt += xValues[i + 1] - xValues[i];
   }
   dt /= (xValues->Length - 1.0f);
   return AnalyseAsync(yValues, dt);
}
