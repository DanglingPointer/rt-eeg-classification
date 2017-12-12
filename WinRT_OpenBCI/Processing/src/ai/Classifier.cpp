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
#include "Learning.h"
#include "Classifier.h"

using namespace Processing;
using namespace Platform;

Processing::Classifier::Classifier()
{
   throw ref new Platform::NotImplementedException();
}

void Classifier::Foo()
{
   //auto pvec = std::make_shared<std::vector<double>>();

   //ContAttribute<double> attr(42.0, pvec, [](double id) { return id + id; });

   //TreeNode<ContAttribute<double>> node(std::shared_ptr<ContAttribute<double>>(&attr));

   auto WeightFactory = [](size_t fanIn) { return fanIn / 4.0; };
   FixedSizeNetwork<float> nn(31, 32, 3, 2);
   nn.ComputeOutputs(NULL, NULL);

   auto pnn = std::make_unique<FixedSizeNetwork<float>>(31, 31, 31, 31);

   Trainer<FixedSizeNetwork<float>> t(std::move(pnn));
   t.Train(std::vector<std::vector<float>>(), std::vector<std::vector<float>>());
}