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

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

#define REQUIRES_FLOAT(T) typename = std::enable_if_t<std::is_floating_point_v<T>>

namespace Processing
{
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

}