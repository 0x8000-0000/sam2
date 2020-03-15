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
#include <numeric>

void sam2::Document::pushParagraph()
{
   size_t paragraphLength =
      std::accumulate(m_paragraphAccumulator.cbegin(),
                      m_paragraphAccumulator.cend(),
                      size_t{0U},
                      [](size_t partial, const std::string_view& view) { return partial + view.size(); });

   std::vector<char> paragraph;
   paragraph.reserve(paragraphLength + m_paragraphAccumulator.size() + 1);
   std::for_each(
      m_paragraphAccumulator.cbegin(), m_paragraphAccumulator.cend(), [&paragraph](const std::string_view& view) {
         paragraph.insert(paragraph.end(), view.cbegin(), view.cend());
         paragraph.push_back(' ');
      });

   paragraph.back() = '\0';
   m_paragraphs.push_back(paragraph);

   m_paragraphAccumulator.clear();
}
