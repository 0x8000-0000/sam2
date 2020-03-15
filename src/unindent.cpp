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

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <optional>
#include <ostream>
#include <sstream>
#include <vector>

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

      explicit Accumulator(std::ostream& out) : output{out}, spaces(/* __n = */ k_MaxIndent, /* __v = */ ' ')
      {
         indents.reserve(k_MaxIndent);
      }

      std::ostream&       output;
      std::vector<size_t> indents;
      size_t              currentIndent    = 0;
      bool                lastLineWasEmpty = false;

      std::optional<std::string> pushLine(size_t indent, const char* base, size_t length);

      static constexpr std::array<char, 3> k_indentMarker   = {'{', '{', '\n'};
      static constexpr std::array<char, 3> k_deindentMarker = {'}', '}', '\n'};

      std::vector<char> spaces;
   };

   void pushLine(size_t lineNumber, size_t indent, const char* base, size_t length);

   Accumulator m_accumulator;
};

std::optional<std::string> Normalizer::Accumulator::pushLine(size_t indent, const char* base, size_t length)
{
   if (length == 0)
   {
      if (!lastLineWasEmpty)
      {
         output.put('\n');
      }

      lastLineWasEmpty = true;

      return std::nullopt;
   }

   lastLineWasEmpty = false;

   if (indent == currentIndent)
   {
      // continue
   }
   else if (indent > currentIndent)
   {
      indents.push_back(currentIndent);
      currentIndent = indent;

      output.write(k_indentMarker.data(), k_indentMarker.size());
   }
   else
   {
      if (indents.empty())
      {
         std::ostringstream error;
         error << "No previous indent level; current level: " << currentIndent << " observed: " << indent;
         return std::optional<std::string>(error.str());
      }

      std::ostringstream detail;
      detail << "This indent: " << indent << " current: " << currentIndent << " history: ";
      std::ostream_iterator<int> outIter{detail, ", "};
      std::copy(indents.cbegin(), indents.cend(), outIter);

      if (indent == indents.back())
      {
         // shortcut; most indents go back just one level
         currentIndent = indent;
         output.write(k_indentMarker.data(), k_indentMarker.size());
         indents.pop_back();
      }
      else
      {
         const auto iter = std::lower_bound(indents.cbegin(), indents.cend(), indent);
         assert(iter != indents.cbegin());

         if (iter == indents.cend())
         {
            std::cerr << detail.str() << '\n';

            std::ostringstream error;
            error << "Excessive indent; current level: " << currentIndent << " observed: " << indent;
            return std::optional<std::string>(error.str());
         }

         if (*iter != indent)
         {
            std::cerr << detail.str() << '\n';

            std::ostringstream error;
            error << "Excessive indent; current level: " << currentIndent << " observed: " << indent
                  << " expected: " << *iter;
            return std::optional<std::string>(error.str());
         }

         const auto deindentLevels = std::distance(iter, indents.cend());

         for (ssize_t ii = 0; ii < deindentLevels; ++ii)
         {
            output.write(k_deindentMarker.data(), k_deindentMarker.size());
         }

         indents.erase(iter, indents.end());
         assert(!indents.empty());

         currentIndent = indent;
      }
   }

   auto reindent = k_Indent * indents.size();
   if (reindent > k_MaxIndent)
   {
      reindent = k_MaxIndent;
   }

   output.write(spaces.data(), static_cast<std::streamsize>(reindent));

   output.write(base, static_cast<std::streamsize>(length));
   output.put('\n');

   return std::nullopt;
}

void Normalizer::pushLine(size_t lineNumber, size_t indent, const char* base, size_t length)
{
   const auto err = m_accumulator.pushLine(indent, base, length);
   if (err)
   {
      std::cerr << "Error on line " << lineNumber << ": " << err.value() << '\n';
   }
}

size_t Normalizer::normalize(std::istream& input)
{
   std::vector<char> buffer(/* __n = */ k_BufferSize, /* __v = */ '\0');

   std::size_t count = 0;

   size_t lastPartialLineLength = 0;

   size_t lineNumber = 1;

   while (!input.eof())
   {
      /*
       * fill buffer
       */

      input.read(std::next(buffer.data(), static_cast<ptrdiff_t>(lastPartialLineLength)),
                 static_cast<std::streamsize>(k_BufferSize - lastPartialLineLength));
      lastPartialLineLength = 0;
      const auto ssize      = input.gcount();
      if (ssize <= 0)
      {
         // TODO(florin): indicate error?
         break;
      }

      const auto size = static_cast<size_t>(ssize);

      const bool lastBuffer = size < (k_BufferSize - lastPartialLineLength);
      count += size;

      /*
       * process buffer
       */
      std::size_t pos = 0;

      while (pos < size)
      {
         unsigned indent = 0;

         /*
          * determine the line indent
          */
         while ((pos < size) && (buffer[pos] == ' '))
         {
            ++pos;
            ++indent;
         }

         if (pos == size)
         {
            if (lastBuffer)
            {
               // empty line on last buffer
               pushLine(lineNumber, indent, nullptr, 0);
            }
            else
            {
               // this line might continue in the next buffer
               std::fill_n(buffer.begin(), indent, ' ');
               lastPartialLineLength = indent;
            }

            continue;
         }

         const auto lineStart = pos;
         while ((pos < size) && (buffer[pos] != '\n'))
         {
            ++pos;
         }

         if (pos == size)
         {
            if (lastBuffer)
            {
               pushLine(lineNumber, indent, &buffer[lineStart], pos - lineStart);
            }
            else
            {
               /*
                * this line might continue in the next buffer
                */

               // copy indent
               std::fill_n(buffer.begin(), indent, ' ');
               // copy partial line
               std::copy_n(std::next(buffer.begin(), indent),
                           pos - lineStart,
                           std::next(buffer.begin(), static_cast<ptrdiff_t>(lineStart)));
               lastPartialLineLength = indent + static_cast<size_t>(pos - lineStart);
            }

            continue;
         }

         pushLine(lineNumber, indent, &buffer[lineStart], pos - lineStart);

         // skip over the new line
         ++pos;
         ++lineNumber;
      }
   }

   return count;
}

int main(int argc, char* argv[])
{
   if (argc < 2)
   {
      std::cerr << "Error: input / output arguments missing\n";
      return 1;
   }

   std::ifstream input(argv[1]);
   std::ofstream output(argv[2]);

   Normalizer normalizer{output};

   const auto size = normalizer.normalize(input);
   std::cerr << "Processed " << size << " bytes\n";

   return 0;
}
