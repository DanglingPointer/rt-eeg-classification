#pragma once
#include <limits>
#include <ppltasks.h>
#include "IImfDecomposition.h"

using namespace Windows::Foundation;

namespace Processing
{
   public ref class Emd sealed
   {
      Emd();

   public:
      static IAsyncOperation<IImfDecomposition^>^ SingleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, int maxImfCount);

      static IAsyncOperation<IImfDecomposition^>^ SingleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues);


      static IAsyncOperation<IImfDecomposition^>^ DoubleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount);

      static IAsyncOperation<IImfDecomposition^>^ DoubleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues);



      static IAsyncOperation<IImfDecomposition^>^ SingleEnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD, int ensembleCount);

      static IAsyncOperation<IImfDecomposition^>^ SingleEnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD);

      static IAsyncOperation<IImfDecomposition^>^ SingleEnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues);



      static IAsyncOperation<IImfDecomposition^>^ DoubleEnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD, int ensembleCount);

      static IAsyncOperation<IImfDecomposition^>^ DoubleEnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD);

      static IAsyncOperation<IImfDecomposition^>^ DoubleEnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues);
   };
}
