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

// #define MANUAL_TRACE

#include "sam2_parser.h"

#include <tao/pegtl.hpp>
#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/contrib/tracer.hpp>

#include <fmt/core.h>

#include <iostream>

namespace
{

namespace pegtl = tao::pegtl;

using WhiteSpace = pegtl::star<pegtl::one<' '>>;

using NewLine = pegtl::ascii::eol;

struct InsertionMarker : pegtl::rep<3, pegtl::one<'<'>>
{
};

struct BlockStart : pegtl::seq<pegtl::rep<2, pegtl::one<'{'>>, pegtl::opt<NewLine>>
{
};

struct BlockEnd : pegtl::seq<pegtl::rep<2, pegtl::one<'}'>>, pegtl::opt<NewLine>>
{
};

struct Text : pegtl::plus<pegtl::sor<pegtl::ascii::alnum, pegtl::range<0x2b, 0x2f>, pegtl::one<' '>>>
{
};

struct ParagraphText: Text
{
};

struct Paragraph : pegtl::seq<WhiteSpace, pegtl::plus<pegtl::seq<ParagraphText, WhiteSpace, NewLine>>, NewLine>
{
};

struct Block;

struct Content : pegtl::star<pegtl::sor<Block, Paragraph>>
{
};

struct IndentedBlock : pegtl::seq<BlockStart, Content, BlockEnd>
{
};

struct BlockIdentifier : pegtl::seq<pegtl::identifier, pegtl::one<':'>>
{
};

struct BlockDescription : pegtl::opt<Text>
{
};

struct Block : pegtl::seq<WhiteSpace,
                          BlockIdentifier,
                          WhiteSpace,
                          BlockDescription,
                          WhiteSpace,
                          pegtl::plus<NewLine>,
                          pegtl::opt<IndentedBlock>>
{
};

struct ExternalResource : pegtl::seq<pegtl::identifier>
{
};

struct BlockInsertion : pegtl::seq<InsertionMarker, ExternalResource>
{
};

struct Grammar : pegtl::seq<Content, pegtl::eof>
{
};

template <typename Rule>
struct Action
{
};

template <>
struct Action<ParagraphText>
{
   template <typename Input>
   static void apply(const Input& in, sam2::Document& doc)
   {
#ifdef MANUAL_TRACE
      std::cerr << "Text: " << in.string_view() << '\n';
#else
      (void)in;
#endif
      doc.pushText(in.string_view());
   }
};

template <>
struct Action<Paragraph>
{
   template <typename Input>
   static void apply(const Input& /* in */, sam2::Document& doc)
   {
      doc.pushParagraph();
   }
};

template <>
struct Action<BlockIdentifier>
{
   template <typename Input>
   static void apply(const Input& in, sam2::Document& doc)
   {
      doc.observeIdentifier(in.string_view());
   }
};

template <>
struct Action<BlockDescription>
{
   template <typename Input>
   static void apply(const Input& in, sam2::Document& doc)
   {
      doc.observeDescription(in.string_view());
   }
};

template <>
struct Action<BlockStart>
{
   static void apply0(sam2::Document& doc)
   {
#ifdef MANUAL_TRACE
      std::cerr << "BlockStart\n";
#endif
      doc.startBlock();
   }
};

template <>
struct Action<Block>
{
   template <typename Input>
   static void apply(const Input& in, sam2::Document& doc)
   {
#ifdef MANUAL_TRACE
      std::cerr << "Block: " << in.size() << '\n';
#else
      (void)in;
#endif
      doc.finishBlock();
   }
};

template <>
struct Action<Content>
{
   template <typename Input>
   static void apply(const Input& in, sam2::Document& /* doc */)
   {
#ifdef MANUAL_TRACE
      std::cerr << "Content: " << in.size() << '\n';
#else
      (void)in;
#endif
   }
};

} // namespace

sam2::Document sam2::parse(std::string_view input)
{
   sam2::Document doc;

   pegtl::memory_input in(input.data(), input.size(), "");

   try
   {
      auto result = pegtl::parse<Grammar, Action /*, pegtl::tracer */>(in, doc);
      if (result)
      {
         std::cerr << "Parse succeeded!\n";
      }
      else
      {
         std::cerr << "Parse failed\n";
      }
   }
   catch (const tao::pegtl::parse_error& parseError)
   {
      throw std::runtime_error(fmt::format("Failed to parse input: {}", parseError.std::exception::what()));
   }
   catch (...)
   {
      std::cerr << "Unexpected error" << std::endl;
      throw std::runtime_error("Unexpected error");
   }

   return doc;
}
