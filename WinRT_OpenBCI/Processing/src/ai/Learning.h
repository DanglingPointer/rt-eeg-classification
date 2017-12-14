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
#include <utility>
#include <algorithm>
#include <vector>
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

#ifdef REQUIRES_FLOAT
 #undef REQUIRES_FLOAT
#endif

#define REQUIRES_FLOAT(T) static_assert(std::is_floating_point_v<T>)

#ifndef OUT
 #define OUT
#endif

namespace Processing
{

#pragma region Perceptron

   template <typename TData>
   struct SgnFunc
   {
      REQUIRES_FLOAT(TData);

      TData operator()(TData in) const
      {
         return in == 0.0 ? in : (in > 0 ? 1.0 : -1.0);
      }
   };
   template <typename TData>
   struct SigmoidFunc
   {
      REQUIRES_FLOAT(TData);

      TData operator()(TData in) const
      {
         return 1.0 / (1.0 + std::exp(-in));
      }
   };

   template <typename TData, template <typename> typename TFunc>
   class Unit
   {
      REQUIRES_FLOAT(TData);

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

#pragma endregion

   
#pragma region Interfaces

   template <typename TData>
   class ITrainer;

   template <typename TData>
   class INetwork
   {
      REQUIRES_FLOAT(TData);
   public:
      virtual ~INetwork() { }
      virtual size_t GetNodesCount() const noexcept = 0;
      virtual size_t GetLayerCount() const noexcept = 0;
      virtual size_t GetLayerSize(size_t layer) const noexcept = 0;
      virtual void ComputeOutputs(const TData *pinputs, OUT TData *poutputs) const = 0;
      virtual std::unique_ptr<ITrainer<TData>> GetTrainer() = 0;
   };

   template <typename TData>
   class ITrainer
   {
      REQUIRES_FLOAT(TData);
   public:
      virtual ~ITrainer() { }
      virtual std::shared_ptr<INetwork<TData>> GetNetwork() const noexcept = 0;
      virtual void Train(const std::vector<std::vector<TData>>& trainingSet, const std::vector<std::vector<TData>>& outputs) = 0;
   };

#pragma endregion


#pragma region Implementations

   // https://infoscience.epfl.ch/record/82296/files/94-07.pdf, L.Bottou approach
   template <typename TData>
   TData BottouWeightFactory(size_t fanIn)
   {
      REQUIRES_FLOAT(TData);
      std::random_device rd;
      TData bound = 2.38 / std::sqrt((TData)fanIn);
      std::uniform_real_distribution<TData> u(-bound, bound);
      return u(rd);
   }

   template <typename TData, template <typename> typename TNetwork>
   class Trainer;

   template <typename TData>
   class FixedSizeNetwork : public INetwork<TData>
   {
   public:
      template<typename T>
      using My_tt = FixedSizeNetwork<T>;
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
         return layer == L - 1 ? OUT_N : N;
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
      virtual std::unique_ptr<ITrainer<TData>> GetTrainer()
      {
         return std::make_unique<Trainer<TData, My_tt>>(this);
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
   };

   template <typename TData>
   class TilingNetwork : public INetwork<TData>
   {
   public:
      template<typename T>
      using My_tt = TilingNetwork<T>;
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
      virtual std::unique_ptr<ITrainer<TData>> GetTrainer()
      {
         return std::make_unique<Trainer<TData, My_tt>>(this);
      }
   };

   template <typename TData, template <typename> typename TNetwork>
   class Trainer : public ITrainer<TData>
   {
      typedef TData val_t;
      static_assert(std::is_base_of_v<INetwork<val_t>, TNetwork<val_t>>);

      std::function<val_t(int)> m_RateFactory;
      std::shared_ptr<TNetwork<val_t>> m_pnet;
      std::unique_ptr<val_t[]> m_pdeltas;

   public:
      template <typename TPtr, typename TFunc>
      Trainer(TPtr&& network, TFunc&& LearningRateFactory) : m_RateFactory(std::forward<TFunc>(LearningRateFactory)), m_pnet(std::forward<TPtr>(network)),
         m_pdeltas(std::make_unique<val_t[]>(m_pnet->GetNodesCount() + m_pnet->GetLayerCount()))
      { }
      template <typename TPtr>
      explicit Trainer(TPtr&& network) : Trainer(std::forward<TPtr>(network), [](int t) { return 100.0 / (100.0 + t); })
      { }
      std::shared_ptr<INetwork<val_t>> GetNetwork() const noexcept
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
                  Delta(layer, node) = a * (1.0 - a) * (out[node] - a);
               }

               // other layers
               for (--layer; layer >= 0; --layer) {
                  for (int node = 0; node < pnet->GetLayerSize(layer); ++node) {
                     val_t a = pnet->GetOutputAt(layer, node);

                     val_t sum = 0.0;
                     for (int prevnode = 0; prevnode < pnet->GetLayerSize(layer + 1); prevnode++) {
                        sum += Delta(layer + 1, prevnode) * pnet->GetNodeAt(layer + 1, prevnode)->GetWeightAt(node);
                     }
                     Delta(layer, node) = a * (1.0 - a) * sum;
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


#pragma region C++/CX classes

   template <typename TData>
   private ref class Classifier
   {
      REQUIRES_FLOAT(TData);

      std::unique_ptr<INetwork<TData>> m_pNetwork;

      std::vector<std::vector<TData>> m_inputs;
      std::vector<std::vector<TData>> m_outputs;

   internal:
      Classifier() : m_pNetwork(nullptr)
      { }
      void CreateFixedSizeNetwork(int32 inN, int32 N, int32 outN, int32 L)
      {
         m_pNetwork = std::make_unique<FixedSizeNetwork<TData>>(inN, N, outN, L, &BottouWeightFactory<TData>);
      }
      void AddExample(const Array<TData>^ input, const Array<TData>^ output)
      {
         std::vector<TData> in(begin(input), end(input));
         std::vector<TData> out(begin(output), end(output));

         Normalize(in);
         Normalize(out);

         m_inputs.emplace_back(std::move(in));
         m_outputs.emplace_back(std::move(out));;
      }
      void Train()
      {
         auto ptrainer = m_pNetwork->GetTrainer();
         ptrainer->Train(m_inputs, m_outputs);

         m_inputs.clear();
         m_outputs.clear();
      }
      void Classify(const Array<TData>^ data, WriteOnlyArray<TData>^ output)
      {
         std::vector<TData> norm(begin(data), end(data));
         Normalize(norm);
         m_pNetwork->ComputeOutputs(norm.data(), output->Data);
      }

   private:
      static void Normalize(std::vector<TData>& data)
      {
         TData mean = std::accumulate(data.begin(), data.end(), 0.0);
         mean /= data.size();

         std::vector<TData> temp(data.size());
         std::transform(data.begin(), data.end(), temp.begin(), [mean](TData val) { return val - mean; });
         TData sd = std::inner_product(temp.begin(), temp.end(), temp.begin(), 0.0);
         sd = std::sqrt(sd / (temp.size() - 1));
         
         std::transform(data.begin(), data.end(), data.begin(), [mean, sd](TData val) { return (val - mean) / sd; });
      }
   };

#pragma endregion

}