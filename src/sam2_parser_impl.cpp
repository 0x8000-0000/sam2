/*
   Copyright 2020 Florin Iucha

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "sam2_parser.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <string_view>

sam2::Paragraph::Paragraph(const std::vector<std::string_view>& segments)
{
   size_t paragraphLength = std::accumulate(
      segments.cbegin(), segments.cend(), size_t{0U}, [](size_t partial, const std::string_view& view) {
         return partial + view.size();
      });

   m_text.reserve(paragraphLength + segments.size() + 1);
   std::for_each(segments.cbegin(), segments.cend(), [this](const std::string_view& view) {
      m_text.insert(m_text.end(), view.cbegin(), view.cend());
      m_text.push_back(' ');
   });

   m_text.back() = '\0';
}

void sam2::Document::pushParagraph()
{
   m_elements.emplace_back(Paragraph{m_textAccumulator});

   m_textAccumulator.clear();
}

void sam2::Document::startBlock()
{
   m_identifierStack.push(std::move(m_currentIdentifier));
   m_currentIdentifier = std::string();

   m_descriptionStack.push(std::move(m_currentDescription));
   m_currentDescription = std::string();

   m_elementStack.push(std::move(m_elements));
   m_elements = std::vector<Block::Element>();
}

void sam2::Document::finishBlock()
{
   if (!m_identifierStack.empty())
   {
      std::swap(m_currentIdentifier, m_identifierStack.top());
      std::swap(m_currentDescription, m_descriptionStack.top());
      m_identifierStack.pop();
      m_descriptionStack.pop();
   }
   else
   {
      assert(m_elementStack.empty());
   }

#ifdef MANUAL_TRACE
   std::cerr << "-- Block(" << m_currentIdentifier << ", " << m_currentDescription << ")\n";
#endif

   if (!m_elementStack.empty())
   {
      m_elementStack.top().emplace_back(
         Block{std::move(m_currentIdentifier), std::move(m_currentDescription), std::move(m_elements)});

      std::swap(m_elements, m_elementStack.top());
      m_elementStack.pop();
   }
   else
   {
      m_elements.emplace_back(
         Block{std::move(m_currentIdentifier), std::move(m_currentDescription), std::vector<Block::Element>()});
   }

   m_currentIdentifier  = std::string_view();
   m_currentDescription = std::string_view();
   m_textAccumulator.clear();
}

namespace
{
class StreamPrinter
{
public:
   explicit StreamPrinter(std::ostream& os) : m_os{os}
   {
   }

   void increaseLevel() noexcept
   {
      ++m_level;
   }

   void decreaseLevel() noexcept
   {
      --m_level;
   }

   void printIndent() const noexcept
   {
      for (int ii = 0; ii < m_level; ++ii)
      {
         m_os << "   ";
      }
   }

   void operator()(const sam2::Paragraph& para)
   {
      printIndent();
      m_os << para.getText() << "\n\n";
   }

   void operator()(const sam2::Block& block)
   {
      printIndent();
      m_os << block.getType() << " " << block.getDescription() << "\n\n";
      increaseLevel();
      block.forEachElement(*this);
      decreaseLevel();
      m_os << '\n';
   }

private:
   std::ostream& m_os;
   int           m_level = 0;
};
} // namespace

std::ostream& operator<<(std::ostream& os, const sam2::Document& doc)
{
   StreamPrinter printer(os);
   doc.forEachElement(printer);
   return os;
}
