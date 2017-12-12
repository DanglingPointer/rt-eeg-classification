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
#include <type_traits>
#include <numeric>
#include <cmath>
#include <initializer_list>
#include <stdexcept>
#include <random>
#include <cassert>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

#define REQUIRES_FLOAT(T) typename = std::enable_if_t<std::is_floating_point_v<T>>
#define REQUIRES(expr) typename = std::enable_if_t<expr>

#ifndef OUT
 #define OUT
#endif

namespace Processing
{
#pragma region Decision tree

#if 0
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

#endif

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
      Unit(TData *pweights, size_t size) : m_size(size + 1), m_pweights(pweights), m_thrWeight(*pweights), m_alloc(false)
      { }
      explicit Unit(size_t size) : m_size(size + 1), m_pweights(new TData[size]), m_alloc(true)
      { }
      Unit(size_t size, TData initVal) : Unit(size)
      {
         std::fill(m_pweights, m_pweights + m_size, initVal);
         m_thrWeight = initVal;
      }
      Unit(std::initializer_list<TData> weights) : Unit(weights.size())
      {
         std::copy(weights.begin() + 1, weights.end(), m_pweights);
         m_thrWeight = *(weights.begin());
      }
      ~Unit()
      {
         if (m_alloc)
            delete[] m_pweights;
      }
      size_t GetWeightsCount() const noexcept
      {
         return m_size;
      }
      void SetWeightAt(int index, TData val)
      {
         assert(index < m_size);
         if (index == 0)
            m_thrWeight = val;
         else
            m_pweights[index - 1] = val;
      }
      TData GetWeightAt(int index) const
      {
         assert(index < m_size);
         return index == 0 ? m_thrWeight : m_pweights[index - 1];
      }
      TData GetOutput(const TData *pinputs)
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
      virtual size_t GetNodesCount() const noexcept = 0;
      virtual size_t GetLayerCount() const noexcept = 0;
      virtual size_t GetLayerSize(size_t layer) const noexcept = 0;
      virtual void ComputeOutputs(const TData *pinputs, OUT TData *poutputs) const = 0;
   };

   template <typename TData>
   class FixedSizeNetwork : public INetwork<TData>
   {
   public:
      typedef TData val_t;
      typedef FixedSizeNetwork<TData> My_t;
      typedef INetwork<TData> Base_t;
      typedef SigmoidUnit<TData> Node_t;
      typedef unsigned char byte;

   private:
      std::unique_ptr<byte[]> m_pmemblock;
      byte * const m_pnodes;
      byte * const m_pweights;
      byte * const m_pouts;
      const size_t IN_N, OUT_N, N, L;

      FixedSizeNetwork(size_t nodeCount, size_t weightsCount, size_t inN, size_t N, size_t outN, size_t L) 
         : m_pmemblock(std::make_unique<byte[]>((sizeof(Node_t) + sizeof(TData)) * nodeCount + sizeof(TData) * weightsCount)),
         m_pnodes(m_pmemblock.get()), 
         m_pweights(m_pnodes + sizeof(Node_t) * nodeCount),
         m_pouts(m_pweights + sizeof(TData) * weightsCount),
         IN_N(inN), OUT_N(outN), N(N), L(L)
      { }

   public:
      // inN doesn't include threshold unit
      // InitWeightFactory takes fan-in (size_t) and returns one initial weight (TData)
      template <typename F>
      FixedSizeNetwork(size_t inN, size_t N, size_t outN, size_t L, F InitWeightFactory) 
         : FixedSizeNetwork((L - 1) * N + outN, 
                            N*inN + ((L - 2) * N + outN) * N, 
                            inN, N, outN, L)
      {
         byte *pn = m_pnodes;
         byte *pw = m_pweights;
         
         for (int layer = 0; layer < L; ++layer) {
            for (int node = 0; node < GetLayerSize(layer); ++node) {
               // allocate weights array
               size_t wlen = (layer == 0) ? IN_N : N;
               TData *pweights = new(pw) TData[wlen];
               for (int i = 0; i < wlen; ++i)
                  pweights[i] = InitWeightFactory(wlen + 1);
               pw += sizeof(TData) * wlen;
               
               // allocate node
               Node_t *pnode = new(pn) Node_t(pweights, wlen);
               pn += sizeof(Node_t);
            }
         }
      }
      // uses default init weights that require normalized input
      FixedSizeNetwork(size_t inN, size_t N, size_t outN, size_t L) : FixedSizeNetwork(inN, N, outN, L, DefaultWeightFactory)
      { }
      FixedSizeNetwork(const My_t&) = delete;
      My_t& operator =(const My_t&) = delete;
      virtual size_t GetNodesCount() const noexcept
      {
         return N * (L - 1) + OUT_N;
      }
      virtual size_t GetLayerCount() const noexcept
      {
         return L;
      }
      virtual size_t GetLayerSize(size_t layer) const noexcept
      {
         return layer == L-1 ? OUT_N : N;
      }
      virtual void ComputeOutputs(const TData *pinputs, OUT TData *poutputs) const
      {
         assert(pinputs != nullptr);

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

         if (poutputs)
            std::copy(pres, pres + OUT_N, poutputs);
      }
      Node_t *GetNodeAt(size_t layer, size_t index) const
      {
         assert(layer < L);
         assert((layer == L - 1 && index < OUT_N) || (layer < L - 1 && index < N));

         Node_t *pn = reinterpret_cast<Node_t *>(m_pnodes);
         return &pn[layer * N + index];
      }
      TData GetOutputAt(size_t layer, size_t index) const
      {
         assert(layer < L);
         assert((layer == L - 1 && index < OUT_N) || (layer < L - 1 && index < N));

         TData *pres = reinterpret_cast<TData *>(m_pouts);
         return pres[layer * N + index];
      }
   private:
      static val_t DefaultWeightFactory(size_t fanIn)
      {
         // https://infoscience.epfl.ch/record/82296/files/94-07.pdf, L.Bottou approach
         std::random_device rd;
         val_t bound = 2.38 / std::sqrt((val_t)fanIn);
         std::uniform_real_distribution<val_t> u(-bound, bound);
         return u(rd);
      }
   };

   template <typename TData>
   class TilingNetwork : public INetwork<TData>
   {
   public:
      typedef TData val_t;
      typedef TilingNetwork<TData> My_t;
      typedef INetwork<TData> Base_t;
      typedef SgnUnit<TData> Node_t;
      typedef unsigned char byte;

   private:
      std::vector<std::vector<Node_t>> m_layers;

   public:
      virtual size_t GetNodesCount() const noexcept
      {
         size_t sum = 0;
         for (const auto& layer : m_layers)
            sum += layer.size();
         return sum;
      }
      virtual size_t GetLayerCount() const noexcept
      {
         return m_layers.size();
      }
      virtual size_t GetLayerSize(size_t layer) const noexcept
      {
         return m_layers[layer].size();
      }
      virtual void ComputeOutputs(const TData *pinputs, OUT TData *poutputs) const
      {
         // TODO
      }
   };

   template <typename TNetwork>
   class Trainer
   {
      typedef typename TNetwork::val_t val_t;
      static_assert(std::is_base_of_v<INetwork<val_t>, TNetwork>);

      std::function<val_t(int)> m_RateFactory;
      std::shared_ptr<TNetwork> m_pnet;
      std::unique_ptr<val_t[]> m_pdeltas;

   public:
      template <typename TPtr, typename TFunc>
      Trainer(TPtr&& network, TFunc&& LearningRateFactory) : m_RateFactory(std::forward<TFunc>(LearningRateFactory)), m_pnet(std::forward<TPtr>(network)),
         m_pdeltas(std::make_unique<val_t[]>(m_pnet->GetNodesCount() + m_pnet->GetLayerCount()))
      { }
      template <typename TPtr>
      Trainer(TPtr&& network) : Trainer(std::forward<TPtr>(network), [](int t) { return 100.0 / (100.0 + t); })
      { }
      std::shared_ptr<TNetwork> GetNetwork() const noexcept
      {
         return m_pnet;
      }
      void Train(const std::vector<std::vector<val_t>>& trainingSet, const std::vector<std::vector<val_t>>& outputs)
      {
         assert(trainingSet.size() == outputs.size());

         // Dividing examples into trainings set and validation set 2:1
         std::vector<const std::vector<val_t> *> trainset;
         std::vector<const std::vector<val_t> *> trainout;
         std::vector<const std::vector<val_t> *> valset;
         std::vector<const std::vector<val_t> *> valout;

         size_t exCount = trainingSet.size();
         size_t valCount = exCount / 3;

         size_t i;
         for (i = 0; i < valCount; ++i) {
            valset.push_back(&trainingSet[i]);
            valout.push_back(&outputs[i]);
         }
         for (; i < exCount; ++i) {
            trainset.push_back(&trainingSet[i]);
            trainout.push_back(&outputs[i]);
         }

         m_pnet = InternalTrain(std::move(m_pnet), trainset, trainout, valset, valout);
      }

   private:
      std::shared_ptr<FixedSizeNetwork<val_t>> InternalTrain(
         std::shared_ptr<FixedSizeNetwork<val_t>> pnet, 
         const std::vector<const std::vector<val_t> *>& trainSet, const std::vector<const std::vector<val_t> *>& trainOuts,
         const std::vector<const std::vector<val_t> *>& valSet, const std::vector<const std::vector<val_t> *>& valOuts)
      {
         auto pvalres = std::make_unique<val_t[]>(valOuts.size()); // validation results
         val_t optErr;
         bool done = false;

         // back-propagation
         for (int t = 0; !done; ++t) {
            val_t alpha = m_RateFactory(t);

            for (int ex = 0; ex < trainSet.size(); ++ex) {
               const std::vector<val_t>& in = *(trainSet[ex]);
               const std::vector<val_t>& out = *(trainOuts[ex]);

               pnet->ComputeOutputs(in.data(), nullptr);

               // output layer
               int layer = pnet->GetLayerCount() - 1;
               for (int node = 0; node < pnet->GetLayerSize(layer); ++node) {
                  val_t a = pnet->GetOutputAt(layer, node);
                  Delta(layer, node) = a*(1.0 - a) * (out[node] - a);
               }

               // other layers
               for (--layer; layer >= 0; --layer) {
                  for (int node = 0; node < pnet->GetLayerSize(layer); ++node) {
                     val_t a = pnet->GetOutputAt(layer, node);

                     val_t sum = 0.0;
                     for (int prevnode = 0; prevnode < pnet->GetLayerSize(layer + 1); prevnode++) {
                        sum += Delta(layer + 1, prevnode) * pnet->GetNodeAt(layer + 1, prevnode)->GetWeightAt(node);
                     }
                     Delta(layer, node) = a*(1.0 - a) * sum;
                  }
               }

               // update weights
               for (int layer = 0; layer < pnet->GetLayerCount(); ++layer) {
                  for (int node = 0; node < pnet->GetLayerSize(layer); ++node) {
                     auto pnode = pnet->GetNodeAt(layer, node);

                     val_t delta = Delta(layer, node);
                     for (int i = 0; i < pnode->GetWeightsCount(); ++i) {
                        val_t a = (i == 0) ? 1.0 : ((layer == 0) ? in[i - 1] : pnet->GetOutputAt(layer - 1, i - 1));
                        val_t w = pnode->GetWeightAt(i) + alpha * a * delta;
                        pnode->SetWeightAt(i, w);
                     }

                  }
               }

            } // foreach ex

            if (t % 5 == 0) {
               // validation for each 5th epoch
               val_t avgErr = 0.0;
               for (int ex = 0; ex < valSet.size(); ++ex) {
                  const std::vector<val_t>& in = *(valSet[ex]);
                  const std::vector<val_t>& out = *(valOuts[ex]);

                  pnet->ComputeOutputs(in.data(), pvalres.get());

                  // error for the given example:
                  val_t err = std::abs(std::accumulate(pvalres.get(), pvalres.get() + out.size(), 0.0) - std::accumulate(out.cbegin(), out.cend(), 0.0));
                  err /= out.size();

                  avgErr += err;
               }
               avgErr /= valSet.size(); // avg error over all validation examples
               if (t == 0) {
                  optErr = avgErr;
                  continue;
               }
               // using stoppping criterion GL_2 from here: http://page.mi.fu-berlin.de/prechelt/Biblio/stop_neurnetw98.pdf
               if (avgErr <= optErr)
                  optErr = avgErr;
               else
                  done = (100.0 * (avgErr / optErr - 1)) > 2;
            }

         } // foreach epoch
         return std::move(pnet);
      }
      std::shared_ptr<TilingNetwork<val_t>> InternalTrain(
         std::shared_ptr<TilingNetwork<val_t>> pnet,
         const std::vector<const std::vector<val_t> *>& trainSet, const std::vector<const std::vector<val_t> *>& trainOuts,
         const std::vector<const std::vector<val_t> *>& valSet, const std::vector<const std::vector<val_t> *>& valOuts)
      {
         throw std::logic_error("Function not implemented");

         return std::move(pnet);
      }
      val_t& Delta(size_t layer, size_t node)
      {
         size_t ind = 0;
         for (int i = 0; i < layer; ++i)
            ind += m_pnet->GetLayerSize(layer);
         return m_pdeltas[ind + node];
      }     
   };


#pragma endregion


}