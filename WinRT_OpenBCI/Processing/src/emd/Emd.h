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
#pragma once
#include "IImfDecomposition.h"

using namespace Windows::Foundation;
using namespace Platform;

namespace Processing
{
   /// <summary>
   /// Empirical mode decomposition
   /// </summary>
   public ref class Emd sealed
   {
      Emd();

   public:
      /// <summary>
      /// Single-precision asynchronous decomposition
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="maxImfCount">Max number of intrinsic mode functions to extract. Used if less than log2 of data length</param>
      /// <returns>Intrinsic mode functions and residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ DecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, int maxImfCount);
      /// <summary>
      /// Single-precision asynchronous decomposition, extracting max log2(length) intrinsic mode functions
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <returns>Intrinsic mode functions and residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ DecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues);

      /// <summary>
      /// Double-precision asynchronous decomposition
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="maxImfCount">Max number of intrinsic mode functions to extract. Used if less than log2 of data length</param>
      /// <returns>Intrinsic mode functions and residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ DecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount);
      /// <summary>
      /// Double-precision asynchronous decomposition, extracting max log2(length) intrinsic mode functions
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <returns>Intrinsic mode functions and residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ DecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues);


      /// <summary>
      /// Single-precision asynchronous ensemble empirical mode decomposition
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="noiseSD">Standard deviation of the white noise</param>
      /// <param name="ensembleCount">Number of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD, int ensembleCount);
      /// <summary>
      /// Single-precision asynchronous ensemble empirical mode decomposition using 100 ensembles
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="noiseSD">Standard deviation of the white noise</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD);
      /// <summary>
      /// Single-precision asynchronous ensemble empirical mode decomposition using 100 ensembles and white noise standard deviation equal 1.0
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues);

      /// <summary>
      /// Double-precision asynchronous ensemble empirical mode decomposition
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="noiseSD">Standard deviation of the white noise</param>
      /// <param name="ensembleCount">Number of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD, int ensembleCount);
      /// <summary>
      /// Double-precision asynchronous ensemble empirical mode decomposition using 100 ensembles
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="noiseSD">Standard deviation of the white noise</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD);
      /// <summary>
      /// Double-precision asynchronous ensemble empirical mode decomposition using 100 ensembles and white noise standard deviation equal 1.0
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues);
   };
}
