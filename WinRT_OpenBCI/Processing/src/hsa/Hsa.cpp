/*
*    Copyright 2017 Mikhail Vasilyev
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*/
#include "pch.h"
#include "Analysis.h"
#include "Hsa.h"

using namespace Processing;
using namespace Platform;

template<typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
inline TData MeanStep(const Array<TData>^ pdata)
{
   TData dt = (TData)0.0;
   for (uint32 i = 0; i < pdata->Length - 1; ++i) {
      dt += pdata[i + 1] - pdata[i];
   }
   dt /= (pdata->Length - (TData)1.0);
   return dt;
}

[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
Double::ISpectralAnalysis ^ Hsa::Analyse(const Array<double>^ yValues, double timeStep)
{
   return ref new SpectralAnalyzer<double>(yValues, timeStep);
}
[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
inline IAsyncOperation<Double::ISpectralAnalysis^>^ Hsa::AnalyseAsync(const Array<double>^ yValues, double timeStep)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, timeStep);
   });
}

Double::ISpectralAnalysis ^ Hsa::Analyse(const Array<double>^ yValues, const Array<double>^ xValues)
{
   return Hsa::Analyse(yValues, MeanStep(xValues));
}
inline IAsyncOperation<Double::ISpectralAnalysis^>^ Hsa::AnalyseAsync(const Array<double>^ yValues, const Array<double>^ xValues)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, xValues);
   });
}

Single::ISpectralAnalysis ^ Hsa::Analyse(const Array<float>^ yValues, float timeStep)
{
   return ref new SpectralAnalyzer<float>(yValues, timeStep);
}
inline IAsyncOperation<Single::ISpectralAnalysis^>^ Hsa::AnalyseAsync(const Array<float>^ yValues, float timeStep)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, timeStep);
   });
}

Single::ISpectralAnalysis ^ Hsa::Analyse(const Array<float>^ yValues, const Array<float>^ xValues)
{
   return ref new SpectralAnalyzer<float>(yValues, MeanStep(xValues));
}
inline IAsyncOperation<Single::ISpectralAnalysis^>^ Hsa::AnalyseAsync(const Array<float>^ yValues, const Array<float>^ xValues)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, xValues);
   });
}


[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
Double::IHilbertSpectrum ^ Hsa::GetHilbertSpectrum(Double::IImfDecomposition ^ emd, double timestep)
{
   return ref new HilbertSpectrum<double>(emd->ImfFunctions, timestep);
}
[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
inline IAsyncOperation<Double::IHilbertSpectrum^>^ Hsa::GetHilbertSpectrumAsync(Double::IImfDecomposition ^ emd, double timestep)
{
   return concurrency::create_async([=]() {
      return Hsa::GetHilbertSpectrum(emd, timestep);
   });
}

Single::IHilbertSpectrum ^ Hsa::GetHilbertSpectrum(Single::IImfDecomposition ^ emd, float timestep)
{
   return ref new HilbertSpectrum<float>(emd->ImfFunctions, timestep);
}
inline IAsyncOperation<Single::IHilbertSpectrum^>^ Hsa::GetHilbertSpectrumAsync(Single::IImfDecomposition ^ emd, float timestep)
{
   return concurrency::create_async([=]() {
      return Hsa::GetHilbertSpectrum(emd, timestep);
   });
}

Double::IHilbertSpectrum ^ Hsa::GetHilbertSpectrum(Double::IImfDecomposition ^ emd, const Array<double>^ xValues)
{
   return ref new HilbertSpectrum<double>(emd->ImfFunctions, MeanStep(xValues));
}
inline IAsyncOperation<Double::IHilbertSpectrum^>^ Hsa::GetHilbertSpectrumAsync(Double::IImfDecomposition ^ emd, const Array<double>^ xValues)
{
   return concurrency::create_async([=]() {
      return Hsa::GetHilbertSpectrum(emd, xValues);
   });
}

Single::IHilbertSpectrum ^ Hsa::GetHilbertSpectrum(Single::IImfDecomposition ^ emd, const Array<float>^ xValues)
{
   return ref new HilbertSpectrum<float>(emd->ImfFunctions, MeanStep(xValues));
}
inline IAsyncOperation<Single::IHilbertSpectrum^>^ Hsa::GetHilbertSpectrumAsync(Single::IImfDecomposition ^ emd, const Array<float>^ xValues)
{
   return concurrency::create_async([=]() {
      return Hsa::GetHilbertSpectrum(emd, xValues);
   });
}

