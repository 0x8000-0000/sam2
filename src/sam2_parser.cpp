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
#include "tao/pegtl/ascii.hpp"

#include <tao/pegtl.hpp>

#include <fmt/core.h>

#include <iostream>

namespace
{

namespace pegtl = tao::pegtl;

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

struct Paragraph : pegtl::seq<pegtl::plus<pegtl::seq<Text, NewLine>>, NewLine>
{
};

struct Block;

struct BlockContent : pegtl::seq<BlockStart, pegtl::star<pegtl::sor<Block, Paragraph>>, BlockEnd>
{
};

struct Block : pegtl::seq<pegtl::identifier, pegtl::one<':'>, pegtl::opt<Text>, NewLine, pegtl::opt<BlockContent>>
{
};

struct Identifier : pegtl::seq<pegtl::identifier>
{
};

struct BlockInsertion : pegtl::seq<InsertionMarker, Identifier>
{
};

struct Grammar : pegtl::seq<pegtl::star<Paragraph>, pegtl::eof>
{
};

template <typename Rule>
struct Action
{
};

template <>
struct Action<Text>
{
   template <typename Input>
   static void apply(const Input& in, sam2::Document& /* doc */)
   {
      std::cerr << "Text: " << in.string() << '\n';
   }
};


template <>
struct Action<Paragraph>
{
   template <typename Input>
   static void apply(const Input& in, sam2::Document& /* doc */)
   {
      std::cerr << "Paragraph: " << in.string() << '\n';
   }
};

} // namespace

sam2::Document sam2::parse(std::string_view input)
{
   sam2::Document doc;

   pegtl::memory_input in(input.data(), input.size(), "");

   try
   {
      pegtl::parse<Grammar, Action>(in, doc);
   }
   catch (const tao::pegtl::parse_error& parseError)
   {
      throw std::runtime_error(fmt::format("Failed to parse input: {}", parseError.std::exception::what()));
   }

   return doc;
}
