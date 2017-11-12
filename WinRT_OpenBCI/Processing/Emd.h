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
      /// <param name="ensembleCount">NUmber of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD, int ensembleCount);
      /// <summary>
      /// Single-precision asynchronous ensemble empirical mode decomposition using 100 ensembles
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="ensembleCount">NUmber of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD);
      /// <summary>
      /// Single-precision asynchronous ensemble empirical mode decomposition using 100 ensembles and white noise standard deviation equal 1.0
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="ensembleCount">NUmber of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues);

      /// <summary>
      /// Double-precision asynchronous ensemble empirical mode decomposition
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="noiseSD">Standard deviation of the white noise</param>
      /// <param name="ensembleCount">NUmber of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD, int ensembleCount);
      /// <summary>
      /// Double-precision asynchronous ensemble empirical mode decomposition using 100 ensembles
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="ensembleCount">NUmber of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD);
      /// <summary>
      /// Double-precision asynchronous ensemble empirical mode decomposition using 100 ensembles and white noise standard deviation equal 1.0
      /// </summary>
      /// <param name="xValues">Data x-axis</param>
      /// <param name="yValues">Data y-axis</param>
      /// <param name="ensembleCount">NUmber of ensembles to use</param>
      /// <returns>Max log2(length) intrinsic mode functions, no residue</returns>
      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues);
   };
}
