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
#include "Dtc.h"

using namespace Processing;
using namespace Platform;

void Dtc::Foo()
{
   auto pvec = std::make_shared<std::vector<double>>();

   ContAttribute<double> attr(42.0, pvec, [](double id) { return id + id; });

   TreeNode<ContAttribute<double>> node(std::shared_ptr<ContAttribute<double>>(&attr));
}