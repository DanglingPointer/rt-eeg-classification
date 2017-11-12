#pragma once

using namespace Platform;

namespace Processing
{
   public interface class ISpectralAnalysisDouble
   {
      property Array<double>^ AnalyticFunction {
         Array<double>^ get();
      }
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
   public interface class ISpectralAnalysisSingle
   {
      property Array<float>^ AnalyticFunction {
         Array<float>^ get();
      }
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