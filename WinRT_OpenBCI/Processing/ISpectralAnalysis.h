#pragma once

using namespace Platform;

namespace Processing
{
   /// <summary>
   /// Double-precision results of a spectral analysis
   /// </summary>
   public interface class ISpectralAnalysisDouble
   {
      property Array<double>^ InstAmplitudes {
         Array<double>^ get();
      }
      property Array<double>^ InstPhases {
         Array<double>^ get();
      }
      property Array<double>^ InstFrequencies {
         Array<double>^ get();
      }
   };
   /// <summary>
   /// Single-precision results of a spectral analysis
   /// </summary>
   public interface class ISpectralAnalysisSingle
   {
      property Array<float>^ InstAmplitudes {
         Array<float>^ get();
      }
      property Array<float>^ InstPhases {
         Array<float>^ get();
      }
      property Array<float>^ InstFrequencies {
         Array<float>^ get();
      }
   };
}