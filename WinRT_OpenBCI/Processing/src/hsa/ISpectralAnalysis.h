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

using namespace Platform;

namespace Processing
{
   namespace Double
   {
      /// <summary>
      /// Double-precision results of a spectral analysis
      /// </summary>
      public interface class ISpectralAnalysis
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
   }
   namespace Single
   {
      /// <summary>
      /// Single-precision results of a spectral analysis
      /// </summary>
      public interface class ISpectralAnalysis
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
}