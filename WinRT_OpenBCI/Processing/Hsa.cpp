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

[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
ISpectralAnalysisDouble ^ Hsa::Analyse(const Array<double>^ yValues, double timeStep)
{
   return static_cast<ISpectralAnalysisDouble^>(ref new SpectralAnalyzer<double>(yValues, timeStep));
}
[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
inline IAsyncOperation<ISpectralAnalysisDouble^>^ Hsa::AnalyseAsync(const Array<double>^ yValues, double timeStep)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, timeStep);
   });
}


ISpectralAnalysisDouble ^ Hsa::Analyse(const Array<double>^ yValues, const Array<double>^ xValues)
{
   double dt = 0.0;
   for (uint32 i = 0; i < xValues->Length - 1; ++i) {
      dt += xValues[i + 1] - xValues[i];
   }
   dt /= (xValues->Length - 1.0);
   return Hsa::Analyse(yValues, dt);
}
inline IAsyncOperation<ISpectralAnalysisDouble^>^ Hsa::AnalyseAsync(const Array<double>^ yValues, const Array<double>^ xValues)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, xValues);
   });
}



ISpectralAnalysisSingle ^ Hsa::Analyse(const Array<float>^ yValues, float timeStep)
{
   return static_cast<ISpectralAnalysisSingle^>(ref new SpectralAnalyzer<float>(yValues, timeStep));
}
inline IAsyncOperation<ISpectralAnalysisSingle^>^ Hsa::AnalyseAsync(const Array<float>^ yValues, float timeStep)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, timeStep);
   });
}


ISpectralAnalysisSingle ^ Hsa::Analyse(const Array<float>^ yValues, const Array<float>^ xValues)
{
   float dt = 0.0;
   for (uint32 i = 0; i < xValues->Length - 1; ++i) {
      dt += xValues[i + 1] - xValues[i];
   }
   dt /= (xValues->Length - 1.0f);
   return static_cast<ISpectralAnalysisSingle^>(ref new SpectralAnalyzer<float>(yValues, dt));
}
inline IAsyncOperation<ISpectralAnalysisSingle^>^ Hsa::AnalyseAsync(const Array<float>^ yValues, const Array<float>^ xValues)
{
   return concurrency::create_async([=]() {
      return Hsa::Analyse(yValues, xValues);
   });
}


