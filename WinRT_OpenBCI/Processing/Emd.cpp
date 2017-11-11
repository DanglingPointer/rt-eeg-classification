#include "pch.h"
#include "Emd.h"
#include "Decomposition.h"

using namespace Processing;
using namespace Platform;

//-------------------------------------------------------------------------------------------------------------------------------------------------

IAsyncOperation<IImfDecomposition^>^ Emd::SingleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, int maxImfCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecomposition^>(ref new EmdDecomposer<float>(xValues, yValues, maxImfCount));
   });
}

inline IAsyncOperation<IImfDecomposition^>^ Emd::SingleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues)
{
   return Emd::SingleDecomposeAsync(xValues, yValues, INT_MAX);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

IAsyncOperation<IImfDecomposition^>^ Emd::DoubleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecomposition^>(ref new EmdDecomposer<double>(xValues, yValues, maxImfCount));
   });
}

inline IAsyncOperation<IImfDecomposition^>^ Emd::DoubleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::DoubleDecomposeAsync(xValues, yValues, INT_MAX);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------

IAsyncOperation<IImfDecomposition^>^ Emd::SingleEnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, float noiseSD, int ensembleCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecomposition^>(ref new EemdDecomposer<float>(xValues, yValues, ensembleCount, noiseSD));
   });
}

inline IAsyncOperation<IImfDecomposition^>^ Emd::SingleEnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues, float noiseSD)
{
   return Emd::SingleEnsembleDecomposeAsync(xValues, yValues, noiseSD, 100);
}

inline IAsyncOperation<IImfDecomposition^>^ Emd::SingleEnsembleDecomposeAsync(const Array<float>^ xValues, const Array<float>^ yValues)
{
   return Emd::SingleEnsembleDecomposeAsync(xValues, yValues, 1.0f);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

IAsyncOperation<IImfDecomposition^>^ Emd::DoubleEnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, double noiseSD, int ensembleCount)
{
   return concurrency::create_async([=]() {
      return static_cast<IImfDecomposition^>(ref new EemdDecomposer<double>(xValues, yValues, ensembleCount, noiseSD));
   });
}

inline IAsyncOperation<IImfDecomposition^>^ Emd::DoubleEnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues, double noiseSD)
{
   return Emd::DoubleEnsembleDecomposeAsync(xValues, yValues, noiseSD, 100);
}

inline IAsyncOperation<IImfDecomposition^>^ Emd::DoubleEnsembleDecomposeAsync(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::DoubleEnsembleDecomposeAsync(xValues, yValues, 1.0f);
}

