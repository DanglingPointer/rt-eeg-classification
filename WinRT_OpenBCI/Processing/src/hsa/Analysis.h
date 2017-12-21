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
#include <memory>
#include <cassert>
#include <complex>
#include <cmath>
#include <type_traits>
#include "ISpectralAnalysis.h"
#include "IHilbertSpectrum.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

#ifdef REQUIRES_FLOAT(T)
#undef REQUIRES_FLOAT(T)
#endif
#define REQUIRES_FLOAT(T) typename = std::enable_if_t<std::is_floating_point_v<T>>
#define REQUIRES_INT(T) typename = std::enable_if_t<std::is_integral_v<T>>

//#define RECURSIVE_FFT

namespace Processing
{
#pragma region Numerical Transforms

   template <typename TInt, REQUIRES_INT(TInt)>
   inline TInt BitReversal(TInt value, size_t bitsCount)
   {
      TInt res = 0;
      for (size_t i = 0; i < bitsCount; ++i) {
         if (value & (1 << (bitsCount-1 - i)) )
            res |= (1 << i);
      }
      return res;
   }

   template <typename TData, REQUIRES_FLOAT(TData)>
   class FastFourierTransform final
   {
      typedef TData Val;              // value type
      typedef std::complex<Val> Cval; // complex value type
      typedef std::unique_ptr<Cval[]> CvalArr;

   public:
      FastFourierTransform() = delete;

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
#ifdef RECURSIVE_FFT
         return RecursiveForward(std::move(input), *pResultLength);
#else
         return IterativeForward(std::move(input), *pResultLength);
#endif
      }
      // Method 4 from here: https://www.dsprelated.com/showarticle/800.php
      // Length must be a power of 2
      static CvalArr Inverse(CvalArr data, int length)
      {
         for (int i = 0; i < length; ++i) {
            data[i].imag(-1 * data[i].imag());
         }
#ifdef RECURSIVE_FFT
         data = RecursiveForward(std::move(data), length);
#else
         data = IterativeForward(std::move(data), length);
#endif
         for (int i = 0; i < length; ++i) {
            data[i].imag(-1 * data[i].imag());
            data[i] /= (Val)length;
         }
         return std::move(data);
      }

   private:
      // Parallelized algorithm from "Introduction To Algorithms", p.917
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

   template <typename TData, REQUIRES_FLOAT(TData)>
   class HilbertTransform final
   {
      typedef TData Val; // value type
      typedef std::unique_ptr<Val[]> ValArr;

      typedef std::complex<Val> Cval; // complex value type
      typedef std::unique_ptr<Cval[]> CvalArr;

   public:
      HilbertTransform() = delete;

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


#pragma region Instantaneous Analysis

   template <typename TData, REQUIRES_FLOAT(TData)>
   private ref class SpectralAnalyzerBase
   {
      typedef std::unique_ptr<TData[]> Uptr;
      typedef std::unique_ptr<std::complex<TData>[]> Cuptr;

   private protected:
      const int m_length;
      Array<TData>^ m_pInstAmpl;
      Array<TData>^ m_pInstPhas;
      Array<TData>^ m_pInstFreq;

   internal:
      SpectralAnalyzerBase(const Array<TData>^ yValues, TData timeStep) : m_length(yValues->Length),
         m_pInstAmpl(ref new Array<TData>(m_length)), m_pInstPhas(ref new Array<TData>(m_length)), m_pInstFreq(ref new Array<TData>(m_length - 1))
      {
         assert(yValues->Length > 0);
         Uptr pdata = std::make_unique<TData[]>(m_length);
         memcpy(pdata.get(), yValues->Data, sizeof(TData) * m_length);

         Cuptr hilberted = HilbertTransform<TData>::Forward(std::move(pdata), m_length);

         auto fillAmplTask = [&hilberted, this]() {
            for (int i = 0; i < m_length; ++i)
               this->m_pInstAmpl[i] = std::sqrt(hilberted[i].real() * hilberted[i].real() + hilberted[i].imag() * hilberted[i].imag());            
         };
         auto fillPhasFreqTask = [&hilberted, this, timeStep]() {
            TData invTimestep = 1.0 / timeStep;
            this->m_pInstPhas[0] = std::atan2(hilberted[0].imag(), hilberted[0].real());
            for (int i = 1; i < m_length; ++i) {
               this->m_pInstPhas[i] = std::atan2(hilberted[i].imag(), hilberted[i].real());
               this->m_pInstFreq[i - 1] = (m_pInstPhas[i - 1] - m_pInstPhas[i]) * invTimestep;
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
      TData GetAmplitudeAt(int i) const
      {
         assert(i < m_length);
         return m_pInstAmpl[i];
      }
      TData GetPhaseAt(int i) const
      {
         assert(i < m_length);
         return m_pInstPhas[i];
      }
      TData GetFrequencyAt(int i) const
      {
         assert(i < m_length - 1);
         return m_pInstFreq[i];
      }
      int GetLength() const noexcept
      {
         return m_length;
      }
   };

   template <typename TData>
   ref class SpectralAnalyzer;

   template <>
   private ref class SpectralAnalyzer<double> : public SpectralAnalyzerBase<double>, public Double::ISpectralAnalysis
   {
   internal:
      SpectralAnalyzer(const Array<double>^ yValues, double timeStep) 
         : SpectralAnalyzerBase(yValues, timeStep)
      { }
   public:
      // Inherited via ISpectralAnalysis
      virtual property Array<double>^ InstAmplitudes {
         Array<double>^ get()
         {
            return m_pInstAmpl;
         }
      }
      virtual property Array<double>^ InstPhases {
         Array<double>^ get()
         {
            return m_pInstPhas;
         }
      }
      virtual property Array<double>^ InstFrequencies {
         Array<double>^ get()
         {
            return m_pInstFreq;
         }
      }
   };

   template <>
   private ref class SpectralAnalyzer<float> : public SpectralAnalyzerBase<float>, public Single::ISpectralAnalysis
   {
   internal:
      SpectralAnalyzer(const Array<float>^ yValues, float timeStep)
         : SpectralAnalyzerBase(yValues, timeStep)
      { }
   public:
      // Inherited via ISpectralAnalysis
      virtual property Array<float>^ InstAmplitudes {
         Array<float>^ get()
         {
            return m_pInstAmpl;
         }
      }
      virtual property Array<float>^ InstPhases {
         Array<float>^ get()
         {
            return m_pInstPhas;
         }
      }
      virtual property Array<float>^ InstFrequencies {
         Array<float>^ get()
         {
            return m_pInstFreq;
         }
      }
   };

#pragma endregion


#pragma region Hilbert spectrum

   template <typename TData, REQUIRES_FLOAT(TData)>
   private ref class HilbertSpectrumBase
   {
      typedef SpectralAnalyzerBase<TData>^ AnalyzerPtr;

   private protected:
      std::vector<AnalyzerPtr> m_analyses;
      TData m_maxFreq, m_minFreq;
      TData m_timestep;

      HilbertSpectrumBase(IVector<IVector<TData>^>^ imfs, TData timestep) : m_analyses(imfs->Size), m_maxFreq(0.0), m_minFreq(0.0), m_timestep(timestep)
      {
         assert(imfs->Size > 0);
         concurrency::parallel_for((size_t)0, (size_t)(imfs->Size), [this, imfs](size_t i) {
            IVector<TData>^ imf = imfs->GetAt(i);
            Array<TData>^ pdata = ref new Array<TData>(imf->Size);
            std::copy(begin(imf), end(imf), pdata->begin());
            this->m_analyses[i] = ref new SpectralAnalyzerBase<TData>(pdata, this->m_timestep);
         });
         
         m_maxFreq = m_minFreq = (m_analyses[0])->GetFrequencyAt(0);
         for (const AnalyzerPtr& pAnalyzer : m_analyses) {
            for (int i = 0; i < pAnalyzer->GetLength() - 1; ++i) {
               TData freq = pAnalyzer->GetFrequencyAt(i);
               if (freq < m_minFreq)
                  m_minFreq = freq;
               else if (freq > m_maxFreq)
                  m_maxFreq = freq;
            }
         }
      }
      TData GetSpectrumAt(TData w, int t, TData maxError) const
      {
         TData res = 0.0;
         for (const AnalyzerPtr& pAnalysis : m_analyses) {
            TData error = pAnalysis->GetFrequencyAt(t) - w;
            if (error < maxError && error > -maxError) {
               res += pAnalysis->GetAmplitudeAt(t);
            }
         }
         return res;
      }
      TData GetMarginalAt(TData w, TData maxError) const
      {
         const int length = (m_analyses[0])->GetLength() - 1;
         TData res = 0.0;
         for (int i = 1; i < length; ++i) {
            // linear interpolation
            TData mean = (TData)0.5 * (GetSpectrumAt(w, i - 1, maxError) + GetSpectrumAt(w, i, maxError)) * m_timestep;
            res += mean * m_timestep;
         }
         return res;
      }
   };

   template <typename TData>
   ref class HilbertSpectrum;

   template<>
   private ref class HilbertSpectrum<double> : public HilbertSpectrumBase<double>, public Double::IHilbertSpectrum
   {
   internal:
      HilbertSpectrum(IVector<IVector<double>^>^ imfs, double timestep) : HilbertSpectrumBase(imfs, timestep)
      { }

   public:
      // Inherited via IHilbertSpectrum
      virtual property double MaxFrequency {
         double get()
         {
            return m_maxFreq;
         }
      }
      virtual property double MinFrequency {
         double get()
         {
            return m_minFreq;
         }
      }
      virtual double ComputeAt(double t, double w)
      {
         double error = (m_maxFreq - m_minFreq) / 1000.0;
         return GetSpectrumAt(w, (int)t, error);
      }
      virtual double ComputeMarginalAt(double w)
      {
         double error = (m_maxFreq - m_minFreq) / 1000.0;
         return GetMarginalAt(w, error);
      }
   };

   template<>
   private ref class HilbertSpectrum<float> : public HilbertSpectrumBase<float>, public Single::IHilbertSpectrum
   {
   internal:
      HilbertSpectrum(IVector<IVector<float>^>^ imfs, float timestep) : HilbertSpectrumBase(imfs, timestep)
      { }

   public:
      // Inherited via IHilbertSpectrum
      virtual property float MaxFrequency {
         float get()
         {
            return m_maxFreq;
         }
      }
      virtual property float MinFrequency {
         float get()
         {
            return m_minFreq;
         }
      }
      virtual float ComputeAt(float t, float w)
      {
         float error = (m_maxFreq - m_minFreq) / 1000.0f;
         return GetSpectrumAt(w, (int)t, error);
      }
      virtual float ComputeMarginalAt(float w)
      {
         float error = (m_maxFreq - m_minFreq) / 1000.0f;
         return GetMarginalAt(w, error);
      }
   };

#pragma endregion

}