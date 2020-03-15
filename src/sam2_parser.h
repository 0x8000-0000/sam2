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

#ifndef SAM2_PARSER_H_INCLUDED
#define SAM2_PARSER_H_INCLUDED

#include <string_view>
#include <vector>

namespace sam2
{
class Block
{
};

class Document
{
public:
   size_t getBlockCount() const noexcept
   {
      return m_blocks.size();
   }

   void pushText(std::string_view segment)
   {
      m_paragraphAccumulator.push_back(segment);
   }

   void pushParagraph();

private:
   std::vector<Block> m_blocks;

   std::vector<std::string_view> m_paragraphAccumulator;

   std::vector<std::vector<char>> m_paragraphs;
};

Document parse(std::string_view input);
} // namespace sam2

#endif // SAM2_PARSER_H_INCLUDED
