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
#include "ISpectralAnalysis.h"
#include "IImfDecomposition.h"
#include "IHilbertSpectrum.h"

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
      /// Double-precision synchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="xValues">Used to calculate intervals between data points</param>
      /// <returns>Analysis results</returns>
      static ISpectralAnalysisDouble^ Analyse(const Array<double>^ yValues, const Array<double>^ xValues);
      /// <summary>
      /// Double-precision synchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="timeStep">Average interval between data points</param>
      /// <returns>Analysis results</returns>
      static ISpectralAnalysisDouble^ Analyse(const Array<double>^ yValues, double timeStep);

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
      /// Single-precision synchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="xValues">Used to calculate intervals between data points</param>
      /// <returns>Analysis results</returns>
      static ISpectralAnalysisSingle^ Analyse(const Array<float>^ yValues, const Array<float>^ xValues);
      /// <summary>
      /// Single-precision synchronous Hilbert analysis
      /// </summary>
      /// <param name="yValues">Data to analyse</param>
      /// <param name="timeStep">Average interval between data points</param>
      /// <returns>Analysis results</returns>
      static ISpectralAnalysisSingle^ Analyse(const Array<float>^ yValues, float timeStep);

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


      /// <summary>
      /// 
      /// </summary>
      /// <param name="emd"></param>
      /// <returns></returns>
      static IHilbertSpectrumDouble^ GetHilbertSpectrum(const IImfDecompositionDouble^ emd);
      /// <summary>
      /// 
      /// </summary>
      /// <param name="emd"></param>
      /// <returns></returns>
      static IHilbertSpectrumSingle^ GetHilbertSpectrum(const IImfDecompositionSingle^ emd);

      /// <summary>
      /// 
      /// </summary>
      /// <param name="emd"></param>
      /// <returns></returns>
      static IAsyncOperation<IHilbertSpectrumDouble^>^ GetHilbertSpectrumAsync(const IImfDecompositionDouble^ emd);
      /// <summary>
      /// 
      /// </summary>
      static IAsyncOperation<IHilbertSpectrumSingle^>^ GetHilbertSpectrumAsync(const IImfDecompositionSingle^ emd);
   };
}

