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

   // Based on http://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm
   // Ported from C# implementation by Ryan Seghers
   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   struct TriDiagonalMatrix
   {
      typedef std::unique_ptr<TData[]> ValArray;

      ValArray A;
      ValArray B;
      ValArray C;

      const int N;

      TriDiagonalMatrix(int n) : N(n), A(std::make_unique<TData[]>(n)), B(std::make_unique<TData[]>(n)), C(std::make_unique<TData[]>(n))
      {
         std::fill(A.get(), A.get() + n, 0.0);
         std::fill(B.get(), B.get() + n, 0.0);
         std::fill(C.get(), C.get() + n, 0.0);
      }
      ValArray Solve(ValArray d)
      {
         ValArray cPrime = std::make_unique<TData[]>(N);
         cPrime[0] = C[0] / B[0];
         for (int i = 0; i < N; ++i) {
            cPrime[i] = C[i] / (B[i] - cPrime[i - 1] * A[i]);
         }

         ValArray dPrime = std::make_unique<TData[]>(N);
         dPrime[0] = d[0] / B[0];
         for (int i = 1; i < N; ++i) {
            dPrime[i] = (d[i] - dPrime[i - 1] * A[i]) / (B[i] - cPrime[i - 1] * A[i]);
         }

         ValArray x = std::make_unique<TData[]>(N);
         x[N - 1] = dPrime[N - 1];
         for (int i = N - 2; i >= 0; --i) {
            x[i] = dPrime[i] - cPrime[i] * x[i + 1];
         }

         return std::move(x);
      }
   };

   // Based on http://en.wikipedia.org/wiki/Spline_interpolation
   // Ported from C# implementation by Ryan Seghers
   template <typename TData, typename = std::enable_if_t<std::is_floating_point_v<TData>>>
   class CubicSpline
   {
      typedef std::unique_ptr<TData[]> ValArray;

      ValArray m_a, m_b;
      ValArray m_xOrig, m_yOrig;

      int m_origLength;
      int m_lastIndex;

   public:
      static ValArray Compute(const std::vector<TData>& x, const std::vector<TData>& y, const ValArray& xs, int splineLength)
      {
         int dataLength = x.size();

         ValArray px = std::make_unique<TData[]>(dataLength);
         std::copy(x.begin(), x.end(), px.get());

         ValArray py = std::make_unique<TData[]>(dataLength);
         std::copy(y.begin(), y.end(), py.get());

         CubicSpline<TData> spline(dataLength);
         spline.Fit(std::move(px), std::move(py));
         return spline.Eval(xs, splineLength);
      }

   private:
      explicit CubicSpline(int origLength) : m_lastIndex(0), m_origLength(origLength)
      { }
      int GetNextXIndex(TData x)
      {
         while (m_lastIndex < m_origLength - 2 && x > m_xOrig[m_lastIndex + 1]) {
            m_lastIndex++;
         }
         return m_lastIndex;
      }
      TData EvalSpline(TData x, int j)
      {
         TData dx = m_xOrig[j + 1] - m_xOrig[j];
         TData t = (x - m_xOrig[j]) / dx;
         TData y = (1 - t) * m_yOrig[j] + t * m_yOrig[j + 1] + t * (1 - t) * (m_a[j] * (1 - t) + m_b[j] * t); 
         return y;
      }
      void Fit(ValArray x, ValArray y)
      {
         ValArray r = std::make_unique<TData[]>(m_origLength); // the right hand side numbers: wikipedia page overloads b

         TriDiagonalMatrix<TData> tdm(m_origLength);
         TData dx1, dx2, dy1, dy2;

         // First row is different (equation 16 from the article)
         dx1 = x[1] - x[0];
         tdm.C[0] = 1.0 / dx1;
         tdm.B[0] = 2.0 * tdm.C[0];
         r[0] = 3 * (y[1] - y[0]) / (dx1 * dx1);

         // Body rows (equation 15 from the article)
         for (int i = 1; i < m_origLength - 1; ++i) {
            dx1 = x[i] - x[i - 1];
            dx2 = x[i + 1] - x[i];

            tdm.A[i] = 1.0 / dx1;
            tdm.C[i] = 1.0 / dx2;
            tdm.B[i] = 2.0 * (tdm.A[i] + tdm.C[i]);

            dy1 = y[i] - y[i - 1];
            dy2 = y[i + 1] - y[i];
            r[i] = 3 * (dy1 / (dx1 * dx1) + dy2 / (dx2 * dx2));
         }

         // Last row also different (equation 17 from the article)
         dx1 = x[m_origLength - 1] - x[m_origLength - 2];
         dy1 = y[m_origLength - 1] - y[m_origLength - 2];
         tdm.A[m_origLength - 1] = 1.0 / dx1;
         tdm.B[m_origLength - 1] = 2.0 * tdm.A[m_origLength - 1];
         r[m_origLength - 1] = 3 * (dy1 / (dx1 * dx1));

         // k is the solution to the matrix
         ValArray k = tdm.Solve(std::move(r));
         
         // a and b are each spline's coefficients
         m_a = std::make_unique<TData[]>(m_origLength - 1);
         m_b = std::make_unique<TData[]>(m_origLength - 1);

         for (int i = 1; i < m_origLength; i++) {
            dx1 = x[i] - x[i - 1];
            dy1 = y[i] - y[i - 1];
            m_a[i - 1] = k[i - 1] * dx1 - dy1;  // equation 10 from the article
            m_b[i - 1] = -k[i] * dx1 + dy1;     // equation 11 from the article
         }

         m_xOrig = std::move(x);
         m_yOrig = std::move(y);
      }
      ValArray Eval(const ValArray& x, int n)
      {
         ValArray y = std::make_unique<TData[]>(n);

         for (int i = 0; i < n; i++) {
            // Find which spline can be used to compute this x (by simultaneous traverse)
            int j = GetNextXIndex(x[i]);

            // Evaluate using j'th spline
            y[i] = EvalSpline(x[i], j);
         }
         return std::move(y);
      }
   };

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
               this->m_upperEnvelope = CubicSpline<TData>::Compute(maxX, maxY, xValues, this->m_length);
            }
         };
         auto lowerSplineTask = [&minX, &minY, &xValues, this]() {
            if (this->m_lowerExtremaCount == 2) {
               this->m_lowerEnvelope = LinearSpline<TData>::Compute(minX, minY, xValues, this->m_length);
            }
            else {
               this->m_lowerEnvelope = CubicSpline<TData>::Compute(minX, minY, xValues, this->m_length);
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
         memcpy(m_pprevH.get(), yValues.get(), sizeof(TData)*length);
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
         maxImfCount = min(maxImfCount, std::log2(length) + 1);
         m_imfs.reserve(maxImfCount);

         try {
            do {
               Sifter<TData> s(xValues, m_pResidue, m_length);
               m_imfs.push_back(s.MoveImf()); // might throw
               m_pResidue = s.MoveResidue();
            } while (--maxImfCount > 0);
         }
         catch (const ImfUnavailableException&) { }
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
            UPtr pImf = decomp.MoveImfAt(imfIndex);

            Vector<double>^ imf = ref new Vector<double>(length);
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

         // Compute average

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