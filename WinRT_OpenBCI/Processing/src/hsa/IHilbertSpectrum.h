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