#pragma once
#include <collection.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

namespace Processing
{
   public interface class IImfDecomposition
   {
      property IVector<IVector<double>^>^ ImfFunctions {
         IVector<IVector<double>^>^ get();
      }
      property Array<double>^ ResidueFunction {
         Array<double>^ get();
      }
   };
}