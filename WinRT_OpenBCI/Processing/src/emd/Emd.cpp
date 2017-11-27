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
#include "ISpectralAnalysis.h"
#include "Emd.h"
#include "Decomposition.h"

using namespace Processing;
using namespace Platform;

//-------------------------------------------------------------------------------------------------------------------------------------------------

IAsyncOperation<IImfDecompositionSingle^>^ Emd::DecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, int maxImfCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecompositionSingle^>(ref new EmdDecomposer<float>(xValues, yValues, maxImfCount));
   });
}

inline IAsyncOperation<IImfDecompositionSingle^>^ Emd::DecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues)
{
   return Emd::DecomposeAsync(xValues, yValues, INT_MAX);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
IAsyncOperation<IImfDecompositionDouble^>^ Emd::DecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecompositionDouble^>(ref new EmdDecomposer<double>(xValues, yValues, maxImfCount));
   });
}

inline IAsyncOperation<IImfDecompositionDouble^>^ Emd::DecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::DecomposeAsync(xValues, yValues, INT_MAX);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------

IAsyncOperation<IImfDecompositionSingle^>^ Emd::EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, float noiseSD, int ensembleCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecompositionSingle^>(ref new EemdDecomposer<float>(xValues, yValues, ensembleCount, noiseSD));
   });
}

inline IAsyncOperation<IImfDecompositionSingle^>^ Emd::EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, float noiseSD)
{
   return Emd::EnsembleDecomposeAsync(xValues, yValues, noiseSD, 100);
}

inline IAsyncOperation<IImfDecompositionSingle^>^ Emd::EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues)
{
   return Emd::EnsembleDecomposeAsync(xValues, yValues, 1.0f);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

[Windows::Foundation::Metadata::DefaultOverloadAttribute()]
IAsyncOperation<IImfDecompositionDouble^>^ Emd::EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, double noiseSD, int ensembleCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecompositionDouble^>(ref new EemdDecomposer<double>(xValues, yValues, ensembleCount, noiseSD));
   });
}

inline IAsyncOperation<IImfDecompositionDouble^>^ Emd::EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, double noiseSD)
{
   return Emd::EnsembleDecomposeAsync(xValues, yValues, noiseSD, 100);
}

inline IAsyncOperation<IImfDecompositionDouble^>^ Emd::EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::EnsembleDecomposeAsync(xValues, yValues, 1.0f);
}

