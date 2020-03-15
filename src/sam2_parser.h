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

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace sam2
{
class Block
{
public:
   Block(std::string_view type, const std::vector<std::string_view>& description) : m_type{type}
   {
      if (!description.empty())
      {
         m_description = std::string(description.back());
      }
   }

private:
   std::string m_type;
   std::string m_description;
};

class Paragraph
{
public:
   explicit Paragraph(const std::vector<std::string_view>& segments);

   Paragraph(const Paragraph& other) = default;
   Paragraph(Paragraph&& other)      = default;
   ~Paragraph()                      = default;
   Paragraph& operator=(const Paragraph& other) = default;
   Paragraph& operator=(Paragraph&& other) = default;

private:
   std::vector<char> m_text;
};

class Document
{
public:
   size_t getBlockCount() const noexcept
   {
      return m_elements.size();
   }

   void pushText(std::string_view segment)
   {
      m_textAccumulator.push_back(segment);
   }

   void pushParagraph();

   void observeIdentifier(std::string_view segment)
   {
      m_currentIdentifier = segment;
   }

   void pushBlock();

private:
   using Element = std::variant<Block, Paragraph>;

   std::vector<std::string_view> m_textAccumulator;

   std::vector<std::vector<char>> m_paragraphs;

   std::string_view m_currentIdentifier;

   std::vector<Element> m_elements;
};

Document parse(std::string_view input);
} // namespace sam2

#endif // SAM2_PARSER_H_INCLUDED
