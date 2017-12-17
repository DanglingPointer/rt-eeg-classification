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
using namespace Windows::Foundation;
using namespace Platform;

namespace Processing
{
   template<typename TData>
   ref class Classifier;

   namespace Single
   {
      /// <summary>
      /// Single-precision classifier using Artificial Neural Networks
      /// </summary>
      public ref class Classifier sealed
      {
         Processing::Classifier<float>^ m_pc;

      public:
         Classifier();

         void CreateFixedSizeNetwork(int32 inputSize, int32 layerSize, int32 outputSize, int32 layerCount);

         void CreateCascadeNetwork(int32 inputSize, int32 outputSize);

         void AddExample(const Array<float>^ trainingInput, const Array<float>^ trainingOutput);

         IAsyncAction^ TrainAsync();

         IAsyncAction^ ClassifyAsync(const Array<float>^ data, WriteOnlyArray<float>^ output);

         void Classify(const Array<float>^ data, WriteOnlyArray<float>^ output);
      };
   }

   namespace Double
   {
      /// <summary>
      /// Double-precision classifier using Artificial Neural Networks
      /// </summary>
      public ref class Classifier sealed
      {
         Processing::Classifier<double>^ m_pc;

      public:
         Classifier();

         void CreateFixedSizeNetwork(int32 inputSize, int32 layerSize, int32 outputSize, int32 layerCount);

         void CreateCascadeNetwork(int32 inputSize, int32 outputSize);

         void AddExample(const Array<double>^ trainingInput, const Array<double>^ trainingOutput);

         IAsyncAction^ TrainAsync();

         IAsyncAction^ ClassifyAsync(const Array<double>^ data, WriteOnlyArray<double>^ output);

         void Classify(const Array<double>^ data, WriteOnlyArray<double>^ output);
      };
   }

}

