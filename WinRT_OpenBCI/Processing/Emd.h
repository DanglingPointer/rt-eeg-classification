#pragma once
//#include <ppltasks.h>
#include "IImfDecomposition.h"

using namespace Windows::Foundation;
using namespace Platform;

namespace Processing
{
   public ref class Emd sealed
   {
      Emd();

   public:
      static IAsyncOperation<IImfDecompositionSingle^>^ DecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, int maxImfCount);

      static IAsyncOperation<IImfDecompositionSingle^>^ DecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues);


      static IAsyncOperation<IImfDecompositionDouble^>^ DecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount);

      static IAsyncOperation<IImfDecompositionDouble^>^ DecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues);



      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD, int ensembleCount);

      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues,
                                                        float noiseSD);

      static IAsyncOperation<IImfDecompositionSingle^>^ EnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues);


      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD, int ensembleCount);

      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        double noiseSD);

      static IAsyncOperation<IImfDecompositionDouble^>^ EnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues);
   };
}
