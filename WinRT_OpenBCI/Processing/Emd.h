#pragma once
#include <limits>
#include "IImfDecomposition.h"

namespace Processing
{
   public ref class Emd sealed
   {
      Emd();

   public:
      static IImfDecomposition^ SingleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount);

      static IImfDecomposition^ SingleDecompose(const Array<double>^ xValues, const Array<double>^ yValues);

      static IImfDecomposition^ DoubleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount);

      static IImfDecomposition^ DoubleDecompose(const Array<double>^ xValues, const Array<double>^ yValues);



      static IImfDecomposition^ SingleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        int ensembleCount, double noiseSD);

      static IImfDecomposition^ SingleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        int ensembleCount);

      static IImfDecomposition^ SingleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues);



      static IImfDecomposition^ DoubleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        int ensembleCount, double noiseSD);

      static IImfDecomposition^ DoubleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues,
                                                        int ensembleCount);

      static IImfDecomposition^ DoubleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues);
   };
}
