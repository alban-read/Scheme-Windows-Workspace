#include "stdafx.h"
#include "scheme.h"
#include <gdiplus.h>
#include <string>

#define WinGetObject GetObjectW
#define WinSendMessage SendMessageW


#define CALL0(who) Scall0(Stop_level_value(Sstring_to_symbol(who)))
#define CALL1(who, arg) Scall1(Stop_level_value(Sstring_to_symbol(who)), arg)
#define CALL2(who, arg, arg2) Scall2(Stop_level_value(Sstring_to_symbol(who)), arg, arg2)


namespace Text
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Text conversions
	// Text (UTF8) processing functions.


	typedef unsigned int UTF32;
	typedef unsigned short UTF16;
	typedef unsigned char UTF8;
	typedef unsigned char Boolean;

	static const unsigned char total_bytes[256] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6
	};

	static const char trailing_bytes_for_utf8[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
	};

	static const UTF8 first_byte_mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

	static const UTF32 offsets_from_utf8[6] = {
		0x00000000UL, 0x00003080UL, 0x000E2080UL,
		0x03C82080UL, 0xFA082080UL, 0x82082080UL
	};

	typedef enum
	{
		conversionOK,
		/* conversion successful */
		sourceExhausted,
		/* partial character in source, but hit end */
		targetExhausted,
		/* insuff. room in target for conversion */
		sourceIllegal /* source sequence is illegal/malformed */
	} ConversionResult;

	typedef enum
	{
		strictConversion = 0,
		lenientConversion
	} conversion_flags;

	/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF

	static const int halfShift = 10;
	static const UTF32 halfBase = 0x0010000UL;
	static const UTF32 halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START (UTF32)0xD800
#define UNI_SUR_HIGH_END (UTF32)0xDBFF
#define UNI_SUR_LOW_START (UTF32)0xDC00
#define UNI_SUR_LOW_END (UTF32)0xDFFF
#define false 0
#define true 1

	/* is c the start of a utf8 sequence? */
#define ISUTF8(c) (((c)&0xC0) != 0x80)
#define ISASCII(ch) ((unsigned char)ch < 0x80)

#define UTFmax 4
	typedef char32_t Rune;
#define RUNE_C(x) x##L
#define Runeself 0x80
#define Runemax RUNE_C(0x10FFFF)

	int runelen(Rune r)
	{
		if (r <= 0x7F)
			return 1;
		else if (r <= 0x07FF)
			return 2;
		else if (r <= 0xD7FF)
			return 3;
		else if (r <= 0xDFFF)
			return 0; /* surrogate character */
		else if (r <= 0xFFFD)
			return 3;
		else if (r <= 0xFFFF)
			return 0; /* illegal character */
		else if (r <= Runemax)
			return 4;
		else
			return 0; /* rune too large */
	}


	int strlen_utf8(const char* s)
	{
		auto i = 0, j = 0;
		while (s[i])
		{
			if ((s[i] & 0xc0) != 0x80) j++;
			i++;
		}
		return j;
	}


	int runetochar(char* s, const Rune* p)
	{
		const auto r = *p;

		switch (runelen(r)) {
		case 1: /* 0aaaaaaa */
			s[0] = r;
			return 1;
		case 2: /* 00000aaa aabbbbbb */
			s[0] = 0xC0 | ((r & 0x0007C0) >> 6); /* 110aaaaa */
			s[1] = 0x80 | (r & 0x00003F);        /* 10bbbbbb */
			return 2;
		case 3: /* aaaabbbb bbcccccc */
			s[0] = 0xE0 | ((r & 0x00F000) >> 12); /* 1110aaaa */
			s[1] = 0x80 | ((r & 0x000FC0) >> 6); /* 10bbbbbb */
			s[2] = 0x80 | (r & 0x00003F);        /* 10cccccc */
			return 3;
		case 4: /* 000aaabb bbbbcccc ccdddddd */
			s[0] = 0xF0 | ((r & 0x1C0000) >> 18); /* 11110aaa */
			s[1] = 0x80 | ((r & 0x03F000) >> 12); /* 10bbbbbb */
			s[2] = 0x80 | ((r & 0x000FC0) >> 6); /* 10cccccc */
			s[3] = 0x80 | (r & 0x00003F);        /* 10dddddd */
			return 4;
		default:
			return 0; /* error */
		}
	}
	// some functions from unicode.inc

	ConversionResult ConvertUTF16toUTF8(
		const UTF16** sourceStart, const UTF16* sourceEnd,
		UTF8** targetStart, UTF8* targetEnd, conversion_flags flags)
	{
		ConversionResult result = conversionOK;
		const UTF16* source = *sourceStart;
		UTF8* target = *targetStart;
		while (source < sourceEnd)
		{
			UTF32 ch;
			unsigned short bytesToWrite = 0;
			const UTF32 byteMask = 0xBF;
			const UTF32 byteMark = 0x80;
			const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
			ch = *source++;

			/* If we have a surrogate pair, convert to UTF32 first. */
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END)
			{
				/* If the 16 bits following the high surrogate are in the source buffer... */
				if (source < sourceEnd)
				{
					UTF32 ch2 = *source;
					/* If it's a low surrogate, convert to UTF32. */
					if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END)
					{
						ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
						++source;
					}
					else if (flags == strictConversion)
					{
						/* it's an unpaired high surrogate */
						--source; /* return to the illegal value itself */
						result = sourceIllegal;
						break;
					}
				}
				else
				{
					/* We don't have the 16 bits following the high surrogate. */
					--source; /* return to the high surrogate */
					result = sourceExhausted;
					break;
				}
			}
			else if (flags == strictConversion)
			{
				/* UTF-16 surrogate values are illegal in UTF-32 */
				if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END)
				{
					--source; /* return to the illegal value itself */
					result = sourceIllegal;
					break;
				}
			}
			/* Figure out how many bytes the result will require */
			if (ch < static_cast<UTF32>(0x80))
			{
				bytesToWrite = 1;
			}
			else if (ch < static_cast<UTF32>(0x800))
			{
				bytesToWrite = 2;
			}
			else if (ch < static_cast<UTF32>(0x10000))
			{
				bytesToWrite = 3;
			}
			else if (ch < static_cast<UTF32>(0x110000))
			{
				bytesToWrite = 4;
			}
			else
			{
				bytesToWrite = 3;
				ch = UNI_REPLACEMENT_CHAR;
			}

			target += bytesToWrite;
			if (target > targetEnd)
			{
				source = oldSource; /* Back up source pointer! */
				target -= bytesToWrite;
				result = targetExhausted;
				break;
			}
			switch (bytesToWrite)
			{
				/* note: everything falls through. */
			case 4:
				*--target = static_cast<UTF8>((ch | byteMark) & byteMask);
				ch >>= 6;
			case 3:
				*--target = static_cast<UTF8>((ch | byteMark) & byteMask);
				ch >>= 6;
			case 2:
				*--target = static_cast<UTF8>((ch | byteMark) & byteMask);
				ch >>= 6;
			case 1:
				*--target = static_cast<UTF8>(ch | first_byte_mark[bytesToWrite]);
			}
			target += bytesToWrite;
		}
		*sourceStart = source;
		*targetStart = target;
		return result;
	}

	static Boolean is_legal_utf8(const UTF8* source, int length)
	{
		UTF8 a;
		const UTF8* srcptr = source + length;
		switch (length)
		{
		default:
			return false;
			/* Everything else falls through when "true"... */
		case 4:
			if ((a = (*--srcptr)) < 0x80 || a > 0xBF)
				return false;
		case 3:
			if ((a = (*--srcptr)) < 0x80 || a > 0xBF)
				return false;
		case 2:
			if ((a = (*--srcptr)) > 0xBF)
				return false;

			switch (*source)
			{
				/* no fall-through in this inner switch */
			case 0xE0:
				if (a < 0xA0)
					return false;
				break;
			case 0xED:
				if (a > 0x9F)
					return false;
				break;
			case 0xF0:
				if (a < 0x90)
					return false;
				break;
			case 0xF4:
				if (a > 0x8F)
					return false;
				break;
			default:
				if (a < 0x80)
					return false;
			}
		case 1:
			if (*source >= 0x80 && *source < 0xC2)
				return false;
		}
		if (*source > 0xF4)
			return false;
		return true;
	}

	Boolean is_legal_utf8_sequence(const UTF8* source, const UTF8* sourceEnd)
	{
		const int length = trailing_bytes_for_utf8[*source] + 1;
		if (source + length > sourceEnd)
		{
			return false;
		}
		return is_legal_utf8(source, length);
	}

	ConversionResult ConvertUTF8toUTF16(
		const UTF8** sourceStart, const UTF8* sourceEnd,
		UTF16** targetStart, const UTF16* targetEnd, conversion_flags flags);

	ConversionResult ConvertUTF32toUTF8(
		const UTF32** sourceStart, const UTF32* sourceEnd,
		UTF8** targetStart, UTF8* targetEnd, conversion_flags flags)
	{
		ConversionResult result = conversionOK;
		const UTF32* source = *sourceStart;
		UTF8* target = *targetStart;
		while (source < sourceEnd)
		{
			UTF32 ch;
			unsigned short bytesToWrite = 0;
			const UTF32 byteMask = 0xBF;
			const UTF32 byteMark = 0x80;
			ch = *source++;
			if (flags == strictConversion)
			{
				/* UTF-16 surrogate values are illegal in UTF-32 */
				if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)
				{
					--source; /* return to the illegal value itself */
					result = sourceIllegal;
					break;
				}
			}
			/*
			* Figure out how many bytes the result will require. Turn any
			* illegally large UTF32 things (> Plane 17) into replacement chars.
			*/
			if (ch < static_cast<UTF32>(0x80))
			{
				bytesToWrite = 1;
			}
			else if (ch < static_cast<UTF32>(0x800))
			{
				bytesToWrite = 2;
			}
			else if (ch < static_cast<UTF32>(0x10000))
			{
				bytesToWrite = 3;
			}
			else if (ch <= UNI_MAX_LEGAL_UTF32)
			{
				bytesToWrite = 4;
			}
			else
			{
				bytesToWrite = 3;
				ch = UNI_REPLACEMENT_CHAR;
				result = sourceIllegal;
			}
			target += bytesToWrite;
			if (target > targetEnd)
			{
				--source; /* Back up source pointer! */
				target -= bytesToWrite;
				result = targetExhausted;
				break;
			}
			switch (bytesToWrite)
			{
				/* note: everything falls through. */
			case 4:
				*--target = static_cast<UTF8>((ch | byteMark) & byteMask);
				ch >>= 6;
			case 3:
				*--target = static_cast<UTF8>((ch | byteMark) & byteMask);
				ch >>= 6;
			case 2:
				*--target = static_cast<UTF8>((ch | byteMark) & byteMask);
				ch >>= 6;
			case 1:
				*--target = static_cast<UTF8>(ch | first_byte_mark[bytesToWrite]);
			}
			target += bytesToWrite;
		}
		*sourceStart = source;
		*targetStart = target;
		return result;
	}

	ConversionResult ConvertUTF8toUTF32(
		const UTF8** source_start, const UTF8* source_end,
		UTF32** target_start, UTF32* target_end, conversion_flags flags)
	{
		ConversionResult result = conversionOK;
		const UTF8* source = *source_start;
		auto target = *target_start;
		while (source < source_end)
		{
			UTF32 ch = 0;
			const unsigned short extra_bytes_to_read = trailing_bytes_for_utf8[*source];
			if (source + extra_bytes_to_read >= source_end)
			{
				result = sourceExhausted;
				break;
			}
			/* Do this check whether lenient or strict */
			if (!is_legal_utf8(source, extra_bytes_to_read + 1))
			{
				result = sourceIllegal;
				break;
			}
			/*
			* The cases all fall through.
			*/
			switch (extra_bytes_to_read)
			{
			case 5:
				ch += *source++;
				ch <<= 6;
			case 4:
				ch += *source++;
				ch <<= 6;
			case 3:
				ch += *source++;
				ch <<= 6;
			case 2:
				ch += *source++;
				ch <<= 6;
			case 1:
				ch += *source++;
				ch <<= 6;
			case 0:
				ch += *source++;
			}
			ch -= offsets_from_utf8[extra_bytes_to_read];

			if (target >= target_end)
			{
				source -= (extra_bytes_to_read + 1); /* Back up the source pointer! */
				result = targetExhausted;
				break;
			}
			if (ch <= UNI_MAX_LEGAL_UTF32)
			{
				/*
				* UTF-16 surrogate values are illegal in UTF-32, and anything
				* over Plane 17 (> 0x10FFFF) is illegal.
				*/
				if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)
				{
					if (flags == strictConversion)
					{
						source -= (extra_bytes_to_read + 1); /* return to the illegal value itself */
						result = sourceIllegal;
						break;
					}
					else
					{
						*target++ = UNI_REPLACEMENT_CHAR;
					}
				}
				else
				{
					*target++ = ch;
				}
			}
			else
			{
				/* i.e., ch > UNI_MAX_LEGAL_UTF32 */
				result = sourceIllegal;
				*target++ = UNI_REPLACEMENT_CHAR;
			}
		}
		*source_start = source;
		*target_start = target;
		return result;
	}

	// Runes UTF8

	int rune_length(const Rune r)
	{
		if (r <= 0x7F)
			return 1;
		else if (r <= 0x07FF)
			return 2;
		else if (r <= 0xD7FF)
			return 3;
		else if (r <= 0xDFFF)
			return 0; /* surrogate character */
		else if (r <= 0xFFFD)
			return 3;
		else if (r <= 0xFFFF)
			return 0; /* illegal character */
		else if (r <= Runemax)
			return 4;
		else
			return 0; /* rune too large */
	}

	int char_from_rune(char* s, const Rune* p)
	{
		auto r = *p;

		switch (rune_length(r))
		{
		case 1: /* 0aaaaaaa */
			s[0] = r;
			return 1;
		case 2: /* 00000aaa aabbbbbb */
			s[0] = 0xC0 | ((r & 0x0007C0) >> 6); /* 110aaaaa */
			s[1] = 0x80 | (r & 0x00003F); /* 10bbbbbb */
			return 2;
		case 3: /* aaaabbbb bbcccccc */
			s[0] = 0xE0 | ((r & 0x00F000) >> 12); /* 1110aaaa */
			s[1] = 0x80 | ((r & 0x000FC0) >> 6); /* 10bbbbbb */
			s[2] = 0x80 | (r & 0x00003F); /* 10cccccc */
			return 3;
		case 4: /* 000aaabb bbbbcccc ccdddddd */
			s[0] = 0xF0 | ((r & 0x1C0000) >> 18); /* 11110aaa */
			s[1] = 0x80 | ((r & 0x03F000) >> 12); /* 10bbbbbb */
			s[2] = 0x80 | ((r & 0x000FC0) >> 6); /* 10cccccc */
			s[3] = 0x80 | (r & 0x00003F); /* 10dddddd */
			return 4;
		default:
			return 0; /* error */
		}
	}

	int utf8_string_length(const char* s)
	{
		auto i = 0, j = 0;
		while (s[i])
		{
			if ((s[i] & 0xc0) != 0x80)
				j++;
			i++;
		}
		return j;
	}

	// returns a list of UTF decoded strings e.g. from a line of a CSV file.
	// generally input data is UTF8.
	//
	ptr utf8_string_separated_to_list(char* s, const char sep)
	{
		ptr lst = Snil;

		unsigned int byte2;

		if (s == nullptr)
		{
			return Sstring("");
		}
		const auto ll = utf8_string_length(s);
		if (ll == 0)
		{
			return Sstring("");
		}

		const auto cps = static_cast<int*>(calloc(ll + 20, sizeof(int)));
		auto cpsptr = cps;

		const char* cptr = s;
		unsigned int byte = static_cast<unsigned char>(*cptr++);

		bool enquoted = false;
		auto quotes = 0;
		bool eol = false;

		while (byte != 0)
		{
			// handle separated field.
			if (byte == '"')
			{
				enquoted = !enquoted;
			}

			if (!enquoted && byte == sep)
			{
				// we are looking backwards now into our code point array
				auto run = (cpsptr - cps);
				auto jlen = run - 1;

				// reduce run length for cr/lf as we skip these.
				for (auto j = 0; j <= jlen; j++)
				{
					if (cps[j] == '\n' || cps[j] == '\r')
					{
						run--;
						eol = true;
					}
				}

				auto j = 0;

				if (cps[j] == '"')
				{
					j++;
					run--;
					run--;
				}

				// detect comma; eat spaces until quote
				if (cps[j] == sep)
				{
					j++;
					run--;
					while (cps[j] == ' ' && j < jlen)
					{
						j++;
						run--;
					}
				}

				if (cps[j] == '"')
				{
					j++;
					run--;
					run--;
				}

				if (run > 0)
				{
					ptr ss = Sstring_of_length("", run);
					for (auto k = 0; k < run; k++)
					{
						Sstring_set(ss, k, cps[j++]);
					}
					lst = CALL2("cons", ss, lst);
				}
				else if (!eol)
				{
					// null string - avoids turning \r into ""
					lst = CALL2("cons", Sstring(""), lst);
				}

				// reset cpsptr
				cpsptr = cps;
				memset(cps, '*', sizeof(int));
			}

			// ascii
			if (byte < 0x80)
			{
				*cpsptr++ = byte;
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			if (byte < 0xC0)
			{
				*cpsptr++ = byte;
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// skip
			while ((byte < 0xC0) && (byte >= 0x80))
			{
				byte = static_cast<unsigned char>(*cptr++);
			}

			if (byte < 0xE0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);

				if ((byte2 & 0xC0) == 0x80)
				{
					byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
					*cpsptr++ = byte;
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			if (byte < 0xF0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);
				if (byte2 == 0)
				{
					break;
				};
				const unsigned int byte3 = static_cast<unsigned char>(*cptr++);
				if (byte3 == 0)
				{
					break;
				};

				if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80))
				{
					byte = (((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					*cpsptr++ = byte;
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			auto trail = total_bytes[byte] - 1; // expected number of trail bytes
			if (trail > 0)
			{
				int ch = byte & (0x3F >> trail);
				do
				{
					byte2 = static_cast<unsigned char>(*cptr++);
					if ((byte2 & 0xC0) != 0x80)
					{
						*cpsptr++ = byte;
						byte = static_cast<unsigned char>(*cptr++);
						continue;
					}
					ch <<= 6;
					ch |= (byte2 & 0x3F);
					trail--;
				} while (trail > 0);
				*cpsptr++ = byte;
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}
			byte = static_cast<unsigned char>(*cptr++);
		}

		// one last record to check for.
		auto run = (cpsptr - cps);
		const auto jlen = run - 1;

		// reduce length for cr/lf as we skip these.

		for (auto j = 0; j <= jlen; j++)
		{
			if (cps[j] == '\n' || cps[j] == '\r')
			{
				run--;
				eol = true;
			}
		}

		auto j = 0;

		if (cps[j] == '"')
		{
			j++;
			run--;
			run--;
		}

		// detect comma; and eat spaces until quote
		if (cps[j] == sep)
		{
			j++;
			run--;
			while (cps[j] == ' ' && j < jlen)
			{
				j++;
				run--;
			}
		}

		if (cps[j] == '"')
		{
			j++;
			run--;
			run--;
		}

		if (run > 0)
		{
			auto ss = Sstring_of_length("", run);

			for (auto k = 0; k < run; k++)
			{
				Sstring_set(ss, k, cps[j++]);
			}

			lst = CALL2("cons", ss, lst);
		}
		else if (!eol)
		{
			// null string
			lst = CALL2("cons", Sstring(""), lst);
		}

		free(cps);
		return lst;
	}

	ptr UTF8toSstring(char* s)
	{
		unsigned int byte2;

		if (s == nullptr)
		{
			return Sstring("");
		}
		const auto ll = utf8_string_length(s);
		if (ll == 0)
		{
			return Sstring("");
		}

		const char* cptr = s;
		unsigned int byte = static_cast<unsigned char>(*cptr++);

		ptr ss = Sstring_of_length("", ll);

		auto i = 0;

		while (byte != 0 && i < ll)
		{
			// ascii
			if (byte < 0x80)
			{
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			if (byte < 0xC0)
			{
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// skip
			while ((byte < 0xC0) && (byte >= 0x80))
			{
				byte = static_cast<unsigned char>(*cptr++);
			}

			if (byte < 0xE0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);

				if ((byte2 & 0xC0) == 0x80)
				{
					byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			if (byte < 0xF0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);
				if (byte2 == 0)
				{
					break;
				};
				unsigned int byte3 = static_cast<unsigned char>(*cptr++);
				if (byte3 == 0)
				{
					break;
				};

				if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80))
				{
					byte = (((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			int trail = total_bytes[byte] - 1; // expected number of trail bytes
			if (trail > 0)
			{
				int ch = byte & (0x3F >> trail);
				do
				{
					byte2 = static_cast<unsigned char>(*cptr++);
					if ((byte2 & 0xC0) != 0x80)
					{
						Sstring_set(ss, i++, byte);
						byte = static_cast<unsigned char>(*cptr++);
						continue;
					}
					ch <<= 6;
					ch |= (byte2 & 0x3F);
					trail--;
				} while (trail > 0);
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// no match..
			if (i == ll)
			{
				break;
			}
			byte = static_cast<unsigned char>(*cptr++);
		}
		return ss;
	}

	ptr constUTF8toSstring(const char* s)
	{
		// With UTF8 we just have sequences of bytes in a buffer.
		// in scheme we use a single longer integer for a code point.
		// see https://github.com/Komodo/KomodoEdit/blob/master/src/SciMoz/nsSciMoz.cxx
		// passes the greek test.

		unsigned int byte2;

		if (s == nullptr)
		{
			return Sstring("");
		}
		const auto ll = utf8_string_length(s);
		if (ll == 0)
		{
			return Sstring("");
		}

		const char* cptr = s;
		unsigned int byte = static_cast<unsigned char>(*cptr++);

		ptr ss = Sstring_of_length("", ll);

		auto i = 0;

		while (byte != 0 && i < ll)
		{
			// ascii
			if (byte < 0x80)
			{
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			if (byte < 0xC0)
			{
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// skip
			while ((byte < 0xC0) && (byte >= 0x80))
			{
				byte = static_cast<unsigned char>(*cptr++);
			}

			if (byte < 0xE0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);

				if ((byte2 & 0xC0) == 0x80)
				{
					byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			if (byte < 0xF0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);
				const unsigned int byte3 = static_cast<unsigned char>(*cptr++);
				if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80))
				{
					byte = (((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			auto trail = total_bytes[byte] - 1; // expected number of trail bytes
			if (trail > 0)
			{
				int ch = byte & (0x3F >> trail);
				do
				{
					byte2 = static_cast<unsigned char>(*cptr++);
					if ((byte2 & 0xC0) != 0x80)
					{
						Sstring_set(ss, i++, byte);
						byte = static_cast<unsigned char>(*cptr++);
						continue;
					}
					ch <<= 6;
					ch |= (byte2 & 0x3F);
					trail--;
				} while (trail > 0);
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// no match..
			if (i == ll)
			{
				break;
			}
			byte = static_cast<unsigned char>(*cptr++);
		}

		return ss;
	}

	ptr constUTF8toSstringOfLength(const char* s, const int length)
	{
		// With UTF8 we just have sequences of bytes in a buffer.
		// in scheme we use a single longer integer for a code point.
		// see https://github.com/Komodo/KomodoEdit/blob/master/src/SciMoz/nsSciMoz.cxx
		// passes the greek test.

		unsigned int byte2;

		if (s == nullptr)
		{
			return Sstring("");
		}
		const auto ll = utf8_string_length(s);
		if (ll == 0)
		{
			return Sstring("");
		}

		const char* cptr = s;
		unsigned int byte = static_cast<unsigned char>(*cptr++);

		ptr ss = Sstring_of_length("", length);

		auto i = 0;

		while (byte != 0 && i < length)
		{
			// ascii
			if (byte < 0x80)
			{
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			if (byte < 0xC0)
			{
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// skip
			while ((byte < 0xC0) && (byte >= 0x80))
			{
				byte = static_cast<unsigned char>(*cptr++);
			}

			if (byte < 0xE0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);

				if ((byte2 & 0xC0) == 0x80)
				{
					byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			if (byte < 0xF0)
			{
				byte2 = static_cast<unsigned char>(*cptr++);
				const unsigned int byte3 = static_cast<unsigned char>(*cptr++);
				if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80))
				{
					byte = (((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			auto trail = total_bytes[byte] - 1; // expected number of trail bytes
			if (trail > 0)
			{
				int ch = byte & (0x3F >> trail);
				do
				{
					byte2 = static_cast<unsigned char>(*cptr++);
					if ((byte2 & 0xC0) != 0x80)
					{
						Sstring_set(ss, i++, byte);
						byte = static_cast<unsigned char>(*cptr++);
						continue;
					}
					ch <<= 6;
					ch |= (byte2 & 0x3F);
					trail--;
				} while (trail > 0);
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// no match..
			if (i == ll)
			{
				break;
			}
			byte = static_cast<unsigned char>(*cptr++);
		}
		return ss;
	}

	// use windows functions to widen string
	std::wstring Widen(const std::string& in)
	{
		// Calculate target buffer size (not including the zero terminator).
		const auto len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			in.c_str(), in.size(), NULL, 0);
		if (len == 0)
		{
			throw std::runtime_error("Invalid character sequence.");
		}

		std::wstring out(len, 0);
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			in.c_str(), in.size(), &out[0], out.size());
		return out;
	}

	// use windows functions to widen string
	std::wstring widen(const std::string& in)
	{
		// Calculate target buffer size (not including the zero terminator).
		const auto len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			in.c_str(), in.size(), NULL, 0);
		if (len == 0)
		{
			throw std::runtime_error("Invalid character sequence.");
		}

		std::wstring out(len, 0);
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			in.c_str(), in.size(), &out[0], out.size());
		return out;
	}
	// scheme string to c char*
	char* Sstring_to_charptr(ptr sparam)
	{
		ptr bytes = CALL1("string->utf8", sparam);
		const long len = Sbytevector_length(bytes);
		const auto data = Sbytevector_data(bytes);
		const auto text = static_cast<char*>(calloc(len + 1, sizeof(char)));
		memcpy(text, data, len);
		bytes = Snil;
		return text;
	}

	// scheme strings are wider UTF32 than windows wide UTF16 strings.
	// not sure if widestring code points span more than one
	// short int; if they do this is broken.

	ptr wideto_sstring(WCHAR* s)
	{
		if (s == nullptr)
		{
			return Sstring("");
		}
		const int len = wcslen(s);
		if (len == 0)
		{
			return Sstring("");
		}
		auto ss = Sstring_of_length("", len);
		for (auto i = 0; i < len; i++)
		{
			Sstring_set(ss, i, s[i]);
		}
		return ss;
	}

	ConversionResult ConvertUTF8toUTF16(const UTF8** sourceStart, const UTF8* sourceEnd, UTF16** targetStart,
		const UTF16* targetEnd, conversion_flags flags)
	{
		ConversionResult result = conversionOK;
		const UTF8* source = *sourceStart;
		UTF16* target = *targetStart;
		while (source < sourceEnd)
		{
			UTF32 ch = 0;
			unsigned short extra_bytes_to_read = trailing_bytes_for_utf8[*source];
			if (source + extra_bytes_to_read >= sourceEnd)
			{
				result = sourceExhausted;
				break;
			}
			/* Do this check whether lenient or strict */
			if (!is_legal_utf8(source, extra_bytes_to_read + 1))
			{
				result = sourceIllegal;
				break;
			}
			/*
			* The cases all fall through. See "Note A" below.
			*/
			switch (extra_bytes_to_read)
			{
			case 5:
				ch += *source++;
				ch <<= 6; /* remember, illegal UTF-8 */
			case 4:
				ch += *source++;
				ch <<= 6; /* remember, illegal UTF-8 */
			case 3:
				ch += *source++;
				ch <<= 6;
			case 2:
				ch += *source++;
				ch <<= 6;
			case 1:
				ch += *source++;
				ch <<= 6;
			case 0:
				ch += *source++;
			}
			ch -= offsets_from_utf8[extra_bytes_to_read];

			if (target >= targetEnd)
			{
				source -= (extra_bytes_to_read + 1); /* Back up source pointer! */
				result = targetExhausted;
				break;
			}
			if (ch <= UNI_MAX_BMP)
			{
				/* Target is a character <= 0xFFFF */
				/* UTF-16 surrogate values are illegal in UTF-32 */
				if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)
				{
					if (flags == strictConversion)
					{
						source -= (extra_bytes_to_read + 1); /* return to the illegal value itself */
						result = sourceIllegal;
						break;
					}
					else
					{
						*target++ = UNI_REPLACEMENT_CHAR;
					}
				}
				else
				{
					*target++ = static_cast<UTF16>(ch); /* normal case */
				}
			}
			else if (ch > UNI_MAX_UTF16)
			{
				if (flags == strictConversion)
				{
					result = sourceIllegal;
					source -= (extra_bytes_to_read + 1); /* return to the start */
					break; /* Bail out; shouldn't continue */
				}
				else
				{
					*target++ = UNI_REPLACEMENT_CHAR;
				}
			}
			else
			{
				/* target is a character in range 0xFFFF - 0x10FFFF. */
				if (target + 1 >= targetEnd)
				{
					source -= (extra_bytes_to_read + 1); /* Back up source pointer! */
					result = targetExhausted;
					break;
				}
				ch -= halfBase;
				*target++ = static_cast<UTF16>((ch >> halfShift) + UNI_SUR_HIGH_START);
				*target++ = static_cast<UTF16>((ch & halfMask) + UNI_SUR_LOW_START);
			}
		}
		*sourceStart = source;
		*targetStart = target;
		return result;
	}


	// 
	// https://github.com/nlohmann/json/issues/1198
	//
	void trim_utf8(std::string& hairy) {
		std::vector<bool> results;
		std::string smooth;
		size_t len = hairy.size();
		results.reserve(len);
		smooth.reserve(len);
		const unsigned char* bytes = (const unsigned char*)hairy.c_str();

		auto read_utf8 = [](const unsigned char* bytes, size_t len, size_t* pos) -> unsigned {
			int code_unit1 = 0;
			int code_unit2, code_unit3, code_unit4;

			if (*pos >= len) goto ERROR1;
			code_unit1 = bytes[(*pos)++];

			if (code_unit1 < 0x80) return code_unit1;
			else if (code_unit1 < 0xC2) goto ERROR1; // continuation or overlong 2-byte sequence
			else if (code_unit1 < 0xE0) {
				if (*pos >= len) goto ERROR1;
				code_unit2 = bytes[(*pos)++]; //2-byte sequence
				if ((code_unit2 & 0xC0) != 0x80) goto ERROR2;
				return (code_unit1 << 6) + code_unit2 - 0x3080;
			}
			else if (code_unit1 < 0xF0) {
				if (*pos >= len) goto ERROR1;
				code_unit2 = bytes[(*pos)++]; // 3-byte sequence
				if ((code_unit2 & 0xC0) != 0x80) goto ERROR2;
				if (code_unit1 == 0xE0 && code_unit2 < 0xA0) goto ERROR2; // overlong
				if (*pos >= len) goto ERROR2;
				code_unit3 = bytes[(*pos)++];
				if ((code_unit3 & 0xC0) != 0x80) goto ERROR3;
				return (code_unit1 << 12) + (code_unit2 << 6) + code_unit3 - 0xE2080;
			}
			else if (code_unit1 < 0xF5) {
				if (*pos >= len) goto ERROR1;
				code_unit2 = bytes[(*pos)++]; // 4-byte sequence
				if ((code_unit2 & 0xC0) != 0x80) goto ERROR2;
				if (code_unit1 == 0xF0 && code_unit2 < 0x90) goto ERROR2; // overlong
				if (code_unit1 == 0xF4 && code_unit2 >= 0x90) goto ERROR2; // > U+10FFFF
				if (*pos >= len) goto ERROR2;
				code_unit3 = bytes[(*pos)++];
				if ((code_unit3 & 0xC0) != 0x80) goto ERROR3;
				if (*pos >= len) goto ERROR3;
				code_unit4 = bytes[(*pos)++];
				if ((code_unit4 & 0xC0) != 0x80) goto ERROR4;
				return (code_unit1 << 18) + (code_unit2 << 12) + (code_unit3 << 6) + code_unit4 - 0x3C82080;
			}
			else goto ERROR1; // > U+10FFFF

		ERROR4:
			(*pos)--;
		ERROR3:
			(*pos)--;
		ERROR2:
			(*pos)--;
		ERROR1:
			return code_unit1 + 0xDC00;
		};

		unsigned c;
		size_t pos = 0;
		size_t pos_before;
		size_t inc = 0;
		bool valid;

		for (;;) {
			pos_before = pos;
			c = read_utf8(bytes, len, &pos);
			inc = pos - pos_before;
			if (!inc) break; // End of string reached.

			valid = false;

			if ((c <= 0x00007F)
				|| (c >= 0x000080 && c <= 0x0007FF)
				|| (c >= 0x000800 && c <= 0x000FFF)
				|| (c >= 0x001000 && c <= 0x00CFFF)
				|| (c >= 0x00D000 && c <= 0x00D7FF)
				|| (c >= 0x00E000 && c <= 0x00FFFF)
				|| (c >= 0x010000 && c <= 0x03FFFF)
				|| (c >= 0x040000 && c <= 0x0FFFFF)
				|| (c >= 0x100000 && c <= 0x10FFFF)) valid = true;

			if (c >= 0xDC00 && c <= 0xDCFF) {
				valid = false;
			}

			do results.push_back(valid); while (--inc);
		}

		size_t sz = results.size();
		for (size_t i = 0; i < sz; ++i) {
			if (results[i]) smooth.append(1, hairy.at(i));
		}
		hairy.swap(smooth);
	}


	const char* correct_non_utf_8(const char* s)
	{
		std::string in = s;
		const auto str = &in;
		int i, f_size = str->size();
		unsigned char c, c2, c3, c4;
		std::string to;
		to.reserve(f_size);

		for (i = 0; i < f_size; i++) {
			c = (unsigned char)(*str)[i];
			if (c < 32) {//control char
				if (c == 9 || c == 10 || c == 13) {//allow only \t \n \r
					to.append(1, c);
				}
				continue;
			}
			else if (c < 127) {//normal ASCII
				to.append(1, c);
				continue;
			}
			else if (c < 160) {//control char (nothing should be defined here either ASCI, ISO_8859-1 or UTF8, so skipping)
				if (c2 == 128) {//fix microsoft mess, add euro
					to.append(1, 226);
					to.append(1, 130);
					to.append(1, 172);
				}
				if (c2 == 133) {//fix IBM mess, add NEL = \n\r
					to.append(1, 10);
					to.append(1, 13);
				}
				continue;
			}
			else if (c < 192) {//invalid for UTF8, converting ASCII
				to.append(1, (unsigned char)194);
				to.append(1, c);
				continue;
			}
			else if (c < 194) {//invalid for UTF8, converting ASCII
				to.append(1, (unsigned char)195);
				to.append(1, c - 64);
				continue;
			}
			else if (c < 224 && i + 1 < f_size) {//possibly 2byte UTF8
				c2 = (unsigned char)(*str)[i + 1];
				if (c2 > 127 && c2 < 192) {//valid 2byte UTF8
					if (c == 194 && c2 < 160) {//control char, skipping
						;
					}
					else {
						to.append(1, c);
						to.append(1, c2);
					}
					i++;
					continue;
				}
			}
			else if (c < 240 && i + 2 < f_size) {//possibly 3byte UTF8
				c2 = (unsigned char)(*str)[i + 1];
				c3 = (unsigned char)(*str)[i + 2];
				if (c2 > 127 && c2 < 192 && c3>127 && c3 < 192) {//valid 3byte UTF8
					to.append(1, c);
					to.append(1, c2);
					to.append(1, c3);
					i += 2;
					continue;
				}
			}
			else if (c < 245 && i + 3 < f_size) {//possibly 4byte UTF8
				c2 = (unsigned char)(*str)[i + 1];
				c3 = (unsigned char)(*str)[i + 2];
				c4 = (unsigned char)(*str)[i + 3];
				if (c2 > 127 && c2 < 192 && c3>127 && c3 < 192 && c4>127 && c4 < 192) {//valid 4byte UTF8
					to.append(1, c);
					to.append(1, c2);
					to.append(1, c3);
					to.append(1, c4);
					i += 3;
					continue;
				}
			}
			//invalid UTF8, converting ASCII (c>245 || string too short for multi-byte))
			to.append(1, (unsigned char)195);
			to.append(1, c - 64);
		}
		return _strdup(to.c_str());
	}



	ptr clean_utf8_to_sstring(const char* s) {

		unsigned int byte2;
		if (s == nullptr) {
			return Sstring("");
		}
		const auto ll = Text::strlen_utf8(s);
		if (ll == 0) {
			return Sstring("");
		}

		const char* cptr = s;

		unsigned int byte = static_cast<unsigned char>(*cptr++);
		auto ss = Sstring_of_length("", ll);
		auto i = 0;

		while (byte != 0 && i < ll) {


			if (byte < 0x80) {
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			if (byte < 0xC0) {
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// skip
			while ((byte < 0xC0) && (byte >= 0x80)) {
				byte = static_cast<unsigned char>(*cptr++);
			}

			if (byte < 0xE0) {
				byte2 = static_cast<unsigned char>(*cptr++);
				if ((byte2 & 0xC0) == 0x80) {
					byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			if (byte < 0xF0) {

				byte2 = static_cast<unsigned char>(*cptr++);
				if (byte2 == 0) { break; };
				const unsigned int byte3 = static_cast<unsigned char>(*cptr++);
				if (byte3 == 0) { break; };

				if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80)) {
					byte = (((byte & 0x0F) << 12)
						| ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					if (byte == 0x200B || byte == 0x200D) byte = '*';
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			auto trail = total_bytes[byte] - 1; // expected number of trail bytes
			if (trail > 0) {
				int ch = byte & (0x3F >> trail);
				do {
					byte2 = static_cast<unsigned char>(*cptr++);
					if ((byte2 & 0xC0) != 0x80) {
						if (byte == 0x200B || byte == 0x200D) byte = '!';
						Sstring_set(ss, i++, byte);
						byte = static_cast<unsigned char>(*cptr++);
						continue;
					}
					ch <<= 6;
					ch |= (byte2 & 0x3F);
					trail--;
				} while (trail > 0);

				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// no match..
			if (i == ll) {
				break;
			}
			byte = static_cast<unsigned char>(*cptr++);
		}
		return ss;
	}

	ptr utf8_to_sstring(char* s) {
		std::string trimmed = s;
		Text::trim_utf8(trimmed);
		return clean_utf8_to_sstring(trimmed.c_str());
	}




	ptr const_utf8_to_sstring(const char* s) {



		// With UTF8 we just have sequences of bytes in a buffer.
		// in scheme we use a single longer integer for a code point.
		// see https://github.com/Komodo/KomodoEdit/blob/master/src/SciMoz/nsSciMoz.cxx
		// passes the greek test.

		unsigned int byte2;

		if (s == nullptr) {
			return Sstring("");
		}
		const auto ll = Text::strlen_utf8(s);
		if (ll == 0) {
			return Sstring("");
		}

		auto cptr = s;
		unsigned int byte = static_cast<unsigned char>(*cptr++);

		auto ss = Sstring_of_length("", ll);

		auto i = 0;

		while (byte != 0 && i < ll) {

			// ascii
			if (byte < 0x80) {
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			if (byte < 0xC0) {
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// skip
			while ((byte < 0xC0) && (byte >= 0x80)) {
				byte = static_cast<unsigned char>(*cptr++);
			}

			if (byte < 0xE0) {
				byte2 = static_cast<unsigned char>(*cptr++);
				if ((byte2 & 0xC0) == 0x80) {
					byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			if (byte < 0xF0) {
				byte2 = static_cast<unsigned char>(*cptr++);
				const unsigned int byte3 = static_cast<unsigned char>(*cptr++);
				if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80)) {
					byte = (((byte & 0x0F) << 12)
						| ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			auto trail = total_bytes[byte] - 1; // expected number of trail bytes
			if (trail > 0) {
				int ch = byte & (0x3F >> trail);
				do {
					byte2 = static_cast<unsigned char>(*cptr++);
					if ((byte2 & 0xC0) != 0x80) {
						Sstring_set(ss, i++, byte);
						byte = static_cast<unsigned char>(*cptr++);
						continue;
					}
					ch <<= 6;
					ch |= (byte2 & 0x3F);
					trail--;
				} while (trail > 0);
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// no match..
			if (i == ll) {
				break;
			}
			byte = static_cast<unsigned char>(*cptr++);
		}

		return ss;
	}

	ptr const_utf8_to_sstring_of_length(const char* s, const int length) {


		// With UTF8 we just have sequences of bytes in a buffer.
		// in scheme we use a single longer integer for a code point.
		// see https://github.com/Komodo/KomodoEdit/blob/master/src/SciMoz/nsSciMoz.cxx
		// passes the greek test.

		unsigned int byte2;
		if (s == nullptr) {
			return Sstring("");
		}
		const auto ll = Text::strlen_utf8(s);
		if (ll == 0) {
			return Sstring("");
		}

		auto cptr = s;
		unsigned int byte = static_cast<unsigned char>(*cptr++);
		auto ss = Sstring_of_length("", length);
		auto i = 0;

		while (byte != 0 && i < length) {

			// ascii
			if (byte < 0x80) {
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			if (byte < 0xC0) {
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}

			// skip
			while ((byte < 0xC0) && (byte >= 0x80)) {
				byte = static_cast<unsigned char>(*cptr++);
			}

			if (byte < 0xE0) {
				byte2 = static_cast<unsigned char>(*cptr++);
				if ((byte2 & 0xC0) == 0x80) {
					byte = (((byte & 0x1F) << 6) | (byte2 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			if (byte < 0xF0) {
				byte2 = static_cast<unsigned char>(*cptr++);
				const unsigned int byte3 = static_cast<unsigned char>(*cptr++);
				if (((byte2 & 0xC0) == 0x80) && ((byte3 & 0xC0) == 0x80)) {
					byte = (((byte & 0x0F) << 12)
						| ((byte2 & 0x3F) << 6) | (byte3 & 0x3F));
					Sstring_set(ss, i++, byte);
					byte = static_cast<unsigned char>(*cptr++);
				}
				continue;
			}

			auto trail = total_bytes[byte] - 1; // expected number of trail bytes
			if (trail > 0) {
				int ch = byte & (0x3F >> trail);
				do {
					byte2 = static_cast<unsigned char>(*cptr++);
					if ((byte2 & 0xC0) != 0x80) {
						Sstring_set(ss, i++, byte);
						byte = static_cast<unsigned char>(*cptr++);
						continue;
					}
					ch <<= 6;
					ch |= (byte2 & 0x3F);
					trail--;
				} while (trail > 0);
				Sstring_set(ss, i++, byte);
				byte = static_cast<unsigned char>(*cptr++);
				continue;
			}
			// no match..
			if (i == ll) {
				break;
			}
			byte = static_cast<unsigned char>(*cptr++);
		}
		return ss;
	}


	// https://github.com/CovenantEyes/uri-parser
// Based (heavily) on an article from CodeGuru
// http://www.codeguru.com/cpp/cpp/string/conversions/article.php/c12759



	const char hex2_dec[256] =
	{
		/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
		/* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 3 */ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,

		/* 4 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 5 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 6 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 7 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

		/* 8 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 9 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* A */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* B */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

		/* C */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* D */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* E */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* F */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};

	std::string uri_decode(const std::string& s_src)
	{
		const auto* p_src = reinterpret_cast<const unsigned char*>(s_src.c_str());
		const int src_len = s_src.length();
		const auto src_end = p_src + src_len;
		const auto src_last_dec = src_end - 2; // last decodable '%' 
		const auto p_start = new char[src_len];
		auto p_end = p_start;

		while (p_src < src_last_dec)
		{
			if (*p_src == '%')
			{
				char dec1, dec2;
				if (-1 != (dec1 = hex2_dec[*(p_src + 1)])
					&& -1 != (dec2 = hex2_dec[*(p_src + 2)]))
				{
					*p_end++ = (dec1 << 4) + dec2;
					p_src += 3;
					continue;
				}
			}
			*p_end++ = *p_src++;
		}

		// the last 2- chars
		while (p_src < src_end)
			*p_end++ = *p_src++;

		std::string s_result(p_start, p_end);
		delete[] p_start;
		return s_result;
	}

	// Only alphanum is safe.
	const char safe[256] =
	{
		/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
		/* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,

		/* 4 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		/* 6 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,

		/* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* A */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* B */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

		/* C */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* D */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* E */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* F */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	std::string uri_encode(const std::string& s_src)
	{
		const char dec2_hex[16 + 1] = "0123456789ABCDEF";
		auto p_src = reinterpret_cast<const unsigned char*>(s_src.c_str());
		const int src_len = s_src.length();
		const auto p_start = new unsigned char[src_len * 3];
		auto p_end = p_start;
		const auto src_end = p_src + src_len;

		for (; p_src < src_end; ++p_src)
		{
			if (safe[*p_src])
				*p_end++ = *p_src;
			else
			{
				// escape this char
				*p_end++ = '%';
				*p_end++ = dec2_hex[*p_src >> 4];
				*p_end++ = dec2_hex[*p_src & 0x0F];
			}
		}

		std::string s_result(reinterpret_cast<char*>(p_start), reinterpret_cast<char*>(p_end));
		delete[] p_start;
		return s_result;
	}


	static std::string tail_slice(std::string& subject, const std::string& delimiter, const bool keep_delim = false)
	{
		const auto delimiter_location = subject.find(delimiter);
		const auto delimiter_length = delimiter.length();
		std::string output;
		if (delimiter_location < std::string::npos)
		{
			const auto start = keep_delim ? delimiter_location : delimiter_location + delimiter_length;
			const auto end = subject.length() - start;
			output = subject.substr(start, end);
			subject = subject.substr(0, delimiter_location);
		}
		return output;
	}

	static std::string head_slice(std::string& subject, const std::string& delimiter)
	{
		const auto delimiter_location = subject.find(delimiter);
		const auto delimiter_length = delimiter.length();
		std::string output;
		if (delimiter_location < std::string::npos)
		{
			output = subject.substr(0, delimiter_location);
			subject = subject.substr(delimiter_location + delimiter_length,
				subject.length() - (delimiter_location + delimiter_length));
		}
		return output;
	}

	static inline int extract_port(std::string& hostport)
	{
		int port;
		const std::string delim = ":";
		auto portstring = tail_slice(hostport, delim);
		try { port = atoi(portstring.c_str()); }
		catch (std::exception& e) { port = -1; }
		return port;
	}

	std::string extract_path(std::string& in)
	{
		const std::string delim = "/";
		return head_slice(tail_slice(in, delim, false), "?");
	}

	static inline std::string extract_protocol(std::string& in)
	{
		const std::string delim = "://";
		return head_slice(in, delim);
	}

	std::string extract_search(std::string& in)
	{
		const std::string delim = "?";
		return tail_slice(in, delim);
	}

	static inline std::string extract_password(std::string& in)
	{
		const std::string delim = ":";
		return tail_slice(in, delim);
	}

	static inline std::string extract_user_pass(std::string& in)
	{
		const std::string delim = "@";
		return head_slice(in, delim);
	}



} // namespace Text



 

// we write onto the active_surface; we display from the display surface
Gdiplus::Bitmap* active_surface = nullptr;
Gdiplus::Bitmap* display_surface = nullptr;
Gdiplus::Bitmap* temp_surface = nullptr;
int _graphics_mode;
HANDLE g_image_rotation_mutex;


namespace GlobalGraphics
{
	using namespace Gdiplus;

	const wchar_t font_face[128] = L"Calibri";
	int font_size = 8;
	int font_hinting = Gdiplus::TextRenderingHintAntiAliasGridFit;
	Gdiplus::SolidBrush* solid_brush = nullptr;
	Gdiplus::SolidBrush* paper_brush = nullptr;
	Gdiplus::HatchBrush* hatch_brush = nullptr;
	Gdiplus::LinearGradientBrush* gradient_brush = nullptr;
	Gdiplus::Color foreground_colour;
	Gdiplus::Matrix* transform_matrix = new Gdiplus::Matrix();
	Gdiplus::SmoothingMode quality_mode = Gdiplus::SmoothingModeHighQuality;
	Gdiplus::Region* clip_region;
	float _pen_width = static_cast<float>(1.2);

	extern "C" __declspec(dllexport) ptr QUALITYHIGH()
	{
		quality_mode = Gdiplus::SmoothingModeHighQuality;
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr QUALITYFAST()
	{
		quality_mode = Gdiplus::SmoothingModeHighSpeed;
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr QUALITYANTIALIAS()
	{
		quality_mode = Gdiplus::SmoothingModeAntiAlias;
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr MATRIXRESET()
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		transform_matrix->Reset();
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr MATRIXINVERT()
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		transform_matrix->Invert();
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr MATRIXROTATEAT(int x, int y, float angle)
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		transform_matrix->RotateAt(angle, PointF(x, y));
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr MATRIXROTATE(float angle)
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		transform_matrix->Rotate(angle);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr MATRIXSHEAR(float x, float y)
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		transform_matrix->Shear(x, y);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr MATRIXSCALE(float x, float y)
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		transform_matrix->Scale(x, y);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr MATRIXTRANSLATE(float x, float y)
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		transform_matrix->Translate(x, y);
		return Strue;
	}

	int get_encoder_clsid(WCHAR* format, CLSID* p_clsid)
	{
		unsigned int num = 0, size = 0;

		GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1;

		auto* p_image_codec_info = static_cast<ImageCodecInfo*>(malloc(size));
		if (p_image_codec_info == nullptr)
			return -1;

		GetImageEncoders(num, size, p_image_codec_info);
		for (unsigned int j = 0; j < num; ++j)
		{
			if (wcscmp(p_image_codec_info[j].MimeType, format) == 0)
			{
				*p_clsid = p_image_codec_info[j].Clsid;
				free(p_image_codec_info);
				return j;
			}
		}
		free(p_image_codec_info);
		return -1;
	}

	Gdiplus::Bitmap* resize_clone(Bitmap* bmp, INT width, INT height)
	{
		const auto o_height = bmp->GetHeight();
		const auto o_width = bmp->GetWidth();
		auto n_width = width;
		auto n_height = height;
		const auto ratio = static_cast<double>(o_width) / static_cast<double>(o_height);
		if (o_width > o_height)
		{
			// Resize down by width
			n_height = static_cast<UINT>(static_cast<double>(n_width) / ratio);
		}
		else
		{
			n_width = static_cast<UINT>(n_height * ratio);
		}
		auto* new_bitmap = new Gdiplus::Bitmap(n_width, n_height, bmp->GetPixelFormat());
		Gdiplus::Graphics graphics(new_bitmap);
		graphics.DrawImage(bmp, 0, 0, n_width, n_height);
		return new_bitmap;
	}

	Gdiplus::Status h_bitmap_to_bitmap(HBITMAP source, Gdiplus::PixelFormat pixel_format, Gdiplus::Bitmap** result_out)
	{
		BITMAP source_info = { 0 };
		if (!::GetObjectW(source, sizeof(source_info), &source_info))
			return Gdiplus::GenericError;

		Gdiplus::Status s;

		std::unique_ptr<Gdiplus::Bitmap> target(
			new Gdiplus::Bitmap(source_info.bmWidth, source_info.bmHeight, pixel_format));
		if (!target.get())
			return Gdiplus::OutOfMemory;
		if ((s = target->GetLastStatus()) != Gdiplus::Ok)
			return s;

		Gdiplus::BitmapData target_info{};
		Gdiplus::Rect rect(0, 0, source_info.bmWidth, source_info.bmHeight);

		s = target->LockBits(&rect, Gdiplus::ImageLockModeWrite, pixel_format, &target_info);
		if (s != Gdiplus::Ok)
			return s;

		if (target_info.Stride != source_info.bmWidthBytes)
			return Gdiplus::InvalidParameter; // pixel_format is wrong!

		CopyMemory(target_info.Scan0, source_info.bmBits, source_info.bmWidthBytes * source_info.bmHeight);

		s = target->UnlockBits(&target_info);
		if (s != Gdiplus::Ok)
			return s;

		*result_out = target.release();

		return Gdiplus::Ok;
	}

	extern "C" __declspec(dllexport) ptr DRAWSTRING(const int x, const int y, char* text)
	{
		// text arrives from scheme as UTF8; GDI+ needs wide text
		auto draw_text = Text::Widen(text);
		if (active_surface == nullptr)
		{
			return Snil;
		}

		Graphics g2(active_surface);
		g2.SetClip(clip_region, CombineModeReplace);
		g2.SetSmoothingMode(quality_mode);
		g2.SetTransform(transform_matrix);
		g2.SetTextRenderingHint(static_cast<Gdiplus::TextRenderingHint>(font_hinting));
		const PointF origin(x, y);
		Gdiplus::Font font(font_face, font_size);
		g2.DrawString(draw_text.data(), draw_text.length(), &font, origin, nullptr, solid_brush);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr DRAWGRADIENTSTRING(const int x, const int y, char* text)
	{
		auto draw_text = Text::Widen(text);
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Graphics g2(active_surface);
		g2.SetClip(clip_region, CombineModeReplace);
		g2.SetSmoothingMode(quality_mode);
		g2.SetTransform(transform_matrix);
		g2.SetTextRenderingHint(static_cast<Gdiplus::TextRenderingHint>(font_hinting));
		const PointF origin(x, y);
		Gdiplus::Font font(font_face, font_size);
		g2.DrawString(draw_text.data(), draw_text.length(), &font, origin, nullptr, gradient_brush);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr SAVEASPNG(char* fname)
	{
		auto file_name = Text::Widen(fname);
		ULONG u_quality = 100;
		CLSID image_clsid;
		get_encoder_clsid(L"image/png", &image_clsid);
		const auto hr_ret = active_surface->Save(file_name.data(), &image_clsid, nullptr) == 0 ? S_OK : E_FAIL;
		if (hr_ret != S_OK)
		{
			return Sfalse;
		}
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr SAVEASJPEG(char* fname)
	{
		auto file_name = Text::Widen(fname);
		ULONG u_quality = 100;
		CLSID image_clsid;
		EncoderParameters encoder_params{};
		encoder_params.Count = 1;
		encoder_params.Parameter[0].NumberOfValues = 1;
		encoder_params.Parameter[0].Guid = EncoderQuality;
		encoder_params.Parameter[0].Type = EncoderParameterValueTypeLong;
		encoder_params.Parameter[0].Value = &u_quality;
		get_encoder_clsid(L"image/jpeg", &image_clsid);
		const HRESULT hr_ret = active_surface->Save(file_name.data(), &image_clsid, &encoder_params) == 0 ? S_OK : E_FAIL;
		if (hr_ret != S_OK)
		{
			return Sfalse;
		}
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr SAVETOCLIPBOARD(char* fname)
	{
		if (!OpenClipboard(nullptr))
		{
			return Snil;
		}

		if (!EmptyClipboard())
		{
			return Snil;
		}

		const Color color(255, 0, 0, 0);
		HBITMAP h_bitmap = nullptr;
		active_surface->GetHBITMAP(color, &h_bitmap);

		DIBSECTION ds;
		WinGetObject(h_bitmap, sizeof(ds), &ds);
		ds.dsBmih.biCompression = BI_RGB;
		const auto hDC = GetDC(nullptr);
		const auto hDDB = CreateDIBitmap(hDC, &ds.dsBmih,
			CBM_INIT, ds.dsBm.bmBits, reinterpret_cast<BITMAPINFO*>(&ds.dsBmih),
			DIB_RGB_COLORS);
		ReleaseDC(nullptr, hDC);

		if (!SetClipboardData(CF_BITMAP, hDDB))
		{
			DeleteObject(h_bitmap);
			CloseClipboard();
			return Strue;
		}
		DeleteObject(h_bitmap);
		CloseClipboard();
		return Snil;
	}

	extern "C" __declspec(dllexport) ptr FLIP(int d)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		active_surface->RotateFlip(static_cast<Gdiplus::RotateFlipType>(d));
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr SETPIXEL(const int x, const int y)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		active_surface->SetPixel(x, y, foreground_colour);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr RSETPIXEL(const REAL x, const REAL y)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		active_surface->SetPixel(x, y, foreground_colour);
		return Strue;
	}

#define CLIP_SMOOTH_TRANSFORM                    \
	g2.SetClip(clip_region, CombineModeReplace); \
	g2.SetSmoothingMode(quality_mode);           \
	g2.SetTransform(transform_matrix);

	extern "C" __declspec(dllexport) ptr FILLSOLIDRECT(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (solid_brush == nullptr)
		{
			solid_brush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillRectangle(solid_brush, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr REALFILLSOLIDRECT(const REAL x, const REAL y, const REAL w, const REAL h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (solid_brush == nullptr)
		{
			solid_brush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillRectangle(solid_brush, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr FILLGRADIENTRECT(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (gradient_brush == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillRectangle(gradient_brush, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr FILLHATCHRECT(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (hatch_brush == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillRectangle(hatch_brush, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr FILLSOLIDELLIPSE(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (solid_brush == nullptr)
		{
			solid_brush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillEllipse(solid_brush, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr FILLGRADIENTELLIPSE(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (gradient_brush == nullptr)
		{

			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillEllipse(gradient_brush, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr FILLHATCHELLIPSE(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (hatch_brush == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillEllipse(hatch_brush, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr
		FILLSOLIDPIE(const int x, const int y, const int w, const int h, const int i, const int j)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (solid_brush == nullptr)
		{
			solid_brush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
			g2.FillPie(solid_brush, x, y, w, h, i, j);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr
		FILLGRADIENTPIE(const int x, const int y, const int w, const int h, const int i, const int j)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (gradient_brush == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.FillPie(gradient_brush, x, y, w, h, i, j);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr
		FILLHATCHPIE(const int x, const int y, const int w, const int h, const int i, const int j)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		if (hatch_brush == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.FillPie(hatch_brush, x, y, w, h, i, j);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr DRAWRECT(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.DrawRectangle(&pen, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr
		DRAWARC(const int x, const int y, const int w, const int h, const int i, const int j)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.DrawArc(&pen, x, y, w, h, i, j);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr DRAWELLIPSE(const int x, const int y, const int w, const int h)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.DrawEllipse(&pen, x, y, w, h);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr
		DRAWPIE(const int x, const int y, const int w, const int h, const int i, const int j)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.DrawPie(&pen, x, y, w, h, i, j);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr DRAWLINE(const int x, const int y, const int x0, const int y0)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(foreground_colour, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.DrawLine(&pen, x, y, x0, y0);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr DRAWGRADIENTLINE(const int x, const int y, const int x0, const int y0)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Gdiplus::Pen pen(gradient_brush, _pen_width);
		Graphics g2(active_surface);
		CLIP_SMOOTH_TRANSFORM
		g2.DrawLine(&pen, x, y, x0, y0);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr PENWIDTH(const float w)
	{
		_pen_width = w;
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr SETFONTSIZE(const int w)
	{
		font_size = w;
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr COLR(const int r, const int g, const int b, const int a)
	{
		foreground_colour = Gdiplus::Color(a, r, g, b);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr
		GRADIENTBRUSH(int x, int y, int w, int h, int r, int g, int b, int a, int r0, int g0, int b0, int a0, double angle,
			bool z)
	{
		delete gradient_brush;
		gradient_brush = new Gdiplus::LinearGradientBrush(Gdiplus::Rect(x, y, w, h), Color(a, r, g, b),
			Color(a0, r0, g0, b0), angle, z);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr GRADIENTSHAPE(char* type, float focus, float scale)
	{
		if (gradient_brush == nullptr)
		{
			return Snil;
		}
		if (strcmp(type, "bell") == 0)
		{
			gradient_brush->SetBlendBellShape(focus, scale);
		}
		else if (strcmp(type, "triangular") == 0)
		{
			gradient_brush->SetBlendTriangularShape(focus, scale);
		}
		else
		{
			return Snil;
		}
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr SOLIDBRUSH(const int r, const int g, const int b, const int a)
	{
		if (solid_brush != nullptr)
		{
			delete solid_brush;
		}
		solid_brush = new Gdiplus::SolidBrush(Gdiplus::Color(a, r, g, b));
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr
		SETHATCHBRUSH(int style, int r, int g, int b, int a, int r0, int g0, int b0, int a0)
	{
		if (hatch_brush != nullptr)
		{
			delete hatch_brush;
		}

		hatch_brush = new Gdiplus::HatchBrush(Gdiplus::HatchStyle(style), Gdiplus::Color(a, r, g, b),
			Color(a0, r0, g0, b0));
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr PAPER(const int r, const int g, const int b, const int a)
	{
		if (paper_brush != nullptr)
		{
			delete paper_brush;
		}
		paper_brush = new Gdiplus::SolidBrush(Gdiplus::Color(a, r, g, b));
		return Strue;
	}

	// make and clear graphics images.

	// clear graphics
	extern "C" __declspec(dllexport) ptr CLRS(const int x, const int y)
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		if (paper_brush == nullptr)
		{
			paper_brush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
		}
		if (active_surface != nullptr)
		{
			delete active_surface;
		}
		if (display_surface != nullptr)
		{
			delete display_surface;
		}
		active_surface = new Gdiplus::Bitmap(x, y, PixelFormat32bppPARGB);
		display_surface = new Gdiplus::Bitmap(x, y, PixelFormat32bppPARGB);
		clip_region = new Gdiplus::Region(Gdiplus::Rect(0, 0, active_surface->GetWidth(), active_surface->GetHeight()));
		Gdiplus::Graphics g2(active_surface);
		g2.FillRectangle(paper_brush, 0, 0, x, y);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr CLRG(const int x, const int y)
	{
		if (transform_matrix == nullptr)
		{
			transform_matrix = new Gdiplus::Matrix();
		}
		if (gradient_brush == nullptr)
		{

			return Snil;
		}
		if (active_surface != nullptr)
		{
			delete active_surface;
		}
		if (display_surface != nullptr)
		{
			delete display_surface;
		}
		active_surface = new Gdiplus::Bitmap(x, y, PixelFormat32bppRGB);
		display_surface = new Gdiplus::Bitmap(x, y, PixelFormat32bppRGB);
		clip_region = new Gdiplus::Region(Gdiplus::Rect(0, 0, active_surface->GetWidth(), active_surface->GetHeight()));
		Gdiplus::Graphics g2(active_surface);
		g2.FillRectangle(gradient_brush, 0, 0, x, y);
		return Strue;
	}

	// swap the two buffers; if n=1 copy old to new; preserve progress.
	extern "C" __declspec(dllexport) ptr SWAP(int n) {

		WaitForSingleObject(g_image_rotation_mutex, INFINITE);
		if (n > 0) { // copy active surface to display surface.
			Gdiplus::Graphics g(display_surface);
			g.SetCompositingMode(CompositingModeSourceCopy);
			g.SetCompositingQuality(CompositingQualityHighSpeed);
			g.SetPixelOffsetMode(PixelOffsetModeNone);
			g.SetSmoothingMode(SmoothingModeNone);
			g.SetInterpolationMode(InterpolationModeDefault);
			g.DrawImage(active_surface, 0, 0);
		}
		temp_surface = active_surface;
		active_surface = display_surface;
		display_surface = temp_surface;
	
		ReleaseMutex(g_image_rotation_mutex);
		Sleep(1);
		return Strue;
	}

	Gdiplus::Bitmap * __stdcall get_display_surface(void) {
		return display_surface;
	}


	extern "C" __declspec(dllexport) Gdiplus::Bitmap * __stdcall RESIZEDCLONEDBITMAP(Gdiplus::Bitmap * bmp, int w, int h)
	{
		UINT o_height = bmp->GetHeight();
		UINT o_width = bmp->GetWidth();
		INT n_width = w;
		INT n_height = h;
		double ratio = ((double)o_width) / ((double)o_height);
		if (o_width > o_height) {
			// Resize down by width
			n_height = static_cast<UINT>(((double)n_width) / ratio);
		}
		else {
			n_width = static_cast<UINT>(n_height * ratio);
		}
		Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(n_width, n_height, bmp->GetPixelFormat());
		Gdiplus::Graphics graphics(newBitmap);
		graphics.DrawImage(bmp, 0, 0, n_width, n_height);
		return newBitmap;
	}

	extern "C" __declspec(dllexport) Gdiplus::Image * __stdcall RESIZEDCLONEIMAGE(Gdiplus::Image * image, int w, int h)
	{
		UINT o_height = image->GetHeight();
		UINT o_width = image->GetWidth();
		INT n_width = w;
		INT n_height = h;
		double ratio = ((double)o_width) / ((double)o_height);
		if (o_width > o_height) {
			// Resize down by width
			n_height = static_cast<UINT>(((double)n_width) / ratio);
		}
		else {
			n_width = static_cast<UINT>(n_height * ratio);
		}
		Gdiplus::Image* new_image = new Gdiplus::Bitmap(n_width, n_height, image->GetPixelFormat());
		Gdiplus::Graphics graphics(new_image);
		graphics.DrawImage(image, 0, 0, n_width, n_height);
		return new_image;
	}


	extern "C" __declspec(dllexport) Gdiplus::Image * __stdcall ROTATEDCLONEDIMAGE(int a, Gdiplus::Image * image)
	{

		FLOAT angle = a * 0.1;
		int h = image->GetHeight();
		int w = image->GetWidth();
		Gdiplus::PointF center(w / 2, h / 2);
		int new_h = h * 1.8;
		int new_w = w * 1.8;
		Gdiplus::Image* new_image = new Gdiplus::Bitmap(new_w, new_h, image->GetPixelFormat());
		Graphics g(new_image);
		Gdiplus::Matrix matrix;
		matrix.Translate(new_w / 2, new_h / 2);
		matrix.RotateAt(angle, center);
		g.SetTransform(&matrix);
		g.DrawImage(image, 0, 0);
		return new_image;
	}


	extern "C" __declspec(dllexport) ptr  __stdcall DISPLAYACTIVE(HDC h, int x, int y)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Graphics g(h);
		g.DrawImage(active_surface, x, y);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr  __stdcall DISPLAYSURFACE(Gdiplus::Bitmap * bitmap, HDC h, int x, int y)
	{
		if (active_surface == nullptr || bitmap == nullptr)
		{
			return Snil;
		}
		Graphics g(h);
		g.DrawImage(bitmap, x, y);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr  __stdcall IMAGETOSURFACE(Image * image, int x, int y)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Graphics g(active_surface);
		g.DrawImage(image, x, y);
		return Strue;
	}


	extern "C" __declspec(dllexport) ptr  __stdcall ROTATEDIMAGETOSURFACE(int angle, Image * image, int x, int y)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Graphics g(active_surface);
		Gdiplus::PointF center(x / 2, y / 2);
		Gdiplus::Matrix matrix;
		matrix.RotateAt(angle * 0.1, center);
		g.SetTransform(&matrix);
		g.DrawImage(image, x, y);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr  __stdcall SCALEDIMAGETOSURFACE(int s, Image * image, int x, int y)
	{
		if (active_surface == nullptr || image==nullptr)
		{
			return Snil;
		}
		Graphics g(active_surface);
		Gdiplus::Matrix matrix;
		matrix.Scale(s * 0.1, s * 0.1);
		Point p(x, y);
		g.SetTransform(&matrix);
		g.DrawImage(image, p);
		return Strue;
	}


	extern "C" __declspec(dllexport) ptr  __stdcall SCALEDROTATEDIMAGETOSURFACE(int s, int a, Image * image, int x, int y)
	{
		if (active_surface == nullptr || image == nullptr)
		{
			return Snil;
		}
		Graphics g(active_surface);
		Gdiplus::Matrix matrix;
		Gdiplus::PointF center(image->GetWidth() / 2, image->GetHeight() / 2);
		matrix.Scale(s * 0.1, s * 0.1);
		matrix.RotateAt(a * 0.1, center);
		g.SetTransform(&matrix);
		g.DrawImage(image, x, y);
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr  __stdcall LOADTOSURFACE(char* filename, int x, int y)
	{
		if (active_surface == nullptr)
		{
			return Snil;
		}
		Image image(Text::widen(filename).c_str());
		Graphics g2(active_surface);
		g2.DrawImage(&image, x, y);
		return Strue;
	}



	extern "C" __declspec(dllexport) ptr __stdcall FREESURFACE(Gdiplus::Bitmap * surface)
	{
		if (surface != nullptr)
		{
			delete surface;
		}
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr __stdcall FREEIMAGE(Image * image)
	{
		if (image != nullptr)
		{
			delete image;
		}
		return Strue;
	}

	extern "C" __declspec(dllexport) ptr __stdcall ACTIVATESURFACE(Gdiplus::Bitmap * surface)
	{
		if (surface != nullptr)
		{
			active_surface = surface;
		}
		return Strue;
	}

	extern "C" __declspec(dllexport) void* __stdcall MAKESURFACE(int w, int h)
	{
		auto new_surface = new Gdiplus::Bitmap(w, h, PixelFormat32bppRGB);
		Gdiplus::Graphics g2(new_surface);
		g2.FillRectangle(paper_brush, 0, 0, w, h);
		return(void*)new_surface;
	}

	extern "C" __declspec(dllexport) void* __stdcall LOADIMAGE(char* filename)
	{
		auto new_image = new Image(Text::widen(filename).c_str());
		return new_image;
	}

	extern "C" __declspec(dllexport)  void* __stdcall get_surface() {
		return active_surface;
	}


	// mode; stretch; fill etc.
	extern "C" __declspec(dllexport) ptr GRMODE(int m)
	{
		_graphics_mode = m;
		return Strue;
	}


	int graphics_mode() {
		return _graphics_mode;
	}

} // namespace 




void _init_graphics() {
	// graphics
	Sforeign_symbol("CLRS", static_cast<ptr>(GlobalGraphics::CLRS));
	Sforeign_symbol("CLRG", static_cast<ptr>(GlobalGraphics::CLRG));
	Sforeign_symbol("GSWAP", static_cast<ptr>(GlobalGraphics::SWAP));
	Sforeign_symbol("SAVEASPNG", static_cast<ptr>(GlobalGraphics::SAVEASPNG));
	Sforeign_symbol("SAVEASJPEG", static_cast<ptr>(GlobalGraphics::SAVEASJPEG));
	Sforeign_symbol("SAVETOCLIPBOARD", static_cast<ptr>(GlobalGraphics::SAVETOCLIPBOARD));
	Sforeign_symbol("PENWIDTH", static_cast<ptr>(GlobalGraphics::PENWIDTH));
	Sforeign_symbol("SOLIDBRUSH", static_cast<ptr>(GlobalGraphics::SOLIDBRUSH));
	Sforeign_symbol("SETHATCHBRUSH", static_cast<ptr>(GlobalGraphics::SETHATCHBRUSH));
	Sforeign_symbol("GRADIENTBRUSH", static_cast<ptr>(GlobalGraphics::GRADIENTBRUSH));
	Sforeign_symbol("GRADIENTSHAPE", static_cast<ptr>(GlobalGraphics::GRADIENTSHAPE));
	Sforeign_symbol("PAPER", static_cast<ptr>(GlobalGraphics::PAPER));
	Sforeign_symbol("DRAWRECT", static_cast<ptr>(GlobalGraphics::DRAWRECT));
	Sforeign_symbol("FILLSOLIDRECT", static_cast<ptr>(GlobalGraphics::FILLSOLIDRECT));
	Sforeign_symbol("REALFILLSOLIDRECT", static_cast<ptr>(GlobalGraphics::REALFILLSOLIDRECT));
	Sforeign_symbol("FILLGRADIENTRECT", static_cast<ptr>(GlobalGraphics::FILLGRADIENTRECT));
	Sforeign_symbol("FILLHATCHRECT", static_cast<ptr>(GlobalGraphics::FILLHATCHRECT));
	Sforeign_symbol("DRAWELLIPSE", static_cast<ptr>(GlobalGraphics::DRAWELLIPSE));
	Sforeign_symbol("FILLSOLIDELLIPSE", static_cast<ptr>(GlobalGraphics::FILLSOLIDELLIPSE));
	Sforeign_symbol("FILLGRADIENTELLIPSE", static_cast<ptr>(GlobalGraphics::FILLGRADIENTELLIPSE));
	Sforeign_symbol("FILLHATCHELLIPSE", static_cast<ptr>(GlobalGraphics::FILLHATCHELLIPSE));
	Sforeign_symbol("DRAWARC", static_cast<ptr>(GlobalGraphics::DRAWARC));
	Sforeign_symbol("DRAWPIE", static_cast<ptr>(GlobalGraphics::DRAWPIE));
	Sforeign_symbol("FILLSOLIDPIE", static_cast<ptr>(GlobalGraphics::FILLSOLIDPIE));
	Sforeign_symbol("FILLGRADIENTPIE", static_cast<ptr>(GlobalGraphics::FILLGRADIENTPIE));
	Sforeign_symbol("FILLHATCHPIE", static_cast<ptr>(GlobalGraphics::FILLHATCHPIE));
	Sforeign_symbol("DRAWLINE", static_cast<ptr>(GlobalGraphics::DRAWLINE));
	Sforeign_symbol("DRAWGRADIENTLINE", static_cast<ptr>(GlobalGraphics::DRAWGRADIENTLINE));
	Sforeign_symbol("DRAWSTRING", static_cast<ptr>(GlobalGraphics::DRAWSTRING));
	Sforeign_symbol("DRAWGRADIENTSTRING", static_cast<ptr>(GlobalGraphics::DRAWGRADIENTSTRING));
	Sforeign_symbol("SETPIXEL", static_cast<ptr>(GlobalGraphics::SETPIXEL));
	Sforeign_symbol("RSETPIXEL", static_cast<ptr>(GlobalGraphics::RSETPIXEL));
	Sforeign_symbol("COLR", static_cast<ptr>(GlobalGraphics::COLR));
	Sforeign_symbol("GRMODE", static_cast<ptr>(GlobalGraphics::GRMODE));
	Sforeign_symbol("SETFONTSIZE", static_cast<ptr>(GlobalGraphics::SETFONTSIZE));
	Sforeign_symbol("QUALITYFAST", static_cast<ptr>(GlobalGraphics::QUALITYFAST));
	Sforeign_symbol("QUALITYHIGH", static_cast<ptr>(GlobalGraphics::QUALITYHIGH));
	Sforeign_symbol("QUALITYANTIALIAS", static_cast<ptr>(GlobalGraphics::QUALITYANTIALIAS));
	Sforeign_symbol("FLIP", static_cast<ptr>(GlobalGraphics::FLIP));
	Sforeign_symbol("MATRIXRESET", static_cast<ptr>(GlobalGraphics::MATRIXRESET));
	Sforeign_symbol("MATRIXINVERT", static_cast<ptr>(GlobalGraphics::MATRIXINVERT));
	Sforeign_symbol("MATRIXSCALE", static_cast<ptr>(GlobalGraphics::MATRIXSCALE));
	Sforeign_symbol("MATRIXTRANSLATE", static_cast<ptr>(GlobalGraphics::MATRIXTRANSLATE));
	Sforeign_symbol("MATRIXSHEAR", static_cast<ptr>(GlobalGraphics::MATRIXSHEAR));
	Sforeign_symbol("MATRIXROTATE", static_cast<ptr>(GlobalGraphics::MATRIXROTATE));
	Sforeign_symbol("MATRIXROTATEAT", static_cast<ptr>(GlobalGraphics::MATRIXROTATEAT));
	Sforeign_symbol("LOADIMAGE", static_cast<ptr>(GlobalGraphics::LOADIMAGE));
	Sforeign_symbol("MAKESURFACE", static_cast<ptr>(GlobalGraphics::MAKESURFACE));
	Sforeign_symbol("ACTIVATESURFACE", static_cast<ptr>(GlobalGraphics::ACTIVATESURFACE));
 
	Sforeign_symbol("FREESURFACE", static_cast<ptr>(GlobalGraphics::FREESURFACE));
	Sforeign_symbol("FREEIMAGE", static_cast<ptr>(GlobalGraphics::FREEIMAGE));
	Sforeign_symbol("LOADTOSURFACE", static_cast<ptr>(GlobalGraphics::LOADTOSURFACE));
	Sforeign_symbol("SCALEDROTATEDIMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::SCALEDROTATEDIMAGETOSURFACE));
	Sforeign_symbol("SCALEDIMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::SCALEDIMAGETOSURFACE));
	Sforeign_symbol("ROTATEDIMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::ROTATEDIMAGETOSURFACE));
	Sforeign_symbol("IMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::IMAGETOSURFACE));
	Sforeign_symbol("DISPLAYSURFACE", static_cast<ptr>(GlobalGraphics::DISPLAYSURFACE));
	Sforeign_symbol("DISPLAYACTIVE", static_cast<ptr>(GlobalGraphics::DISPLAYACTIVE));
	Sforeign_symbol("RESIZEDCLONEDBITMAP", static_cast<ptr>(GlobalGraphics::RESIZEDCLONEDBITMAP));
	Sforeign_symbol("RESIZEDCLONEIMAGE", static_cast<ptr>(GlobalGraphics::RESIZEDCLONEIMAGE));
	Sforeign_symbol("RESIZEDCLONEDBITMAP", static_cast<ptr>(GlobalGraphics::RESIZEDCLONEDBITMAP));
	Sforeign_symbol("ROTATEDCLONEDIMAGE", static_cast<ptr>(GlobalGraphics::ROTATEDCLONEDIMAGE));
	Sforeign_symbol("IMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::IMAGETOSURFACE));
	Sforeign_symbol("ROTATEDIMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::ROTATEDIMAGETOSURFACE));
	Sforeign_symbol("SCALEDIMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::SCALEDIMAGETOSURFACE));
	Sforeign_symbol("SCALEDROTATEDIMAGETOSURFACE", static_cast<ptr>(GlobalGraphics::SCALEDROTATEDIMAGETOSURFACE));
	Sforeign_symbol("LOADTOSURFACE", static_cast<ptr>(GlobalGraphics::LOADTOSURFACE));
	Sforeign_symbol("get_surface", static_cast<ptr>(GlobalGraphics::get_surface));
}

namespace Assoc
{
	ptr sstring(const char* symbol, const char* value)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), Text::constUTF8toSstring(value));
		return a;
	}

	ptr sflonum(const char* symbol, const float value)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), Sflonum(value));
		return a;
	}

	ptr sfixnum(const char* symbol, const int value)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), Sfixnum(value));
		return a;
	}

	ptr sptr(const char* symbol, ptr value)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), value);
		return a;
	}

	ptr cons_sstring(const char* symbol, const char* value, ptr l)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), Text::constUTF8toSstring(value));
		l = CALL2("cons", a, l);
		return l;
	}

	ptr cons_sbool(const char* symbol, bool value, ptr l)
	{
		ptr a = Snil;
		if (value) {
			a = CALL2("cons", Sstring_to_symbol(symbol), Strue);
		}
		else
		{
			a = CALL2("cons", Sstring_to_symbol(symbol), Sfalse);
		}
		l = CALL2("cons", a, l);
		return l;
	}

	ptr cons_sptr(const char* symbol, ptr value, ptr l)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), value);
		l = CALL2("cons", a, l);
		return l;
	}

	ptr cons_sfixnum(const char* symbol, const int value, ptr l)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), Sfixnum(value));
		l = CALL2("cons", a, l);
		return l;
	}

	ptr cons_sflonum(const char* symbol, const float value, ptr l)
	{
		ptr a = Snil;
		a = CALL2("cons", Sstring_to_symbol(symbol), Sflonum(value));
		l = CALL2("cons", a, l);
		return l;
	}
} // namespace Assoc