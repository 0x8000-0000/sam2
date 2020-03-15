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

void sam2::Document::pushBlock()
{
   std::cerr << "-- Block(" << m_currentIdentifier << ", " << m_currentDescription << ")\n";

   std::vector<Block::Element> newElements;
   newElements.emplace_back(Block{std::move(m_currentIdentifier), std::move(m_currentDescription), std::move(m_elements)});
   m_elements = std::move(newElements);

   m_currentIdentifier = std::string_view();
   m_textAccumulator.clear();
}
