/*
   Copyright 2019 Florin Iucha

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
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <stack>
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

   struct Accumulator
   {
      explicit Accumulator(std::ostream& out) : output{out}
      {
      }

      std::ostream&        output;
      std::stack<unsigned> indents;
      unsigned             previousIndent = 0;

      void pushLine(size_t indent, const char* base, size_t length);
   };

   Accumulator m_accumulator;
};

void Normalizer::Accumulator::pushLine(size_t /* indent */, const char* base, size_t length)
{
   output.write(base, static_cast<std::streamsize>(length));
   output.put('\n');
}

size_t Normalizer::normalize(std::istream& input)
{
   std::vector<char> buffer(k_BufferSize, '\0');

   std::size_t count = 0;

   size_t lastPartialLineLength = 0;

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
               m_accumulator.pushLine(indent, nullptr, 0);
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
               m_accumulator.pushLine(indent, &buffer[lineStart], pos - lineStart);
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

         m_accumulator.pushLine(indent, &buffer[lineStart], pos - lineStart);

         // skip over the new line
         ++pos;
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
