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
using namespace Windows::Foundation::Collections;

namespace Processing
{
   /// <summary>
   /// Double-precision results of an Empirical Mode decomposiiton
   /// </summary>
   public interface class IImfDecompositionDouble
   {
      property IVector<IVector<double>^>^ ImfFunctions {
         IVector<IVector<double>^>^ get();
      }
      property Array<double>^ ResidueFunction {
         Array<double>^ get();
      }
   };
   /// <summary>
   /// Single-precision results of an Empirical Mode decomposiiton
   /// </summary>
   public interface class IImfDecompositionSingle
   {
      property IVector<IVector<float>^>^ ImfFunctions {
         IVector<IVector<float>^>^ get();
      }
      property Array<float>^ ResidueFunction {
         Array<float>^ get();
      }
   };
}