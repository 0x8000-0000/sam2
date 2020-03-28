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

#include "normalizer.h"

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <sstream>

std::optional<std::string> samx::Normalizer::Accumulator::pushLine(size_t indent, const char* base, size_t length)
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
         return fmt::format("No previous indent level; current level: {}; observed: {}", currentIndent, indent);
      }

      std::ostringstream detail;
      detail << "This indent: " << indent << " current: " << currentIndent << " history: ";
      std::ostream_iterator<int> outIter{detail, ", "};
      std::copy(indents.cbegin(), indents.cend(), outIter);

      if (indent == indents.back())
      {
         // shortcut; most indents go back just one level
         currentIndent = indent;
         output.write(k_deindentMarker.data(), k_deindentMarker.size());
         indents.pop_back();
      }
      else
      {
         const auto iter = std::lower_bound(indents.cbegin(), indents.cend(), indent);
         assert(iter != indents.cbegin());

         if (iter == indents.cend())
         {
            std::cerr << detail.str() << '\n';

            return fmt::format("Excessive de-indent; current level: {}; observed: {}", currentIndent, indent);
         }

         if (*iter != indent)
         {
            return fmt::format(
               "Excessive de-indent; current level: {}; observed: {}; expected: {}", currentIndent, indent, *iter);
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

void samx::Normalizer::Accumulator::flush()
{
   pushLine(0, nullptr, 0);

   const auto deindent = indents.size();
   for (size_t ii = 0; ii < deindent; ++ii)
   {
      output.write(k_deindentMarker.data(), k_deindentMarker.size());
   }
}

void samx::Normalizer::pushLine(size_t lineNumber, size_t indent, const char* base, size_t length)
{
   const auto err = m_accumulator.pushLine(indent, base, length);
   if (err)
   {
      std::cerr << "Error on line " << lineNumber << ": " << err.value() << '\n';
   }
}

size_t samx::Normalizer::normalize(std::istream& input)
{
   std::vector<char> buffer(/* __n = */ k_BufferSize, /* __value = */ '\0');

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

   m_accumulator.flush();

   return count;
}
