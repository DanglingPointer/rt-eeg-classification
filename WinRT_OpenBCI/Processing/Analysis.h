#pragma once
#include <memory>
#include <complex>
#include <cmath>
#include "ISpectralAnalysis.h"

using namespace Platform;

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

namespace Processing
{
   inline int BitReversal(int value, int bitsCount)
   {
      int res = 0;
      for (int i = 0; i < bitsCount; ++i) {
         if (value & (1 << (bitsCount-1 - i)) )
            res |= (1 << i);
      }
      return res;
   }

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class FastFourierTransform
   {
      typedef TData Val; // value type
      typedef std::complex<Val> Cval; // complex value type
      typedef std::unique_ptr<Cval[]> CvalArr;

   public:
      // Uses zero-padding if length is not power of 2
      static CvalArr Forward(std::unique_ptr<Val[]> data, int length)
      {
         int newLength = 1;
         while (newLength < length)
            newLength *= 2;

         CvalArr input = std::make_unique<Cval[]>(newLength);

         int i;
         for (i = 0; i < length; ++i) {
            input[i] = Cval(data[i], 0.0);
         }
         for (; i < newLength; ++i) {
            input[i] = Cval(0.0, 0.0);
         }
         return RecursiveForward(std::move(input), length); // or IterativeForward
      }
      // Method 4 from here: https://www.dsprelated.com/showarticle/800.php
      // Length must be a power of 2
      static CvalArr Inverse(CvalArr data, int length)
      {
         for (int i = 0; i < length; ++i) {
            data[i].imag(-1 * data[i].imag());
         }
         data = IterativeForward(std::move(data), length);
         for (int i = 0; i < length; ++i) {
            data[i].imag(-1 * data[i].imag());
            data[i] /= length;
         }
         return std::move(data);
      }

   private:
      // "Introduction To Algorithms", p.917
      static CvalArr IterativeForward(CvalArr a, int n)
      {
         const Cval i(0, 1);

         int stageCount = std::log2(n);
         CvalArr A = std::make_unique<Cval[]>(n);
         for (int k = 0; k < n; ++k) {
            A[BitReversal(k, stageCount)] = a[k];
         }

         for (int s = 1; s <= stageCount; ++s) {
            int m = 1 << s;
            Cval wm = std::exp(2 * (Val)M_PI * i / (Val)m);

            concurrency::parallel_for(0, n, m, [&A, m, wm](int k) {
               Cval w(1, 0);
               for (int j = 0; j < m / 2; ++j) {
                  Cval t = w * A[k + j + m / 2];
                  Cval u = A[k + j];
                  A[k + j] = u + t;
                  A[k + j + m / 2] = u - t;
                  w *= wm;
               }
            });
         }
         return std::move(A);
      }
      // "Introduction To Algorithms", p.911
      static CvalArr RecursiveForward(CvalArr a, int n)
      {
         if (n == 1) {
            return std::move(a);
         }
         const Cval i(0, 1);
         const Cval wn = std::exp(2 * (Val)M_PI * i / (Val)n);

         CvalArr aEven = std::make_unique<Cval[]>(n / 2);    
         CvalArr aOdd = std::make_unique<Cval[]>(n / 2);

         auto evenTask = [&aEven, &a, n]() {
            int index = 0;
            for (int i = 0; i <= n - 2; i += 2) {
               aEven[index++] = a[i];
            }
            aEven = RecursiveForward(std::move(aEven), index);
         };
         auto oddTask = [&aOdd, &a, n]() {
            int index = 0;
            for (int i = 1; i <= n - 1; i += 2) {
               aOdd[index++] = a[i];
            }
            aOdd = RecursiveForward(std::move(aOdd), index);
         };
         if (n > 64) {
            concurrency::parallel_invoke(evenTask, oddTask);
         }
         else {
            evenTask();
            oddTask();
         }

         int halfN = n / 2;
         Cval w(1, 0);
         for (int k = 0; k < halfN; ++k) {
            Cval t = w * aOdd[k];
            a[k] = aEven[k] + t;
            a[k + halfN] = aEven[k] - t;
            w *= wn;
         }
         return std::move(a);
      }
   };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   private ref class SpectralAnalyzer : public ISpectralAnalysisDouble, public ISpectralAnalysisSingle
   {
      Array<TData>^ m_pAnalyticFunc;

      Array<TData>^ m_pInstAmpl;

      Array<TData>^ m_pInstPhas;

      Array<TData>^ m_pInstFreq;



   public:
      // Inherited via ISpectralAnalysisDouble
      virtual property Array<double>^ AnalyticFunctionD {
         Array<double>^ get() = ISpectralAnalysisDouble::AnalyticFunction::get
         {
            return reinterpret_cast<Array<double>^>(m_pAnalyticFunc);
         }
      }
      virtual property Array<double>^ InstAmplitudesD {
         Array<double>^ get() = ISpectralAnalysisDouble::InstAmplitudes::get
         {
            return reinterpret_cast<Array<double>^>(m_pInstAmpl);
         }
      }
      virtual property Array<double>^ InstPhasesD {
         Array<double>^ get() = ISpectralAnalysisDouble::InstPhases::get
         {
            return reinterpret_cast<Array<double>^>(m_pInstPhas);
         }
      }
      virtual property Array<double>^ InstFrequenciesD {
         Array<double>^ get() = ISpectralAnalysisDouble::InstFrequencies::get
         {
            return reinterpret_cast<Array<double>^>(m_pInstFreq);
         }
      }
      // Inherited via ISpectralAnalysisSingle
      virtual property Array<float>^ AnalyticFunctionS {
         Array<float>^ get() = ISpectralAnalysisSingle::AnalyticFunction::get
         {
            return reinterpret_cast<Array<float>^>(m_pAnalyticFunc);
         }
      }
      virtual property Array<float>^ InstAmplitudesS {
         Array<float>^ get() = ISpectralAnalysisSingle::InstAmplitudes::get
         {
            return reinterpret_cast<Array<float>^>(m_pInstAmpl);
         }
      }
      virtual property Array<float>^ InstPhasesS {
         Array<float>^ get() = ISpectralAnalysisSingle::InstPhases::get
         {
            return reinterpret_cast<Array<float>^>(m_pInstPhas);
         }
      }
      virtual property Array<float>^ InstFrequenciesS {
         Array<float>^ get() = ISpectralAnalysisSingle::InstFrequencies::get
         {
            return reinterpret_cast<Array<float>^>(m_pInstFreq);
         }
      }
   };

}