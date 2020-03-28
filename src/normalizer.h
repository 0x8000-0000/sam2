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

#ifndef SAMX_NORMALIZER_H_INCLUDED
#define SAMX_NORMALIZER_H_INCLUDED

#include <array>
#include <iosfwd>
#include <optional>
#include <vector>

namespace samx
{

class Normalizer
{
public:
   explicit Normalizer(std::ostream& output) : m_accumulator{output}
   {
   }

   size_t normalize(std::istream& input);

private:
   static constexpr size_t k_BufferSize = 64 * 1024;
   static constexpr size_t k_MaxIndent  = 1024;

   struct Accumulator
   {
      static constexpr size_t k_Indent = 4;

      explicit Accumulator(std::ostream& out) : output{out}, spaces(/* __n = */ k_MaxIndent, /* __value = */ ' ')
      {
         indents.reserve(k_MaxIndent);
      }

      std::ostream&       output;
      std::vector<size_t> indents;
      size_t              currentIndent    = 0;
      bool                lastLineWasEmpty = false;

      std::optional<std::string> pushLine(size_t indent, const char* base, size_t length);
      void flush();

      static constexpr std::array<char, 3> k_indentMarker   = {'{', '{', '\n'};
      static constexpr std::array<char, 3> k_deindentMarker = {'}', '}', '\n'};

      std::vector<char> spaces;
   };

   void pushLine(size_t lineNumber, size_t indent, const char* base, size_t length);

   Accumulator m_accumulator;
};

} // namespace samx

#endif // SAMX_NORMALIZER_H_INCLUDED
