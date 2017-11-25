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
#include <array>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <random>
#include <algorithm>
#include "IImfDecomposition.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

namespace Processing
{
#pragma region Interpolation

   /// <summary>
   /// Based on http://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm
   /// </summary>
   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class TriDiagonalMatrix
   {
      typedef std::unique_ptr<TData[]> ValArray;

      const std::unique_ptr<TData[]> m_pmemblock;

      TData *m_pCPrime;
      TData *m_pDPrime;

   public:
      const int N;
      TData * const pA;
      TData * const pB;
      TData * const pC;
      TData * const pD;

      explicit TriDiagonalMatrix(int n) : N(n), m_pmemblock(std::make_unique<TData[]>(n * 6)),
         m_pCPrime(m_pmemblock.get()), 
         m_pDPrime(m_pCPrime + n), 
         pA(m_pDPrime + n), 
         pB(pA + n), 
         pC(pB + n),
         pD(pC + n)
      { }
      ValArray Solve(ValArray result)
      {
         m_pCPrime[0] = pC[0] / pB[0];
         m_pDPrime[0] = pD[0] / pB[0];
         for (int i = 1; i < N; ++i) {
            m_pCPrime[i] = pC[i] / (pB[i] - m_pCPrime[i - 1] * pA[i]);
            m_pDPrime[i] = (pD[i] - m_pDPrime[i - 1] * pA[i]) / (pB[i] - m_pCPrime[i - 1] * pA[i]);
         }

         result[N - 1] = m_pDPrime[N - 1];
         for (int i = N - 2; i >= 0; --i) {
            result[i] = m_pDPrime[i] - m_pCPrime[i] * result[i + 1];
         }
         return std::move(result);
      }
      ValArray Solve()
      {
         return Solve(std::make_unique<TData[]>(N));
      }
   };

   /// <summary>
   /// Based on https://en.wikiversity.org/wiki/Cubic_Spline_Interpolation, but minimizing number of divisions
   /// </summary>
   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class CubicSpline final
   {
      typedef std::unique_ptr<TData[]> ValArray;

      std::vector<TData> m_x;
      std::vector<TData> m_y;

      const int m_length;

   public:
      static ValArray Compute(std::vector<TData> x, std::vector<TData> y, const ValArray& xs, int splineLength)
      {
         CubicSpline<TData> cs(std::move(x), std::move(y));
         auto m = cs.Fit();
         return cs.Evaluate(xs, splineLength, std::move(m));
      }

   private:
      CubicSpline(std::vector<TData> x, std::vector<TData> y) noexcept 
         : m_x(std::move(x)), m_y(std::move(y)), m_length(x.size())
      { }
      // returns M
      ValArray Fit() const
      {
         TriDiagonalMatrix<TData> tdm(m_length);

         // mu
         for (int i = 1; i < m_length - 1; ++i) {
            tdm.pA[i] = (m_x[i] - m_x[i - 1]) / (m_x[i + 1] - m_x[i - 1]);
         }
         tdm.pA[m_length - 1] = 0.0;

         // lambda
         for (int i = 1; i < m_length - 1; ++i) {
            tdm.pC[i] = 1.0 - tdm.pA[i];
         }
         tdm.pC[0] = 0.0;

         // diagonal
         std::fill(tdm.pB, tdm.pB + m_length, 2.0);

         // d
         for (int i = 1; i < m_length - 1; ++i) {
            TData dx0 = m_x[i] - m_x[i - 1];
            TData dx1 = m_x[i + 1] - m_x[i];
            TData dx10 = m_x[i + 1] - m_x[i - 1];
            TData dy0 = m_y[i] - m_y[i - 1];
            TData dy1 = m_y[i + 1] - m_y[i];

            tdm.pD[i] = 6.0 * ((dy1*dx0 - dy0*dx1) / (dx10*dx1*dx0));
         }
         tdm.pD[0] = 0.0;
         tdm.pD[m_length - 1] = 0.0;

         return tdm.Solve();
      }
      ValArray Evaluate(const ValArray& xs, int length, ValArray m)
      {
         ValArray ys = std::make_unique<TData[]>(length);

         for (int ind = 0; ind < length; ++ind) {            
            TData x = xs[ind];

            // finding i for Ci
            auto it = std::lower_bound(m_x.cbegin(), m_x.cend(), x);
            int i = std::distance(m_x.cbegin(), it);
            if (i == m_x.size()) 
               i--;
            else if (i == 0) 
               i++;
            
            // y = Ci(x)
            TData dx1 = m_x[i] - x;
            TData dx0 = x - m_x[i - 1];
            TData h = m_x[i] - m_x[i - 1];
            TData num = dx1 * (dx1*dx1*m[i - 1] + 6 * m_y[i - 1] - m[i - 1] * h*h) + dx0 * (dx0*dx0*m[i] + 6 * m_y[i] - m[i] * h*h);
            ys[ind] = num / (6 * h);
         }
         return std::move(ys);
      }
   };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class LinearSpline final
   {
      typedef std::unique_ptr<TData[]> UPtr;

   public:
      LinearSpline() = delete;

      static UPtr Compute(const std::vector<TData>& x, const std::vector<TData>& y, const UPtr& xs, int length)
      {
         std::unique_ptr<TData[]> ys = std::make_unique<TData[]>(length);

         TData a = (y[0] - y[1]) / (x[0] - x[1]);
         TData b = y[0] - a * x[0];

         for (int i = 0; i < length; ++i) {
            ys[i] = a * xs[i] + b;
         }
         return std::move(ys);
      }
   };

#pragma endregion


#pragma region Standard VC++ classes

   class MonotonicFunctionException : public std::exception
   { };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class EnvelopeFinder
   {
      typedef std::unique_ptr<TData[]> UPtr;

      UPtr m_upperEnvelope;
      UPtr m_lowerEnvelope;

      int m_zeroCrossingCount, m_upperExtremaCount, m_lowerExtremaCount;
      const int m_length;

   public:
      EnvelopeFinder(const UPtr& xValues, const UPtr& yValues, int length)
         : m_length(length), m_zeroCrossingCount(0), m_upperExtremaCount(0), m_lowerExtremaCount(0),
         m_upperEnvelope(nullptr), m_lowerEnvelope(nullptr)
      {
         std::vector<TData> maxX;
         std::vector<TData> maxY;
         std::vector<TData> minX;
         std::vector<TData> minY;

         maxY.push_back(yValues[0]);
         maxX.push_back(xValues[0]);
         minY.push_back(yValues[0]);
         minX.push_back(xValues[0]);

         for (int i = 1; i < m_length - 1; ++i) {
            if (yValues[i] > yValues[i - 1] && yValues[i] > yValues[i + 1]) {
               maxY.push_back(yValues[i]);
               maxX.push_back(xValues[i]);
            }
            else if (yValues[i] < yValues[i - 1] && yValues[i] < yValues[i + 1]) {
               minY.push_back(yValues[i]);
               minX.push_back(xValues[i]);
            }
         }
         for (int i = 0; i < m_length - 1; ++i) {
            if ((yValues[i] < 0 && yValues[i + 1] >= 0) || (yValues[i] > 0 && yValues[i + 1] <= 0)) 
               m_zeroCrossingCount++;
         }
         if (yValues[0] == 0.0 && yValues[1] != 0.0)
            m_zeroCrossingCount++;

         maxY.push_back(yValues[m_length - 1]);
         maxX.push_back(xValues[m_length - 1]);
         m_upperExtremaCount = maxY.size() - 2;

         minY.push_back(yValues[m_length - 1]);
         minX.push_back(xValues[m_length - 1]);
         m_lowerExtremaCount = minY.size() - 2;


         if (maxY.size() == 2 && minY.size() == 2) {
            throw MonotonicFunctionException();
         }

         auto upperSplineTask = [&maxX, &maxY, &xValues, this]() {
            if (this->m_upperExtremaCount == 2) {
               this->m_upperEnvelope = LinearSpline<TData>::Compute(maxX, maxY, xValues, this->m_length);
            }
            else {
               this->m_upperEnvelope = CubicSpline<TData>::Compute(std::move(maxX), std::move(maxY), xValues, this->m_length);
            }
         };
         auto lowerSplineTask = [&minX, &minY, &xValues, this]() {
            if (this->m_lowerExtremaCount == 2) {
               this->m_lowerEnvelope = LinearSpline<TData>::Compute(minX, minY, xValues, this->m_length);
            }
            else {
               this->m_lowerEnvelope = CubicSpline<TData>::Compute(std::move(minX), std::move(minY), xValues, this->m_length);
            }
         };
         concurrency::parallel_invoke(upperSplineTask, lowerSplineTask);
      }

      TData GetUpperEnvelopeAt(int index) const
      {
         return m_upperEnvelope[index];
      }
      TData GetLowerEnvelopeAt(int index) const
      {
         return m_lowerEnvelope[index];
      }

      int GetZeroCrossingCount() const noexcept
      {
         return m_zeroCrossingCount;
      }
      int GetUpperExtremaCount() const noexcept
      {
         return m_upperExtremaCount;
      }
      int GetLowerExtremaCount() const noexcept
      {
         return m_lowerExtremaCount;
      }
   };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class Sifter
   {
      typedef std::unique_ptr<TData[]> UPtr;

      const int m_length;
      UPtr m_pprevH, m_pnewH;
      bool m_isImfValid;

   public:
      Sifter(const UPtr& xValues, const UPtr& yValues, int length) : m_isImfValid(true), m_length(length),
         m_pprevH(std::make_unique<TData[]>(length)), m_pnewH(std::make_unique<TData[]>(length))
      {
         memcpy(m_pprevH.get(), yValues.get(), sizeof(TData)*length);
         try {
            bool finished = false;
            while (!finished) {
               EnvelopeFinder<TData> envs(xValues, m_pprevH, m_length);
               for (int i = 0; i < m_length; ++i) {
                  TData meanEnvelope = 0.5 * (envs.GetUpperEnvelopeAt(i) + envs.GetLowerEnvelopeAt(i));
                  m_pnewH[i] = m_pprevH[i] - meanEnvelope;
               }

               int extremaCount = envs.GetUpperExtremaCount() + envs.GetLowerExtremaCount();
               int diff = extremaCount - envs.GetZeroCrossingCount();
               if (!(finished = IsSiftingFinished(diff))) {
                  memcpy(m_pprevH.get(), m_pnewH.get(), sizeof(TData) * m_length);
               }
            }
         }
         catch (const MonotonicFunctionException&) {
            m_isImfValid = false;
            m_pnewH = nullptr;
            m_pprevH = nullptr;
         }
      }
      bool IsImfExtracted() const noexcept
      {
         return m_isImfValid;
      }
      TData GetImfAt(int index) const
      {
         return m_pnewH[index];
      }
      UPtr MoveImf() noexcept
      {
         return std::move(m_pnewH);
      }

   private:
      bool IsSiftingFinished(int extremaCountDiff) const
      {
         //TData numerator = 0.0, denominator = 0.0;
         //for (int i = 0; i < m_length; ++i) {
         //   numerator += (m_pprevH[i] - m_pnewH[i]) * (m_pprevH[i] - m_pnewH[i]);
         //   denominator += m_pprevH[i] * m_pprevH[i];
         //}
         //return (numerator / denominator) < 0.1;
         return extremaCountDiff > -3 && extremaCountDiff < 3;
      }
   };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class InternalEmdDecomposer
   {
      typedef std::unique_ptr<TData[]> UPtr;

      UPtr m_pResidue;
      std::vector<UPtr> m_imfs;
      const int m_length;

   public:
      InternalEmdDecomposer(UPtr xValues, UPtr yValues, int length, int maxImfCount)
         : m_length(length), m_pResidue(std::move(yValues))
      {
         maxImfCount = min(maxImfCount, std::log2(length) + 1);
         m_imfs.reserve(maxImfCount);

         do {
            Sifter<TData> s(xValues, m_pResidue, m_length);
            if (!s.IsImfExtracted())
               return;
            for (int i = 0; i < m_length; ++i) {
               m_pResidue[i] = m_pResidue[i] - s.GetImfAt(i);
            }
            m_imfs.push_back(s.MoveImf());
         } while (--maxImfCount > 0);
      }

      int GetImfCount() const noexcept
      {
         return m_imfs.size();
      }
      UPtr MoveImfAt(int imfIndex)
      {
         return std::move(m_imfs[imfIndex]);
      }
      UPtr MoveResidue() noexcept
      {
         return std::move(m_pResidue);
      }
   };

#pragma endregion


#pragma region Helper functions for copying

   template<typename T>
   inline std::unique_ptr<T[]> CopyArrayToUPtr(const Array<T>^ src, std::unique_ptr<T[]> dest, int len)
   {
      memcpy(dest.get(), src->Data, sizeof(T) * len);
      return std::move(dest);
   }
   template<typename T1, typename T2>
   inline std::unique_ptr<T1[]> CopyArrayToUPtr(const Array<T2>^ src, std::unique_ptr<T1[]> dest, int len)
   {
      std::copy(src->Data, src->Data + len, dest.get());
      return std::move(dest);
   }
   template<typename T>
   inline std::unique_ptr<T[]> CopyUPtrToArr(std::unique_ptr<T[]> src, Array<T>^ dest, int len)
   {
      memcpy(dest->Data, src.get(), len * sizeof(T));
      return std::move(src);
   }
   template<typename T1, typename T2>
   inline std::unique_ptr<T1[]> CopyUPtrToArr(std::unique_ptr<T1[]> src, const Array<T2>^ dest, int len)
   {
      std::copy(src.get(), src.get() + len, dest->Data);
      return std::move(src);
   }

#pragma endregion


#pragma region C++/CX ref classes

   private ref class DecomposerBase : public IImfDecompositionDouble, public IImfDecompositionSingle
   {
      IVector<IVector<double>^>^ m_pImfsD;
      Array<double>^ m_pResidueD;

      IVector<IVector<float>^>^ m_pImfsS;
      Array<float>^ m_pResidueS;

   protected private:
      template <typename TData>
      IVector<IVector<TData>^>^& GetImfs();

      template <typename TData>
      Array<TData>^& GetResidue();

      DecomposerBase()
         : m_pImfsD(nullptr), m_pResidueD(nullptr), m_pImfsS(nullptr), m_pResidueS(nullptr)
      { }

   public:
      virtual property IVector<IVector<double>^>^ ImfFunctionsD {
         IVector<IVector<double>^>^ get() = IImfDecompositionDouble::ImfFunctions::get
         {
            return m_pImfsD;
         }
      }
      virtual property Array<double>^ ResidueFunctionD {
         Array<double>^ get() = IImfDecompositionDouble::ResidueFunction::get
         {
            return m_pResidueD;
         }
      }
      virtual property IVector<IVector<float>^>^ ImfFunctionsS {
         IVector<IVector<float>^>^ get() = IImfDecompositionSingle::ImfFunctions::get
         {
            return m_pImfsS;
         }
      }
      virtual property Array<float>^ ResidueFunctionS {
         Array<float>^ get() = IImfDecompositionSingle::ResidueFunction::get
         {
            return m_pResidueS;
         }
      }
   };
   template<> 
   inline IVector<IVector<double>^>^& DecomposerBase::GetImfs() { return m_pImfsD; }
   template<> 
   inline IVector<IVector<float>^>^& DecomposerBase::GetImfs() { return m_pImfsS; }
   template<> 
   inline Array<double>^& DecomposerBase::GetResidue() { return m_pResidueD; }
   template<> 
   inline Array<float>^& DecomposerBase::GetResidue() { return m_pResidueS; }

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   private ref class EmdDecomposer : public DecomposerBase
   {
      typedef std::unique_ptr<TData[]> UPtr;

   internal:
      EmdDecomposer(const Array<TData>^ xValues, const Array<TData>^ yValues, int maxImfCount)
         : DecomposerBase()
      {
         const int length = xValues->Length;
         GetResidue<TData>() = ref new Array<TData>(length);
         GetImfs<TData>() = ref new Vector<IVector<TData>^>();


         UPtr pxValues = CopyArrayToUPtr(xValues, std::make_unique<TData[]>(length), length);
         UPtr pyValues = CopyArrayToUPtr(yValues, std::make_unique<TData[]>(length), length);

         InternalEmdDecomposer<TData> decomp(std::move(pxValues), std::move(pyValues), length, maxImfCount);

         CopyUPtrToArr(decomp.MoveResidue(), GetResidue<TData>(), length);

         for (int imfIndex = 0; imfIndex < decomp.GetImfCount(); ++imfIndex) {
            UPtr pImf = decomp.MoveImfAt(imfIndex);

            Vector<TData>^ imf = ref new Vector<TData>(length);
            std::copy(pImf.get(), pImf.get() + length, begin(imf));

            GetImfs<TData>()->Append(imf);
         }
      }
   };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   private ref class EemdDecomposer : public DecomposerBase
   {
      typedef std::unique_ptr<TData[]> UPtr;

   internal:
      EemdDecomposer(const Array<TData>^ xValues, const Array<TData>^ yValues, int ensembleCount, TData noiseSD)
         : DecomposerBase()
      {
         const int length = xValues->Length;
         GetImfs<TData>() = ref new Vector<IVector<TData>^>();

         std::vector<UPtr> yValuesEnsembles(ensembleCount);

         // Add white noise

         unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
         std::default_random_engine generator(seed);
         std::normal_distribution<TData> distribution(0.0, noiseSD);

         for (int ensembleInd = 0; ensembleInd < ensembleCount; ++ensembleInd) {
            // copy given y-values
            UPtr pEnsemble = CopyArrayToUPtr(yValues, std::make_unique<TData[]>(length), length);
            // add gaussian noise
            for (int dataInd = 0; dataInd < length; ++dataInd) {
               pEnsemble[dataInd] += distribution(generator);
            }
            yValuesEnsembles[ensembleInd] = std::move(pEnsemble);
         }

         // Decompose the ensembles in parallel
         typedef UPtr ValArray;

         std::vector<std::vector<ValArray>> imfEnsembles(ensembleCount);

         concurrency::parallel_for(0, ensembleCount, [&yValuesEnsembles, &imfEnsembles, xValues, length](int i) {

            UPtr pxValues = CopyArrayToUPtr(xValues, std::make_unique<TData[]>(length), length);
            InternalEmdDecomposer<TData> decomposer(std::move(pxValues), std::move(yValuesEnsembles[i]), length, INT_MAX);

            std::vector<ValArray> tempImfsVec(decomposer.GetImfCount());
            for (int imfIndex = 0; imfIndex < decomposer.GetImfCount(); ++imfIndex) {
               tempImfsVec[imfIndex] = decomposer.MoveImfAt(imfIndex);
            }
            imfEnsembles[i] = std::move(tempImfsVec);
         });

         int maxImfCount = 0;
         for (const std::vector<ValArray>& imfFunctions : imfEnsembles) {
            if (imfFunctions.size() > maxImfCount)
               maxImfCount = imfFunctions.size();
         }

         // Compute average

         for (int imfIndex = 0; imfIndex < maxImfCount; ++imfIndex) {
            Vector<TData>^ resultingImf = ref new Vector<TData>(length, 0.0);
            int actualEnsembleCount = 0; // not all ensembles might have the same number of IMFs

            for (const std::vector<ValArray>& imfFunctions : imfEnsembles) {
               if (imfIndex < imfFunctions.size()) {
                  actualEnsembleCount++;
                  for (int xIndex = 0; xIndex < length; ++xIndex) {
                     double prevValue = resultingImf->GetAt(xIndex);
                     resultingImf->SetAt(xIndex, prevValue + imfFunctions[imfIndex][xIndex]);
                  }
               }
            }
            for (int i = 0; i < length; ++i) {
               double value = resultingImf->GetAt(i);
               resultingImf->SetAt(i, value / actualEnsembleCount);
            }
            GetImfs<TData>()->Append(resultingImf);
         }
      }
   };

#pragma endregion

}