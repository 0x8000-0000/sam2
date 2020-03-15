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

class Paragraph;

class Block
{
public:
   using Element = std::variant<Block, Paragraph>;

   Block(std::string&& type, std::string&& description, std::vector<Element>&& elements)
   {
      std::swap(m_type, type);
      std::swap(m_description, description);
      std::swap(m_elements, elements);
   }

   void setContents() noexcept
   {
   }

   const std::string& getType() const noexcept
   {
      return m_type;
   }

   const std::string& getDescription() const noexcept
   {
      return m_description;
   }

private:
   std::string          m_type;
   std::string          m_description;
   std::vector<Element> m_elements;
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

   void observeDescription(std::string_view segment)
   {
      m_currentDescription = segment;
   }

   void pushBlock();

   void startBlock() {}
   void endBlock() {}

private:
   std::vector<std::string_view> m_textAccumulator;

   std::string m_currentIdentifier;
   std::string m_currentDescription;

   std::vector<Block::Element> m_elements;
};

Document parse(std::string_view input);
} // namespace sam2

#endif // SAM2_PARSER_H_INCLUDED
