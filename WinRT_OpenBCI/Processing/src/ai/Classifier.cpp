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
#include "pch.h"
#include "Classifier.h"
#include "Learning.h"

using namespace Platform;

Processing::Single::Classifier::Classifier() : m_pc(ref new Processing::Classifier<float>())
{ }

void Processing::Single::Classifier::CreateFixedSizeNetwork(int32 inputSize, int32 layerSize, int32 outputSize, int32 layerCount)
{
   m_pc->CreateBPNetwork(inputSize, layerSize, outputSize, layerCount);
}

void Processing::Single::Classifier::CreateCascadeNetwork(int32 inputSize, int32 outputSize)
{
   m_pc->CreateCCNetwork(inputSize, outputSize);
}

void Processing::Single::Classifier::AddExample(const Array<float>^ trainingInput, const Array<float>^ trainingOutput)
{
   m_pc->AddExample(trainingInput, trainingOutput);
}

IAsyncAction^ Processing::Single::Classifier::TrainAsync()
{
   return concurrency::create_async([this]() { m_pc->Train(); });   
}

IAsyncAction^ Processing::Single::Classifier::ClassifyAsync(const Array<float>^ data, WriteOnlyArray<float>^ output)
{
   return concurrency::create_async([=]() { m_pc->Classify(data, output); });   
}

void Processing::Single::Classifier::Classify(const Array<float>^ data, WriteOnlyArray<float>^ output)
{
   m_pc->Classify(data, output);
}


Processing::Double::Classifier::Classifier() : m_pc(ref new Processing::Classifier<double>())
{ }

void Processing::Double::Classifier::CreateFixedSizeNetwork(int32 inputSize, int32 layerSize, int32 outputSize, int32 layerCount)
{
   m_pc->CreateBPNetwork(inputSize, layerSize, outputSize, layerCount);
}

void Processing::Double::Classifier::CreateCascadeNetwork(int32 inputSize, int32 outputSize)
{
   m_pc->CreateCCNetwork(inputSize, outputSize);
}

void Processing::Double::Classifier::AddExample(const Array<double>^ trainingInput, const Array<double>^ trainingOutput)
{
   m_pc->AddExample(trainingInput, trainingOutput);
}

IAsyncAction^ Processing::Double::Classifier::TrainAsync()
{
   return concurrency::create_async([this]() { m_pc->Train(); });
}

IAsyncAction^ Processing::Double::Classifier::ClassifyAsync(const Array<double>^ data, WriteOnlyArray<double>^ output)
{
   return concurrency::create_async([=]() { m_pc->Classify(data, output); });
}

void Processing::Double::Classifier::Classify(const Array<double>^ data, WriteOnlyArray<double>^ output)
{
   m_pc->Classify(data, output);
}
