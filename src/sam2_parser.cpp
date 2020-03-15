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

#include <tao/pegtl.hpp>

#include <fmt/core.h>

namespace
{

namespace pegtl = tao::pegtl;

struct block : pegtl::plus<pegtl::ascii::space>
{
};

struct grammar : pegtl::plus<block>
{
};

template <typename Rule>
struct action
{
};

} // namespace

sam2::Document sam2::parse(std::string_view input)
{
   sam2::Document doc;

   pegtl::memory_input in(input.data(), input.size(), "");

   try
   {
      pegtl::parse<grammar, action>(in, doc);
   }
   catch (const tao::pegtl::parse_error& parseError)
   {
      throw std::runtime_error(fmt::format("Failed to parse input: {}", parseError.std::exception::what()));
   }

   return doc;
}
