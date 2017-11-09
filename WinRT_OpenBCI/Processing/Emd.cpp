#include "pch.h"
#include "Emd.h"
#include "Decomposition.h"

using namespace Processing;
using namespace Platform;

//-------------------------------------------------------------------------------------------------------------------------------------------------

IImfDecomposition ^ Emd::SingleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount)
{
   return ref new EmdDecomposer<float>(xValues, yValues, maxImfCount);
}

inline IImfDecomposition ^ Emd::SingleDecompose(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::SingleDecompose(xValues, yValues, INT_MAX);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

IImfDecomposition ^ Processing::Emd::DoubleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount)
{
   return ref new EmdDecomposer<double>(xValues, yValues, maxImfCount);
}

inline IImfDecomposition ^ Processing::Emd::DoubleDecompose(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::DoubleDecompose(xValues, yValues, INT_MAX);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------

IImfDecomposition ^ Emd::SingleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int ensembleCount, double noiseSD)
{
   return ref new EemdDecomposer<float>(xValues, yValues, ensembleCount, noiseSD);
}

inline IImfDecomposition ^ Emd::SingleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int ensembleCount)
{
   return Emd::SingleEnsembleDecompose(xValues, yValues, ensembleCount, 0.5);
}

inline IImfDecomposition ^ Emd::SingleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::SingleEnsembleDecompose(xValues, yValues, 100);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

IImfDecomposition ^ Emd::DoubleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int ensembleCount, double noiseSD)
{
   return ref new EemdDecomposer<double>(xValues, yValues, ensembleCount, noiseSD);
}

inline IImfDecomposition ^ Emd::DoubleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues, int ensembleCount)
{
   return Emd::DoubleEnsembleDecompose(xValues, yValues, ensembleCount, 0.5);
}

inline IImfDecomposition ^ Emd::DoubleEnsembleDecompose(const Array<double>^ xValues, const Array<double>^ yValues)
{
   return Emd::DoubleEnsembleDecompose(xValues, yValues, 100);
}

