#pragma once
#include "ISpectralAnalysis.h"

using namespace Windows::Foundation;
using namespace Platform;

namespace Processing
{
   /// <summary>
   /// Hilbert spectral analysis
   /// </summary>
   public ref class Hsa sealed
   {
      Hsa();

   public:
      /// <summary>
      /// Double-precision asynchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="xValues">Used to calculate intervals between data points</param>
      /// <returns>Analysis results</returns>
      static IAsyncOperation<ISpectralAnalysisDouble^>^ AnalyseAsync(const Array<double>^ yValues, const Array<double>^ xValues);
      /// <summary>
      /// Double-precision asynchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="timeStep">Average interval between data points</param>
      /// <returns>Analysis results</returns>
      static IAsyncOperation<ISpectralAnalysisDouble^>^ AnalyseAsync(const Array<double>^ yValues, double timeStep);
      /// <summary>
      /// Single-precision asynchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="xValues">Used to calculate intervals between data points</param>
      /// <returns>Analysis results</returns>
      static IAsyncOperation<ISpectralAnalysisSingle^>^ AnalyseAsync(const Array<float>^ yValues, const Array<float>^ xValues);
      /// <summary>
      /// Single-precision asynchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="timeStep">Average interval between data points</param>
      /// <returns>Analysis results</returns>
      static IAsyncOperation<ISpectralAnalysisSingle^>^ AnalyseAsync(const Array<float>^ yValues, float timeStep);
   };
}

