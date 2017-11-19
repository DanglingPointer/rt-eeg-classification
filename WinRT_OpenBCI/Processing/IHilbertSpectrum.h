#pragma once

namespace Processing
{
   public interface class IHilbertSpectrumDouble
   {
      property double MaxFrequency {
         double get();
      }
      property double MinFrequency {
         double get();
      }
      double ComputeAt(double t, double w);
      double ComputeMarginalAt(double w);
   };
   public interface class IHilbertSpectrumSingle
   {
      property float MaxFrequency {
         float get();
      }
      property float MinFrequency {
         float get();
      }
      float ComputeAt(float t, float w);
      float ComputeMarginalAt(float w);
   };
}