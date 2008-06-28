#include "exp.h"
#include "exceptions.h"

namespace YAML
{
	namespace Exp
	{
		unsigned ParseHex(std::string str)
		{
			unsigned value = 0;
			for(unsigned i=0;i<str.size();i++) {
				char ch = str[i];
				int digit = 0;
				if('a' <= ch && ch <= 'f')
					digit = ch - 'a' + 10;
				else if('A' <= ch && ch <= 'F')
					digit = ch - 'A' + 10;
				else if('0' <= ch && ch <= '9')
					digit = ch - '0';
				else
					throw NonHexNumber(ch);

				value = (value << 4) + digit;
			}

			return value;
		}

		std::string Str(char ch)
		{
			return std::string("") + ch;
		}

		// Escape
		// . Translates the next 'codeLength' characters into a hex number and returns the result.
		// . Throws if it's not actually hex.
		std::string Escape(std::istream& in, int& length, int codeLength)
		{
			// grab string
			length += codeLength;
			std::string str;
			for(int i=0;i<codeLength;i++)
				str += in.get();

			// get the value
			unsigned value = ParseHex(str);

			// legal unicode?
			if((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF)
				throw InvalidUnicode(value);

			// now break it up into chars
			if(value <= 0x7F)
				return Str(value);
			else if(value <= 0x7FF)
				return Str(0xC0 + (value >> 6)) + Str(0x80 + (value & 0x3F));
			else if(value <= 0xFFFF)
				return Str(0xE0 + (value >> 12)) + Str(0x80 + ((value >> 6) & 0x3F)) + Str(0x80 + (value & 0x3F));
			else
				return Str(0xF0 + (value >> 18)) + Str(0x80 + ((value >> 12) & 0x3F)) +
					Str(0x80 + ((value >> 6) & 0x3F)) + Str(0x80 + (value & 0x3F));
		}

		// Escape
		// . Escapes the sequence starting 'in' (it must begin with a '\')
		//   and returns the result.
		// . Fills 'length' with how many characters we ate.
		// . Throws if it's an unknown escape character.
		std::string Escape(std::istream& in, int& length)
		{
			// slash + character
			length = 2;

			// eat slash
			in.get();

			// switch on escape character
			char ch = in.get();
			switch(ch) {
				case '0': return "\0";
				case 'a': return "\x07";
				case 'b': return "\x08";
				case 't':
                case '\t': return "\x09";
				case 'n': return "\x0A";
				case 'v': return "\x0B";
				case 'f': return "\x0C";
				case 'r': return "\x0D";
				case 'e': return "\x1B";
				case ' ': return "\x20";
				case '\"': return "\"";
				case '\'': return "\'";
				case '\\': return "\\";
				case 'N': return "\xC2\x85";  // NEL (#x85)
				case '_': return "\xC2\xA0";  // #xA0
				case 'L': return "\xE2\x80\xA8";  // LS (#x2028)
				case 'P': return "\xE2\x80\xA9";  // PS (#x2029)
				case 'x': return Escape(in, length, 2);
				case 'u': return Escape(in, length, 4);
				case 'U': return Escape(in, length, 8);
			}

			throw UnknownEscapeSequence(ch);
		}
	}
}