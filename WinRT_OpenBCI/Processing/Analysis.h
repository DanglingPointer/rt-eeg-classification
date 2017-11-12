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
#pragma region Numerical Transforms

   template <typename TInt, typename = std::enable_if<std::is_integral_v<TInt>>>
   inline TInt BitReversal(TInt value, size_t bitsCount)
   {
      TInt res = 0;
      for (size_t i = 0; i < bitsCount; ++i) {
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
      static CvalArr Forward(std::unique_ptr<Val[]> data, int length, int *pResultLength)
      {
         *pResultLength = 1;
         while (*pResultLength < length)
            *pResultLength *= 2;

         CvalArr input = std::make_unique<Cval[]>(*pResultLength);

         int i;
         for (i = 0; i < length; ++i) {
            input[i] = Cval(data[i], 0.0);
         }
         for (; i < *pResultLength; ++i) {
            input[i] = Cval(0.0, 0.0);
         }
         return IterativeForward(std::move(input), *pResultLength); // or RecursiveForward()
      }
      // Method 4 from here: https://www.dsprelated.com/showarticle/800.php
      // Length must be a power of 2
      static CvalArr Inverse(CvalArr data, int length)
      {
         for (int i = 0; i < length; ++i) {
            data[i].imag(-1 * data[i].imag());
         }
         data = IterativeForward(std::move(data), length); // or RecursiveForward()
         for (int i = 0; i < length; ++i) {
            data[i].imag(-1 * data[i].imag());
            data[i] /= (Val)length;
         }
         return std::move(data);
      }

   private:
      // "Introduction To Algorithms", p.917
      // n must be a power of 2
      static CvalArr IterativeForward(CvalArr a, int n)
      {
         const Cval i(0, 1);

         int stageCount = std::log2(n);
         CvalArr A = std::make_unique<Cval[]>(n);
         for (int k = 0; k < n; ++k) {
            A[BitReversal<int>(k, stageCount)] = a[k];
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
      // n must be a power of 2
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
   class HilbertTransform
   {
      typedef TData Val; // value type
      typedef std::unique_ptr<Val[]> ValArr;

      typedef std::complex<Val> Cval; // complex value type
      typedef std::unique_ptr<Cval[]> CvalArr;

   public:
      // Matlab algorithm: https://se.mathworks.com/help/signal/ref/hilbert.html#f7-960889
      // The result array is at least as long as the input
      static CvalArr Forward(ValArr realData, int length)
      {
         int fftLength;
         CvalArr ffted = FastFourierTransform<Val>::Forward(std::move(realData), length, &fftLength);
         if (fftLength > 1) {
            int i = 1;                     // ffted[0] is unchanged
            for (; i < fftLength / 2; ++i) // the rest of the first half is doubled
               ffted[i] *= 2.0;
            i++;                           // ffted[fftLength/2] is unchanged
            for (; i < fftLength; ++i)     // the rest are zeros
               ffted[i] = 0.0;
         }
         CvalArr iffted = FastFourierTransform<Val>::Inverse(std::move(ffted), fftLength);
         return std::move(iffted);
      }
   };

#pragma endregion


#pragma region C++/CX ref classes

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   private ref class SpectralAnalyzer : public ISpectralAnalysisDouble, public ISpectralAnalysisSingle
   {
      typedef std::unique_ptr<TData[]> Uptr;
      typedef std::unique_ptr<std::complex<TData>[]> Cuptr;

      const int m_length;
      Array<TData>^ m_pInstAmpl;
      Array<TData>^ m_pInstPhas;
      Array<TData>^ m_pInstFreq;

   internal:
      SpectralAnalyzer(const Array<TData>^ yValues, TData timeStep) : m_length(yValues->Length),
         m_pInstAmpl(ref new Array<TData>(m_length)), m_pInstPhas(ref new Array<TData>(m_length)), m_pInstFreq(ref new Array<TData>(m_length - 1))
      {
         Uptr pdata = std::make_unique<TData[]>(m_length);
         memcpy(pdata.get(), yValues->Data, sizeof(TData) * m_length);

         Cuptr hilberted = HilbertTransform<TData>::Forward(std::move(pdata), m_length);

         auto fillAmplTask = [&hilberted, this]() {
            for (int i = 0; i < m_length; ++i)
               this->m_pInstAmpl[i] = std::sqrt(hilberted[i].real() * hilberted[i].real() + hilberted[i].imag() * hilberted[i].imag());            
         };
         auto fillPhasFreqTask = [&hilberted, this, timeStep]() {
            this->m_pInstPhas[0] = std::atan2(hilberted[0].imag(), hilberted[0].real());
            for (int i = 1; i < m_length; ++i) {
               this->m_pInstPhas[i] = std::atan2(hilberted[i].imag(), hilberted[i].real());
               this->m_pInstFreq[i - 1] = (m_pInstPhas[i] - m_pInstPhas[i - 1]) / timeStep;
            }
         };

         if (m_length >= 100) {
            concurrency::parallel_invoke(fillPhasFreqTask, fillAmplTask);
         }
         else {
            fillAmplTask();
            fillPhasFreqTask();
         }
      }

   public:
      // Inherited via ISpectralAnalysisDouble
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

   //template<typename TData>
   //private ref class SpectralAnalyzerWrapper;

   //template <>
   //private ref class SpectralAnalyzerWrapper

#pragma endregion

}