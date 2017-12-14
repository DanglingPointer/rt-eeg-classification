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

using namespace Processing;
using namespace Platform;

ClassifierSingle::ClassifierSingle() : m_pc(ref new Classifier<float>())
{ }

void ClassifierSingle::CreateFixedSizeNetwork(int32 inputSize, int32 layerSize, int32 outputSize, int32 layerCount)
{
   m_pc->CreateFixedSizeNetwork(inputSize, layerSize, outputSize, layerCount);
}

void ClassifierSingle::AddExample(const Array<float>^ trainingInput, const Array<float>^ trainingOutput)
{
   m_pc->AddExample(trainingInput, trainingOutput);
}

void ClassifierSingle::Train()
{
   m_pc->Train();
}

void ClassifierSingle::Classify(const Array<float>^ data, WriteOnlyArray<float>^ output)
{
   m_pc->Classify(data, output);
}


ClassifierDouble::ClassifierDouble() : m_pc(ref new Classifier<double>())
{ }

void ClassifierDouble::CreateFixedSizeNetwork(int32 inputSize, int32 layerSize, int32 outputSize, int32 layerCount)
{
   m_pc->CreateFixedSizeNetwork(inputSize, layerSize, outputSize, layerCount);
}

void ClassifierDouble::AddExample(const Array<double>^ trainingInput, const Array<double>^ trainingOutput)
{
   m_pc->AddExample(trainingInput, trainingOutput);
}

void ClassifierDouble::Train()
{
   m_pc->Train();
}

void ClassifierDouble::Classify(const Array<double>^ data, WriteOnlyArray<double>^ output)
{
   m_pc->Classify(data, output);
}
