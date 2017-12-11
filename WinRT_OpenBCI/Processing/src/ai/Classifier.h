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
#include <algorithm>
#include <memory>
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>
#include <type_traits>
#include <numeric>
#include <cmath>
#include <initializer_list>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

#define REQUIRES_FLOAT(T) typename = std::enable_if_t<std::is_floating_point_v<T>>
#define REQUIRES(expr) typename = std::enable_if_t<expr>

namespace Processing
{
#pragma region Decision tree

   // Attribute in a decision tree
   template <typename TVal, typename TId, typename TImp>
   class IAttribute
   {
   public:
      typedef TVal Val_t;
      typedef TId Id_t;
      typedef TImp Imp_t;
      typedef IAttribute<TVal, TId, TImp> My_t;

      virtual ~IAttribute() { }

      virtual Id_t GetId() const noexcept = 0;

      virtual size_t GetValuesCount() const noexcept = 0;

      virtual Val_t GetValueAt(size_t index) const = 0;

      virtual Imp_t GetImportance() const = 0;
   };

   template <typename TData, REQUIRES_FLOAT(TData)>
   using IContAttribute = IAttribute<std::pair<TData, TData>, TData, TData>;

   template <typename TData>
   class ContAttribute : public IContAttribute<TData>
   {
      typedef IContAttribute<TData> Base_t;
      typedef typename Base_t::Val_t Val_t;
      typedef typename Base_t::Id_t Id_t;
      typedef typename Base_t::Imp_t Imp_t;

      Id_t m_id;
      std::shared_ptr<std::vector<TData>> m_pSplit;
      std::function<Imp_t(Id_t)> m_GetImp; // returns importance for the given attribute

   public:
      template <typename F>
      ContAttribute(Id_t id, std::shared_ptr<std::vector<TData>> psplit, F&& GetImp)
         : m_id(id), m_pSplit(std::move(psplit)), m_GetImp(std::forward<F>(GetImp))
      { }
      virtual Id_t GetId() const noexcept
      {
         return m_id;
      }
      virtual size_t GetValuesCount() const noexcept
      {
         return m_pSplit->size() + 1;
      }
      virtual Val_t GetValueAt(size_t index) const
      {
         return std::make_pair(m_pSplit->operator[](index), m_pSplit->operator[](index + 1));
      }
      size_t GetIndexOf(TData val)
      {
         auto it = std::upper_bound(m_pSplit->cbegin(), m_pSplit->cend(), val);
         return std::distance(m_pSplit->cbegin(), it);
      }
      virtual Imp_t GetImportance() const
      {
         return m_GetImp(m_id);
      }
   };

   template <typename TAttribute>
   class TreeNode;

   template <typename TVal, typename TId, typename TImp>
   class TreeNode<IAttribute<TVal, TId, TImp>>
   {
      typedef IAttribute<TVal, TId, TImp> Attr_t;
      typedef TreeNode<IAttribute<TVal, TId, TImp>> My_t;

      std::shared_ptr<Attr_t> m_pAttr;

   public:
      TreeNode(std::shared_ptr<Attr_t> pattr) : m_pAttr(std::move(pAttr)), Children(pattr->GetValuesCount())
      { }
      std::vector<std::unique_ptr<My_t>> Children;
   };

#pragma endregion


#pragma region Neural networks

   template <typename TData> 
   struct SgnFunc
   {
      static_assert(std::is_floating_point_v<TData>);

      TData operator()(TData in) const
      {
         return in == 0.0 ? in : (in > 0 ? 1.0 : -1.0);
      }
   };
   template <typename TData> 
   struct SigmoidFunc
   {
      static_assert(std::is_floating_point_v<TData>);

      TData operator()(TData in) const
      {
         return 1.0 / (1.0 + std::exp(-in));
      }
   };

   template <typename TData, template <typename> typename TFunc>
   class Unit
   {
      static_assert(std::is_floating_point_v<TData>);

      TFunc<TData> m_Func;
      const size_t m_size;
      TData * const m_pweights;
      TData m_thrWeight;
      const bool m_alloc;

   public:
      Unit(TData *pweights, size_t size) : m_size(size+1), m_pweights(pweights), m_thrWeight(*pweights), m_alloc(false)
      { }

      //explicit Unit(size_t size) : m_size(size+1), m_pweights(new TData[size]), m_alloc(true)
      //{ }
      //Unit(size_t size, TData initVal) : Unit(size)
      //{
      //   std::fill(m_pweights, m_pweights + m_size, initVal);
      //}
      //Unit(const std::initializer_list<TData>& weights) : Unit(weights.size())
      //{
      //   std::copy(weights.begin(), weights.end(), m_pweights);
      //}
      //~Unit()
      //{
      //   if (m_alloc)
      //      delete[] m_pweights;
      //}

      size_t GetWeightsCount() const noexcept
      {
         return m_size;
      }
      void SetWeightAt(int index, TData val) const
      {
         if (index == 0)
            m_thrWeight = val;
         else
            m_pweights[index - 1] = val;
      }
      TData GetWeightAt(int index) const
      {
         return index == 0 ? m_thrWeight : m_pweight[index - 1];
      }
      TData GetOutput(TData *pinputs)
      {
         TData sum = std::inner_product(m_pweights, m_pweights + m_size, pinputs, m_thrWeight);
         return m_Func(sum);
      }
   };

   template <typename TData>
   using SgnUnit = Unit<TData, SgnFunc>;

   template <typename TData>
   using SigmoidUnit = Unit<TData, SigmoidFunc>;
   


   template <typename TData, REQUIRES_FLOAT(TData)>
   class INetwork
   {
   public:
      virtual ~INetwork() { }      
      virtual size_t GetLayersCount() const noexcept = 0;
      virtual size_t GetLayerSize(size_t layer) const noexcept = 0;
      virtual void ComputeOutputs(TData *pinputs, OUT TData *poutputs) const = 0;
   };

   //template <typename TData, typename TUnit>
   //class Network;
   //
   //template <typename TData, template<typename> typename TFunc>
   //class Network<TData, Unit<TData, TFunc>>
   //{
   //   typedef Unit<TData, TFunc> Node;
   //   std::vector<std::vector<Node>> m_layers;
   //public:
   //   size_t GetLayersCount() const noexcept
   //   {
   //      return m_layers.size();
   //   }
   //   size_t GetLayerSize(size_t layer) const noexcept
   //   {
   //      m_layers.push_back
   //      return m_layers[layer].size();
   //   }
   //};

   template <typename TData>
   class FixedSizeNetwork : public INetwork<TData>
   {
   public:
      typedef FixedSizeNetwork<TData> My_t;
      typedef INetwork<TData> Base_t;
      typedef SigmoidUnit<TData> Node_t;
      typedef unsigned char byte;

   private:
      byte * const m_pmemblock;
      byte * const m_pnodes;
      byte * const m_pweights;
      byte * const m_pouts;
      const size_t IN_N, OUT_N, N, L;

      FixedSizeNetwork(size_t nodeCount, size_t weightsCount, size_t inN, size_t N, size_t outN, size_t L) 
         : m_pmemblock(new byte[(sizeof(Node_t) + sizeof(TData)) * nodeCount + sizeof(TData) * weightsCount]),
         m_pnodes(m_pmemblock), 
         m_pweights(m_pnodes + sizeof(Node_t) * nodeCount),
         m_pouts(m_pweights + sizeof(TData) * weightsCount),
         IN_N(inN), OUT_N(outN), N(N), L(L)
      { }

   public:
      // inN doesn't account for threshold unit
      FixedSizeNetwork(size_t inN, size_t N, size_t outN, size_t L, TData initWeight = 1.0) 
         : FixedSizeNetwork((L - 1) * N + outN, 
                            N*inN + ((L - 2) * N + outN) * N, 
                            inN, N, outN, L)
      {
         byte *pn = m_pnodes;
         byte *pw = m_pweights;
         
         for (int layer = 0; layer < L; ++layer) {
            for (int node = 0; node < GetLayerSize(layer); ++node) {
               // allocate weights array
               size_t wlen = layer == 0 ? IN_N : N;
               TData *pweights = new(pw) TData[wlen];
               std::fill(pweights, pweights + wlen, initWeight);
               pw += sizeof(TData) * wlen;
               
               // allocate node
               Node_t *pnode = new(pn) Node_t(pweights, wlen);
               pn += sizeof(Node_t);
            }
         }
      }
      FixedSizeNetwork(const My_t&) = delete;
      My_t& operator =(const My_t&) = delete;
      virtual ~FixedSizeNetwork()
      {
         delete[] m_pmemblock;
      }

      virtual size_t GetLayersCount() const noexcept
      {
         return L;
      }
      virtual size_t GetLayerSize(size_t layer) const noexcept
      {
         return layer == L-1 ? OUT_N : N;
      }
      virtual void ComputeOutputs(TData *pinputs, OUT TData *poutputs) const
      {
         Node_t *pn = reinterpret_cast<Node_t *>(m_pnodes);
         TData *pres = reinterpret_cast<TData *>(m_pouts);

         // layer 0
         for (int node = 0; node < N; ++node) {
            pres[node] = pn->GetOutput(pinputs);
            pn++;
         }
         pres += N;

         // layer 1 through L-2 inclusively
         for (int layer = 1; layer < L - 1; ++layer) {
            TData *pinput = pres - N;
            for (int node = 0; node < N; ++node) {
               pres[node] = pn->GetOutput(pinput);
               pn++;
            }
            pres += N;
         }

         // layer L-1
         TData *pinput = pres - N;
         for (int node = 0; node < OUT_N; ++node) {            
            pres[node] = pn->GetOutput(pinput);
            pn++;
         }

         std::copy(pres, pres + OUT_N, poutputs);
      }
      Node_t *GetNodeAt(size_t layer, size_t index) const
      {
         Node_t *pn = reinterpret_cast<Node_t *>(m_pnodes);
         return &pn[layer * N + index];
      }
      TData *GetOutputAt(size_t layer, size_t index) const
      {
         TData *pres = reinterpret_cast<TData *>(m_pouts);
         return &pres[layer * N + index];
      }
   };

#pragma endregion

}