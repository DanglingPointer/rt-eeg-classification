#pragma once

using namespace Platform;

namespace Processing
{
   // Double-precision results of a spectral analysis
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

   // Single-precision results of a spectral analysis
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