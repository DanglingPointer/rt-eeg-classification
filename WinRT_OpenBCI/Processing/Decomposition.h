#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <random>
#include <ppl.h>
#include "IImfDecomposition.h"

namespace Processing
{
#pragma region Interpolation

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class LinearSpline
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

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class CubicSpline
   {
      typedef std::unique_ptr<TData[]> UPtr;

   public:
      CubicSpline() = delete;

      static UPtr Compute(const std::vector<TData>& x, const std::vector<TData>& y, const UPtr& xs, int splineLength)
      {
         UPtr ys = std::make_unique<TData[]>(splineLength);

         TData a = (y[0] - y[1]) / (x[0] - x[1]);
         TData b = y[0] - a * x[0];

         for (int i = 0; i < splineLength; ++i) {
            ys[i] = a * xs[i] + b;
         }
         return std::move(ys);
      }
   };

#pragma endregion


#pragma region Standard C++ classes

   class MonotonicFunctionException : public std::exception
   { };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class EnvelopeFinder
   {
      typedef std::unique_ptr<TData[]> UPtr;

      UPtr m_upperEnvelope;
      UPtr m_lowerEnvelope;

      int32 m_zeroCrossingCount, m_upperExtremaCount, m_lowerExtremaCount;
      const int32 m_length;

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

         for (int i = 0; i < m_length; ++i) {
            if (yValues[i] > yValues[i - 1] && yValues[i] > yValues[i + 1]) {
               maxY.push_back(yValues[i]);
               maxX.push_back(xValues[i]);
            }
            else if (yValues[i] < yValues[i - 1] && yValues[i] < yValues[i + 1]) {
               minY.push_back(yValues[i]);
               minX.push_back(xValues[i]);
            }
            if ((yValues[i] < 0 && yValues[i + 1] >= 0)
                || (yValues[i] > 0 && yValues[i + 1] <= 0)) {
               m_zeroCrossingCount++;
            }
         }
         maxY.push_back(yValues[m_length - 1]);
         maxX.push_back(xValues[m_length - 1]);
         m_upperExtremaCount = maxY.size();

         minY.push_back(yValues[m_length - 1]);
         minX.push_back(xValues[m_length - 1]);
         m_lowerExtremaCount = minY.size();

         if (yValues[0] == 0.0 || yValues[1] == 0.0)
            m_zeroCrossingCount++;

         if (maxY.size() <= 2 && minY.size() <= 2) {
            throw MonotonicFunctionException();
         }

         auto upperSplineTask = [&maxX, &maxY, &xValues, this]() {
            if (this->m_upperExtremaCount == 2) {
               this->m_upperEnvelope = LinearSpline<TData>::Compute(maxX, maxY, xValues, this->m_length);
            }
            else {
               this->m_upperEnvelope = CubicSpline<TData>::Compute(maxX, maxY, xValues, m_length);
            }
         };
         auto lowerSplineTask = [&minX, &minY, &xValues, this]() {
            if (this->m_lowerExtremaCount == 2) {
               this->m_lowerEnvelope = LinearSpline<TData>::Compute(minX, minY, xValues, this->m_length);
            }
            else {
               this->m_lowerEnvelope = CubicSpline<TData>::Compute(minX, minY, xValues, m_length);
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

      int32 GetZeroCrossingCount() const noexcept
      {
         return m_zeroCrossingCount;
      }
      int32 GetUpperExtremaCount() const noexcept
      {
         return m_upperExtremaCount;
      }
      int32 GetLowerExtremaCount() const noexcept
      {
         return m_lowerExtremaCount;
      }
   };

   class ImfUnavailableException : public std::exception
   { };

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
         try {
            SiftResursively(xValues);
            for (int i = 0; i < m_length; ++i) {
               m_pprevH[i] = yValues[i] - m_pnewH[i];
            }
         }
         catch (const MonotonicFunctionException&) {
            m_isImfValid = false;
         }
      }

      UPtr MoveImf()
      {
         if (!m_isImfValid)
            throw ImfUnavailableException();
         return std::move(m_pnewH);
      }
      UPtr MoveResidue() noexcept
      {
         return std::move(m_pprevH);
      }

   private:
      void SiftResursively(const UPtr& xValues)
      {
         EnvelopeFinder<TData> envelopes(xValues, m_pprevH, m_length);
         for (int i = 0; i < m_length; ++i) {
            TData meanEnvelope = 0.5 * (envelopes.GetUpperEnvelopeAt(i) + envelopes.GetLowerEnvelopeAt(i));
            m_pnewH[i] = m_pprevH[i] - meanEnvelope;
         }

         if (!IsSiftingFinished()) {
            memcpy(m_pprevH.get(), m_pnewH.get(), sizeof(TData) * m_length);
            SiftResursively(xValues);
         }
      }
      bool IsSiftingFinished() const
      {
         TData sd = 0.0;
         for (int i = 0; i < m_length; ++i) {
            TData diff = m_pprevH[i] - m_pnewH[i];
            TData nextSD = (diff*diff) / (m_pprevH[i] * m_pprevH[i]);
            if (!std::isnan(nextSD)) {
               sd += nextSD;
            }
         }
         return sd < 0.3;
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
         maxImfCount = min(maxImfCount, std::log2(length));
         m_imfs.reserve(maxImfCount);

         try {
            do {
               Sifter<TData> s(xValues, m_pResidue, m_length);
               m_imfs.push_back(s.MoveImf());
               m_pResidue = s.MoveResidue();
            } while (--maxImfCount > 0);
         }
         catch (ImfUnavailableException&) { }
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

   private ref class DecomposerBase : public IImfDecomposition
   {
   protected private:
      IVector<IVector<double>^>^ m_pImfs;
      Array<double>^ m_pResidue;

      DecomposerBase(int dataLength)
         : m_pImfs(ref new Vector<IVector<double>^>()), m_pResidue(ref new Array<double>(dataLength))
      { }

   public:
      virtual property IVector<IVector<double>^>^ ImfFunctions {
         IVector<IVector<double>^>^ get()
         {
            return m_pImfs;
         }
      }
      virtual property Array<double>^ ResidueFunction {
         Array<double>^ get()
         {
            return m_pResidue;
         }
      }
   };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   private ref class EmdDecomposer : public DecomposerBase
   {
      typedef std::unique_ptr<TData[]> UPtr;

   internal:
      EmdDecomposer(const Array<double>^ xValues, const Array<double>^ yValues, int maxImfCount)
         : DecomposerBase(xValues->Length)
      {
         const int length = xValues->Length;

         UPtr pxValues = CopyArrayToUPtr(xValues, std::make_unique<TData[]>(length), length);
         UPtr pyValues = CopyArrayToUPtr(yValues, std::make_unique<TData[]>(length), length);

         InternalEmdDecomposer<TData> decomp(std::move(pxValues), std::move(pyValues), length, maxImfCount);

         CopyUPtrToArr(decomp.MoveResidue(), m_pResidue, length);

         for (int imfIndex = 0; imfIndex < decomp.GetImfCount(); ++imfIndex) {
            Vector<double>^ imf = ref new Vector<double>(length);

            UPtr pImf = decomp.MoveImfAt(imfIndex);
            std::copy(pImf.get(), pImf.get() + length, begin(imf));

            m_pImfs->Append(imf);
         }
      }
   };

   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   private ref class EemdDecomposer : public DecomposerBase
   {
      typedef std::unique_ptr<TData[]> UPtr;

   internal:
      EemdDecomposer(const Array<double>^ xValues, const Array<double>^ yValues, int ensembleCount, TData noiseSD)
         : DecomposerBase(xValues->Length)
      {
         const int length = xValues->Length;

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

         // Compute average in parallel

         for (int imfIndex = 0; imfIndex < maxImfCount; ++imfIndex) {
            Vector<double>^ resultingImf = ref new Vector<double>(length, 0.0);
            int actualEnsembleCount = 0;

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
            m_pImfs->Append(resultingImf);
         }
         m_pResidue = nullptr;
      }
   };

#pragma endregion

}