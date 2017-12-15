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
         return (TData)(in == 0.0 ? in : (in > 0 ? 1.0 : -1.0));
      }
   };
   template <typename TData>
   struct SigmoidFunc
   {
      REQUIRES_FLOAT(TData);

      TData operator()(TData in) const
      {
         return (TData)1.0 / ((TData)1.0 + std::exp(-in));
      }
   };

   // wrapper, doesn't allocate anything
   template <typename TData, template <typename> typename TFunc>
   class Unit
   {
      REQUIRES_FLOAT(TData);

      TFunc<TData> m_Func;
      const size_t m_size;
      TData * const m_pweights;
      TData m_biasWeight;

   public:
      // size - length of the pweights array, doesn't include the bias weight
      Unit(TData *pweights, size_t size) : m_size(size + 1), m_pweights(pweights), m_biasWeight(*pweights)
      { }
      size_t GetWeightsCount() const noexcept
      {
         return m_size;
      }
      void SetWeightAt(int index, TData val)
      {
         assert(index < m_size);
         if (index == 0)
            m_biasWeight = val;
         else
            m_pweights[index - 1] = val;
      }
      TData GetWeightAt(int index) const
      {
         assert(index < m_size);
         return index == 0 ? m_biasWeight : m_pweights[index - 1];
      }
      TData GetOutput(const TData *pinputs) const
      {
         TData sum = std::inner_product(m_pweights, m_pweights + m_size, pinputs, m_biasWeight);
         return m_Func(sum);
      }
   };

   template <typename TData>
   using SgnUnit = Unit<TData, SgnFunc>;

   template <typename TData>
   using SigmoidUnit = Unit<TData, SigmoidFunc>;

#pragma endregion

   
#pragma region Interfaces

   // forward declaration
   template <typename TData>
   class ITrainer;

   template <typename TData>
   class INetwork
   {
      REQUIRES_FLOAT(TData);
   public:
      virtual ~INetwork() { }
      virtual size_t GetNodeCount() const noexcept = 0;
      virtual size_t GetInputCount() const noexcept = 0;
      virtual size_t GetOutputCount() const noexcept = 0;
      virtual void ComputeOutputs(const TData *pinputs, OUT TData *poutputs) const = 0;
      virtual std::unique_ptr<ITrainer<TData>> CreateTrainer() = 0;
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


#pragma region Networks
   
   // declaration
   template <typename TNetwork>
   class Trainer;
      
   // Back-propagating network
   template <typename TData>
   class BPNetwork : public INetwork<TData>
   {
   public:
      typedef TData val_t;
      typedef BPNetwork<TData> My_t;
      typedef INetwork<TData> Base_t;
      typedef SigmoidUnit<TData> Node_t;
      typedef unsigned char byte;

   private:
      std::unique_ptr<byte[]> m_pmemblock;
      byte * const m_pnodes;
      byte * const m_pweights;
      byte * const m_pouts;
      const size_t IN_N, OUT_N, N, L;

      BPNetwork(size_t nodeCount, size_t weightsCount, size_t inN, size_t N, size_t outN, size_t L)
         : m_pmemblock(std::make_unique<byte[]>((sizeof(Node_t) + sizeof(TData)) * nodeCount + sizeof(TData) * weightsCount)),
         m_pnodes(m_pmemblock.get()),
         m_pweights(m_pnodes + sizeof(Node_t) * nodeCount),
         m_pouts(m_pweights + sizeof(TData) * weightsCount),
         IN_N(inN), OUT_N(outN), N(N), L(L)
      { }

   public:
      // inN doesn't include bias unit
      // InitWeightFactory takes fan-in (size_t) and returns one initial weight (TData)
      template <typename F>
      BPNetwork(size_t inN, size_t N, size_t outN, size_t L, F InitWeightFactory)
         : BPNetwork((L - 1) * N + outN,
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
               pw += sizeof(TData) * wlen;

               // allocate node
               Node_t *pnode = new(pn) Node_t(pweights, wlen);
               pn += sizeof(Node_t);

               // fill initial weights
               for (int i = 0; i < wlen; ++i) 
                  pweights[i] = InitWeightFactory(wlen + 1);
               TData norm = std::sqrt(std::inner_product(pweights, pweights+wlen, pweights, 0.0));
               pnode->SetWeightAt(0, 0.75 * norm); // Russel & Marks, Neural Smithing, p.103
            }
         }
      }
      BPNetwork(const My_t&) = delete;
      My_t& operator =(const My_t&) = delete;

      virtual size_t GetNodeCount() const noexcept
      {
         return N * (L - 1) + OUT_N;
      }
      virtual size_t GetInputCount() const noexcept
      {
         return IN_N;
      }
      virtual size_t GetOutputCount() const noexcept
      {
         return OUT_N;
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
      virtual std::unique_ptr<ITrainer<TData>> CreateTrainer()
      {
         return std::make_unique<Trainer<My_t>>(this);
      }

      size_t GetLayerCount() const noexcept
      {
         return L;
      }
      size_t GetLayerSize(size_t layer) const noexcept
      {
         return layer == L - 1 ? OUT_N : N;
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

   // Cascade-correlation network
   template <typename TData>
   class CCNetwork : public INetwork<TData>
   {
   public:
      typedef TData val_t;
      typedef CCNetwork<TData> My_t;
      typedef INetwork<TData> Base_t;
      typedef SigmoidUnit<TData> Node_t;

   private:
      const size_t IN_N, OUT_N;

      std::vector<Node_t> m_outNodes;
      std::vector<Node_t> m_hidNodes;

      std::vector<TData> m_outWeights;
      std::vector<TData> m_hidWeights;
      mutable std::vector<TData> m_outputs;  // first hidden nodes, then output layer

      std::function<TData(size_t)> m_WeightFactory;

   public:
      template <typename TFunc>
      CCNetwork(size_t inN, size_t outN, TFunc&& InitWeightFactory) : IN_N(inN), OUT_N(outN),
         m_outWeights(OUT_N * IN_N), m_WeightFactory(std::forward<TFunc>(InitWeightFactory))
      { 
         GenerateOutputNodes(IN_N);

         for (TData& w : m_outWeights) {
            w = m_WeightFactory(IN_N + 1);
         }
         for (int i = 0; i < m_outNodes.size(); ++i) {
            Node_t *pnode = &m_outNodes[i];
            TData *pweights = &m_outWeights[i * IN_N];
            SetBiasWeight(pnode, pweights, IN_N);
         }
      }
      CCNetwork(const My_t&) = delete;
      My_t& operator =(const My_t&) = delete;

      virtual size_t GetNodeCount() const noexcept
      {
         return m_hidNodes.size() + OUT_N;
      }
      virtual size_t GetInputCount() const noexcept
      {
         return IN_N;
      }
      virtual size_t GetOutputCount() const noexcept
      {
         return OUT_N;
      }
      virtual void ComputeOutputs(const TData *pinputs, OUT TData *poutputs) const
      {
         assert(pinputs != nullptr);

         m_outputs.clear();
         std::vector<TData> inputs(pinputs, pinputs + IN_N);

         for (const Node_t& node : m_hidNodes) {
            TData out = node.GetOutput(inputs.data());
            inputs.push_back(out);
            m_outputs.push_back(out);
         }
         for (const Node_t& node : m_outNodes) {
            TData out = node.GetOutput(inputs.data());
            m_outputs.push_back(out);
         }

         if (poutputs)
            std::copy(m_outputs.cbegin() + m_hidNodes.size(), m_outputs.cend(), poutputs);
      }
      virtual std::unique_ptr<ITrainer<TData>> CreateTrainer()
      {
         return std::make_unique<Trainer<My_t>>(this);
      }

      // Adds a new hidden unit and regenerates the output layer
      Node_t *AddHiddenNode()
      {
         // add hidden weights
         size_t wlen = IN_N + m_hidNodes.size();
         for (int i = 0; i < wlen; ++i)
            m_hidWeights.push_back(m_WeightFactory(wlen + 1));
         TData *pweights = &m_hidWeights[m_hidWeights.size() - wlen];

         // add hidden node
         m_hidNodes.emplace_back(pweights, wlen);

         // regenerate output nodes retaining previous weights
         m_outNodes.clear();
         std::vector<TData> prevw(std::move(m_outWeights));
         GenerateOutputNodes(IN_N + m_hidNodes.size());

         wlen++;
         for (int i = 0; i < OUT_N; ++i) {
            // retain the previous weights
            auto srcFirst = prevw.cbegin() + i * (wlen - 1);
            auto srcLast = srcFirst + (wlen - 1);
            auto destFirst = m_outWeights.begin() + i * wlen;
            std::copy(srcFirst, srcLast, destFirst);

            // generate randomly the new extra weight, and calculate bias weight
            Node_t *pnode = &m_outNodes[i];
            pnode->SetWeightAt(wlen, m_WeightFactory(wlen + 1));
            SetBiasWeight(pnode, &(*destFirst), wlen);
         }
         return &m_hidNodes.back();
      }
      Node_t *GetOutputNode(size_t outIndex)
      {
         assert(outIndex < m_outNodes.size());
         return &m_outNodes[outIndex];
      }
      Node_t *GetHiddenNode(size_t hidIndex)
      {
         assert(hidIndex < m_hidNodes.size());
         return &m_hidNodes[hidIndex];
      }
      TData GetHiddenOutput(size_t hidIndex) const
      {
         assert(hidIndex < m_hidNodes.size());
         return m_outputs[hidIndex];
      }
      
   private:
      // does not assign initial weights
      void GenerateOutputNodes(size_t wlen)
      {
         m_outWeights.resize(OUT_N * wlen);

         m_outNodes.clear();
         for (int i = 0; i < OUT_N; ++i) {
            TData *pweights = &m_outWeights[i * wlen];
            m_outNodes.emplace_back(pweights, wlen);
         }
      }
      void SetBiasWeight(Node_t *pnode, TData *pweights, size_t wlen)
      {
         TData norm = std::sqrt(std::inner_product(pweights, pweights + wlen, pweights, 0.0));
         pnode->SetWeightAt(0, 0.75 * norm);
      }
   };

   // https://infoscience.epfl.ch/record/82296/files/94-07.pdf, L.Bottou approach
   template <typename TData>
   TData BottouWeightFactory(size_t fanIn)
   {
      REQUIRES_FLOAT(TData);
      std::random_device rd;
      TData bound = (TData)2.38 / std::sqrt((TData)fanIn);
      std::uniform_real_distribution<TData> u(-bound, bound);
      return u(rd);
   }

   template <typename TData>
   TData DefaultLearningRate(int t)
   {
      REQUIRES_FLOAT(TData);
      return (TData)100.0 / ((TData)100.0 + t);
   }

#pragma endregion


#pragma region Trainers

   template <typename TData, template <typename> typename TNetwork>
   class TrainerBase : public ITrainer<TData>
   {
   protected:
      typedef TData val_t;

      std::function<val_t(int)> m_RateFactory;
      std::shared_ptr<TNetwork<val_t>> m_pnet;

      template <typename TPtr, typename TFunc>
      TrainerBase(TPtr&& network, TFunc&& LearningRateFactory) 
         : m_RateFactory(std::forward<TFunc>(LearningRateFactory)), m_pnet(std::forward<TPtr>(network))
      { }
      // avg error over all validation examples
      TData GetAvgError(val_t *pvalres,
                        const std::vector<const std::vector<val_t> *>& valSet, 
                        const std::vector<const std::vector<val_t> *>& valOuts)
      {
         val_t avgErr = 0.0;
         for (int ex = 0; ex < valSet.size(); ++ex) {
            const std::vector<val_t>& in = *(valSet[ex]);
            const std::vector<val_t>& out = *(valOuts[ex]);

            m_pnet->ComputeOutputs(in.data(), pvalres);

            // error for the given example:
            val_t err = std::abs(std::accumulate(pvalres, pvalres + out.size(), 0) - std::accumulate(out.cbegin(), out.cend(), 0));
            err /= out.size();

            avgErr += err;
         }
         return avgErr / valSet.size(); // avg error over all validation examples
      }

   public:
      virtual std::shared_ptr<INetwork<val_t>> GetNetwork() const noexcept
      {
         return m_pnet;
      }
      virtual void Train(const std::vector<std::vector<val_t>>& trainingSet, const std::vector<std::vector<val_t>>& outputs)
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
         InternalTrain(trainset, trainout, valset, valout);
      }
   protected:
      virtual void InternalTrain(
         const std::vector<const std::vector<val_t> *>& trainSet, const std::vector<const std::vector<val_t> *>& trainOuts,
         const std::vector<const std::vector<val_t> *>& valSet, const std::vector<const std::vector<val_t> *>& valOuts) = 0;

   };

   template <typename TData>
   class Trainer<BPNetwork<TData>> : public TrainerBase<TData, BPNetwork>
   {
   public:
      typedef TrainerBase<TData, BPNetwork> Base_t;
      typedef typename Base_t::val_t val_t;

   private:
      std::unique_ptr<val_t[]> m_pdeltas;

   public:
      template <typename TPtr>
      explicit Trainer(TPtr&& network) : Base_t(std::forward<TPtr>(network), &DefaultLearningRate<val_t>),
         m_pdeltas(std::make_unique<val_t[]>(m_pnet->GetNodeCount()))
      { }

   protected:
      val_t& Delta(size_t layer, size_t node)
      {
         size_t ind = 0;
         for (int i = 0; i < layer; ++i)
            ind += m_pnet->GetLayerSize(i);
         return m_pdeltas[ind + node];
      }
      virtual void InternalTrain(
         const std::vector<const std::vector<val_t> *>& trainSet, const std::vector<const std::vector<val_t> *>& trainOuts,
         const std::vector<const std::vector<val_t> *>& valSet, const std::vector<const std::vector<val_t> *>& valOuts)
      {
         auto pvalres = std::make_unique<val_t[]>(m_pnet->GetOutputCount()); // validation results
         val_t optErr;
         bool done = false;

         // back-propagation
         for (int t = 0; !done; ++t) {
            val_t alpha = m_RateFactory(t);

            for (int ex = 0; ex < trainSet.size(); ++ex) {
               const std::vector<val_t>& in = *(trainSet[ex]);
               const std::vector<val_t>& out = *(trainOuts[ex]);

               m_pnet->ComputeOutputs(in.data(), nullptr);

               // output layer
               int layer = m_pnet->GetLayerCount() - 1;
               for (int node = 0; node < m_pnet->GetLayerSize(layer); ++node) {
                  val_t a = m_pnet->GetOutputAt(layer, node);
                  Delta(layer, node) = a * (1 - a) * (out[node] - a);
               }

               // other layers
               for (--layer; layer >= 0; --layer) {
                  for (int node = 0; node < m_pnet->GetLayerSize(layer); ++node) {
                     val_t a = m_pnet->GetOutputAt(layer, node);

                     val_t sum = 0.0;
                     for (int prevnode = 0; prevnode < m_pnet->GetLayerSize(layer + 1); prevnode++) {
                        sum += Delta(layer + 1, prevnode) * m_pnet->GetNodeAt(layer + 1, prevnode)->GetWeightAt(node);
                     }
                     Delta(layer, node) = a * (1 - a) * sum;
                  }
               }

               // update weights
               for (int layer = 0; layer < m_pnet->GetLayerCount(); ++layer) {
                  for (int node = 0; node < m_pnet->GetLayerSize(layer); ++node) {
                     auto pnode = m_pnet->GetNodeAt(layer, node);

                     val_t delta = Delta(layer, node);
                     for (int i = 0; i < pnode->GetWeightsCount(); ++i) {
                        val_t a = (i == 0) ? 1.0 : ((layer == 0) ? in[i - 1] : m_pnet->GetOutputAt(layer - 1, i - 1));
                        val_t w = pnode->GetWeightAt(i) + alpha * a * delta;
                        pnode->SetWeightAt(i, w);
                     }

                  }
               }

            } // foreach ex

            if (t % 5 == 0) {
               // validation for each 5th epoch
               val_t avgErr = GetAvgError(pvalres.get(), valSet, valOuts); 

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
      }
   };

   template <typename TData>
   class Trainer<CCNetwork<TData>> : public TrainerBase<TData, CCNetwork>
   {
   public:
      typedef TrainerBase<TData, CCNetwork> Base_t;
      typedef typename Base_t::val_t val_t;

   private:
      const val_t m_errThres;

   public:
      template <typename TPtr>
      Trainer(TPtr&& pnetwork, val_t errThreshold = 0.01) : Base_t(std::forward<TPtr>(pnetwork), &DefaultLearningRate<val_t>), m_errThres(errThreshold)
      { }

   protected:
      void TrainOutputs(const std::vector<const std::vector<val_t> *>& trainSet, 
                        const std::vector<const std::vector<val_t> *>& trainOuts, 
                        val_t *pouts, val_t alpha)
      {
         size_t inputCount = m_pnet->GetInputCount();
         size_t outputCount = m_pnet->GetOutputCount();
         for (int ex = 0; ex < trainSet.size(); ++ex) {
            const std::vector<val_t>& in = *(trainSet[ex]);
            const std::vector<val_t>& out = *(trainOuts[ex]);

            m_pnet->ComputeOutputs(in.data(), pouts);

            for (size_t i = 0; i < outputCount; ++i) {
               auto pnode = m_pnet->GetOutputNode(i);
               val_t a = pouts[i];
               val_t delta = a * (1 - a) * (out[i] - a);

               // bias weight
               val_t w0 = pnode->GetWeightAt(0) + alpha * delta;
               pnode->SetWeightAt(0, w0);

               // weights from inputs
               for (size_t input = 0; input < inputCount; ++input) {
                  val_t w = pnode->GetWeightAt(input + 1) + alpha * in[input] * delta;
                  pnode->SetWeightAt(input + 1, w);
               }
               // weights from hidden nodes
               size_t hiddenNodeCount = m_pnet->GetNodeCount() - outputCount;
               for (size_t node = 0; node < hiddenNodeCount; ++node) {
                  int wInd = node + 1 + inputCount;
                  val_t w = pnode->GetWeightAt(wInd) + alpha * m_pnet->GetHiddenOutput(node) * delta;
                  pnode->SetWeightAt(wInd, w);
               }
            }
         }
      }
      void AddNode(const std::vector<const std::vector<val_t> *>& valSet, const std::vector<const std::vector<val_t> *>& valOuts)
      {
         size_t inputCount = m_pnet->GetInputCount();
         size_t outputCount = m_pnet->GetOutputCount();
         size_t hiddenNodeCount = m_pnet->GetNodeCount() - outputCount;
         size_t exCount = valSet.size();

         // --- Compute and save necessary data ---
         std::vector<std::vector<val_t>> errors;         // [ex][output]
         std::vector<std::vector<val_t>> states;         // [ex][node]
         std::vector<val_t> avgErrors(outputCount, 0.0); // [output]
         std::vector<val_t> avgOutput(outputCount, 0.0); // [output]

         for (size_t ex = 0; ex < exCount; ++ex) {
            const std::vector<val_t>& in = *(valSet[ex]);
            const std::vector<val_t>& out = *(valOuts[ex]);

            std::vector<val_t> tempStates(m_pnet->GetNodeCount());
            std::vector<val_t> tempOut(outputCount);
            m_pnet->ComputeOutputs(in.data(), tempOut.data());

            // fill node states (inputs + hidden nodes + outputs)
            std::copy(in.cbegin(), in.cend(), tempStates.begin()); // inputs
            for (size_t node = 0; node < hiddenNodeCount; ++node)
               tempStates[inputCount + node] = m_pnet->GetHiddenOutput(node); // hidden states
            std::copy(tempOut.cbegin(), tempOut.cend(), tempStates.begin() + inputCount + hiddenNodeCount); // outputs
            states.emplace_back(std::move(tempStates));

            // sum outputs and errors
            for (size_t output = 0; output < outputCount; ++output) {
               avgOutput[output] += tempOut[output];
               tempOut[output] = std::abs(tempOut[output] - out[output]);
               avgErrors[output] += tempOut[output];
            }
            errors.emplace_back(std::move(tempOut));
         }
         // calculate average errors and outputs
         for (size_t output = 0; output < outputCount; ++output) {
            avgErrors[output] /= exCount;
            avgOutput[output] /= exCount;
         }

         // --- Add a new node ---
         auto pnode = m_pnet->AddHiddenNode();


         // --- Gradient ascend ---
         bool done = false;
         for (size_t t = 0; !done; ++t) {
            val_t alpha = m_RateFactory(t);

            done = true;
            for (size_t wInd = 0; wInd < pnode->GetWeightsCount(); ++wInd) {

               // calculate the new node's outputs for each example
               std::vector<val_t> nodeState(exCount); // [ex]
               for (size_t ex = 0; ex < exCount; ++ex) {
                  m_pnet->ComputeOutputs((valSet[ex])->data(), nullptr);
                  nodeState[ex] = m_pnet->GetHiddenOutput(hiddenNodeCount);
               }
               val_t avgNodeState = std::accumulate(nodeState.cbegin(), nodeState.cend(), 0.0) / exCount;

               // calculate dS/dw
               val_t sprime = (val_t)0;
               for (size_t output = 0; outputCount; ++output) {

                  val_t cor = 0;
                  for (size_t ex = 0; ex < exCount; ++ex)
                     cor += (nodeState[ex] - avgNodeState) * (states[ex][inputCount + hiddenNodeCount + output] - avgOutput[output]);
                  val_t corSign = cor > 0 ? 1 : -1;

                  for (size_t ex = 0; ex < exCount; ++ex) {
                     val_t fprime = nodeState[ex] * (1 - nodeState[ex]);
                     val_t i = wInd == 0 ? 1 : states[ex][wInd - 1]; // because of bias weight

                     sprime += corSign * (errors[ex][output] - avgErrors[output]) * fprime * i;
                  }
               }
               // update w by gradient ascend
               val_t prevW = pnode->GetWeightAt(wInd);
               val_t w = prevW + alpha * sprime;
               pnode->SetWeightAt(wInd, w);

               // continue if at least one w changed by more than 1 %
               if ((std::abs(w - prevW) / prevW) > 0.01) {
                  done = false;
               }
            } // w
         }
      }
      virtual void InternalTrain(
         const std::vector<const std::vector<val_t> *>& trainSet, const std::vector<const std::vector<val_t> *>& trainOuts,
         const std::vector<const std::vector<val_t> *>& valSet, const std::vector<const std::vector<val_t> *>& valOuts)
      {
         auto pouts = std::make_unique<val_t[]>(m_pnet->GetOutputCount());
         val_t optErr, prevErr, lastNodeErr;

         for (int t = 0; true; ++t) {

            // Train output nodes using training set
            val_t alpha = m_RateFactory(t);
            TrainOutputs(trainSet, trainOuts, pouts.get(), alpha);

            // Compute error
            val_t avgErr = GetAvgError(pouts.get(), valSet, valOuts);

            // Check if we are done (stoppping criterion GL_2 from here: http://page.mi.fu-berlin.de/prechelt/Biblio/stop_neurnetw98.pdf)
            if (t == 0) {
               optErr = avgErr;
               continue;
            }
            if (avgErr <= optErr)
               optErr = avgErr;
            else if ((100.0 * (avgErr / optErr - 1)) > 2)
               return;

            // Check if we need to add a new node ("Neural Smithing", p.199)
            if (t == 0) {
               prevErr = lastNodeErr = avgErr;
               continue;
            }
            val_t deltaT = std::abs(avgErr - prevErr) / lastNodeErr;
            prevErr = avgErr;
            if (deltaT > m_errThres) {
               continue;
            }

            // Adding and training new node
            lastNodeErr = avgErr;
            AddNode(valSet, valOuts);

         } // epoch
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
      void CreateBPNetwork(int32 inN, int32 N, int32 outN, int32 L)
      {
         m_pNetwork = std::make_unique<BPNetwork<TData>>(inN, N, outN, L, &BottouWeightFactory<TData>);
      }
      void CreateCCNetwork(int32 inN, int32 outN)
      {
         m_pNetwork = std::make_unique<CCNetwork<TData>>(inN, outN, &BottouWeightFactory<TData>);
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
         auto ptrainer = m_pNetwork->CreateTrainer();
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
         TData mean = std::accumulate(data.begin(), data.end(), (TData)0.0);
         mean /= data.size();

         std::vector<TData> temp(data.size());
         std::transform(data.begin(), data.end(), temp.begin(), [mean](TData val) { return val - mean; });
         TData sd = std::inner_product(temp.begin(), temp.end(), temp.begin(), (TData)0.0);
         sd = std::sqrt(sd / (temp.size() - 1));
         
         std::transform(data.begin(), data.end(), data.begin(), [mean, sd](TData val) { return (val - mean) / sd; });
      }
   };

#pragma endregion

}