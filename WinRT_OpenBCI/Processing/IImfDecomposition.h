#pragma once

using namespace Platform;
using namespace Windows::Foundation::Collections;

namespace Processing
{
   public interface class IImfDecompositionDouble
   {
      property IVector<IVector<double>^>^ ImfFunctions {
         IVector<IVector<double>^>^ get();
      }
      property Array<double>^ ResidueFunction {
         Array<double>^ get();
      }
   };

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