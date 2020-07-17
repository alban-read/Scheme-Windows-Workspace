// Win32++   Version 8.4
// Release Date: 10th March 2017
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2017  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


// Acknowledgements:
// Thanks to Adam Szulc for his initial CString code.

////////////////////////////////////////////////////////
// wxx_cstring.h
//  Declaration of the CString class

// This class is intended to provide a simple alternative to the MFC/ATL
// CString class that ships with Microsoft compilers. The CString class
// specified here is compatible with other compilers such as Borland 5.5
// and MinGW.

// Differences between this class and the MFC/ATL CString class
// ------------------------------------------------------------
// 1) The constructors for this class accepts both ANSI and Unicode characters and
//    automatically converts these to TCHAR as required.
//
// 2) This class is not reference counted, so these CStrings should be passed as
//    references or const references when used as function arguments. As a result there
//    is no need for functions like LockBuffer and UnLockBuffer.
//
// 3) The Format functions only accepts POD (Plain Old Data) arguments. It does not
//    accept arguments which are class or struct objects. In particular it does not
//    accept CString objects, unless these are cast to LPCTSTR.
//    This is demonstrates valid and invalid usage:
//      CString string1(_T("Hello World"));
//      CString string2;
//
//      // This is invalid, and produces undefined behaviour.
//      string2.Format(_T("String1 is: %s"), string1); // No! you can't do this
//
//      // This is ok
//      string2.Format(_T("String1 is: %s"), (LPCTSTR)string1); // Yes, this is correct
//
//      // This is better. It doesn't use casting
//      string2.Format(_T("String1 is: %s"), string1.c_str());  // This is correct too
//
//    Note: The MFC/ATL CString class uses a non portable hack to make its CString class
//          behave like a POD. Other compilers (such as the MinGW compiler) specifically
//          prohibit the use of non POD types for functions with variable argument lists.
//
// 4) This class provides a few additional functions:
//       c_str			Returns a const TCHAR string. This is an alternative for casting to LPCTSTR.
//       GetErrorString	Assigns CString to the error string for the specified System Error Code
//                      (from ::GetLastErrror() for example).
//       GetString		Returns a reference to the underlying std::basic_string<TCHAR>. This
//						reference can be used to modify the string directly.



#ifndef _WIN32XX_CSTRING_H_
#define _WIN32XX_CSTRING_H_


#include <assert.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <windows.h>
#include "wxx_textconv.h"


namespace Win32xx
{

	// Defines the maximum size for TCHAR strings
	enum Constants
	{
		MAX_MENU_STRING = 80,
		MAX_STRING_SIZE = 255
	};

	/////////////////////////////////////////////
	// Declaration of the CStringT class template
	//
	template <class T>
	class c_string_t
	{
		// Friend functions allow the left hand side to be something other than CStringT

		// These specialized friend declarations are compatible with all supported compilers
		friend c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, const c_string_t<CHAR>& string2);
		friend c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, const CHAR* pszText);
		friend c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, CHAR ch);
		friend c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, int val);
		friend c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, double val);
		friend c_string_t<CHAR> operator + (const CHAR* pszText, const c_string_t<CHAR>& string1);
		friend c_string_t<CHAR> operator + (CHAR ch, const c_string_t<CHAR>& string1);
		friend c_string_t<CHAR> operator + (int val, const c_string_t<CHAR>& string1);
		friend c_string_t<CHAR> operator + (double val, const c_string_t<CHAR>& string1);

		friend c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, const c_string_t<WCHAR>& string2);
		friend c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, const WCHAR* pszText);
		friend c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, WCHAR ch);
		friend c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, int val);
		friend c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, double val);
		friend c_string_t<WCHAR> operator + (const WCHAR* pszText, const c_string_t<WCHAR>& string1);
		friend c_string_t<WCHAR> operator + (WCHAR ch, const c_string_t<WCHAR>& string1);
		friend c_string_t<WCHAR> operator + (int val, const c_string_t<WCHAR>& string1);
		friend c_string_t<WCHAR> operator + (double val, const c_string_t<WCHAR>& string1);

		// These global functions don't need to be friends
	//	bool operator < (const CStringT<T>& string1, const CStringT<T>& string2);
	//	bool operator > (const CStringT<T>& string1, const CStringT<T>& string2);
	//	bool operator <= (const CStringT<T>& string1, const CStringT<T>& string2);
	//	bool operator >= (const CStringT<T>& string1, const CStringT<T>& string2);
	//	bool operator < (const CStringT<T>& string1, const T* pszText);
	//	bool operator > (const CStringT<T>& string1, const T* pszText);
	//	bool operator <= (const CStringT<T>& string1, const T* pszText);
	//	bool operator >= (const CStringT<T>& string1, const T* pszText);

		public:
		c_string_t();
		virtual ~c_string_t();
		c_string_t(const c_string_t& str);
		c_string_t(const T * pszText);
		c_string_t(T ch, int nLength = 1);
		c_string_t(const T * pszText, int nLength);
		c_string_t(int val);
		c_string_t(double val);

		c_string_t& operator = (const c_string_t& str);
		c_string_t& operator = (const T ch);
		c_string_t& operator = (const T* pszText);
		c_string_t& operator = (int val);
		c_string_t& operator = (double val);

		bool     operator == (const T* pszText) const;
		bool	 operator == (const c_string_t& str) const;
		bool     operator != (const T* pszText) const;
		bool	 operator != (const c_string_t& str) const;
				 operator const T*() const;
		T&   	 operator [] (int nIndex);
		c_string_t& operator += (const c_string_t& str);
		c_string_t& operator += (const T* szText);
		c_string_t& operator += (const T ch);
		c_string_t& operator += (int val);
		c_string_t& operator += (double val);

		// Attributes
		const T* c_str() const			{ return m_str.c_str(); }					// alternative for casting to const T*
		virtual  std::basic_string<T> GetString() const { return m_str; }			// returns copy of the std::basic_string<T>
		int      GetLength() const	{ return static_cast<int>(m_str.length()); }	// returns the length in characters

		// Operations
		BSTR     AllocSysString() const;
		void	 AppendFormat(const T* pszFormat,...);
		void	 AppendFormat(UINT nFormatID, ...);
		void	 Assign(const T* pszText, int nCount);
        int      Collate(const T* pszText) const;
        int      CollateNoCase(const T* pszText) const;
        int      Compare(const T* pszText) const;
        int      CompareNoCase(const T* pszText) const;
		int      Delete(int nIndex, int nCount = 1);
		int		 Find(T ch, int nIndex = 0 ) const;
		int      Find(const T* pszText, int nStart = 0) const;
		int		 FindOneOf(const T* pszText) const;
		void	 Format(UINT nID, ...);
		void     Format(const T* pszFormat,...);
        void     FormatV(const T* pszFormat, va_list args);
		void	 FormatMessage(const T* pszFormat,...);
        void     FormatMessageV(const T* pszFormat, va_list args);
		T		 GetAt(int nIndex) const;
		T*		 GetBuffer(int nMinBufLength);
		void	 GetErrorString(DWORD dwError);
		void	 GetWindowText(HWND hWnd);
		void     Empty();
		int      Insert(int nIndex, T ch);
		int      Insert(int nIndex, const c_string_t& str);
		bool     IsEmpty() const;
        c_string_t Left(int nCount) const;
		bool	 LoadString(UINT nID);		// defined in wincore.h
		void     MakeLower();
		void	 MakeReverse();
		void     MakeUpper();
		c_string_t Mid(int nFirst) const;
		c_string_t Mid(int nFirst, int nCount) const;
		void	 ReleaseBuffer( int nNewLength = -1 );
		int      Remove(const T* pszText);
		int      Replace(T chOld, T chNew);
		int      Replace(const T* pszOld, const T* pszNew);
		int      ReverseFind(const T* pszText, int nStart = -1) const;
		c_string_t Right(int nCount) const;
		void	 SetAt(int nIndex, T ch);
		BSTR	 SetSysString(BSTR* pBstr) const;
		c_string_t SpanExcluding(const T* pszText) const;
		c_string_t SpanIncluding(const T* pszText) const;
		c_string_t Tokenize(const T* pszTokens, int& iStart) const;
		void	 Trim();
		void	 TrimLeft();
		void	 TrimLeft(T chTarget);
		void	 TrimLeft(const T* pszTargets);
		void	 TrimRight();
		void	 TrimRight(T chTarget);
		void	 TrimRight(const T* pszTargets);
		void     Truncate(int nNewLength);

#ifndef _WIN32_WCE
		bool	 GetEnvironmentVariable(const T* pszVar);
#endif

	protected:
		std::basic_string<T> m_str;
		std::vector<T> m_buf;

	private:
		int		lstrlenT(const CHAR* pszText)	{ return lstrlenA(pszText); }
		int		lstrlenT(const WCHAR* pszText)	{ return lstrlenW(pszText); }
	};


	typedef c_string_t<CHAR> CStringA;
	typedef c_string_t<WCHAR> CStringW;

	///////////////////////////////////
	// Definition of the CString class
	//
	class c_string : public c_string_t<TCHAR>
	{
		friend c_string operator + (const c_string& string1, const c_string& string2);
		friend c_string operator + (const c_string& string1, const TCHAR* pszText);
		friend c_string operator + (const c_string& string1, TCHAR ch);
		friend c_string operator + (const c_string& string1, int val);
		friend c_string operator + (const c_string& string1, double val);
		friend c_string operator + (const TCHAR* pszText, const c_string& string1);
		friend c_string operator + (TCHAR ch, const c_string& string1);
		friend c_string operator + (int val, const c_string& string1);
		friend c_string operator + (double val, const c_string& string1);

	public:
		c_string() {}
		c_string(const c_string& str)           	: c_string_t<TCHAR>(str) {}
		c_string(LPCSTR pszText)					: c_string_t<TCHAR>(AtoT(pszText)) {}
		c_string(LPCWSTR pszText)				: c_string_t<TCHAR>(WtoT(pszText))	{}
		c_string(LPCSTR pszText, int nLength)	: c_string_t<TCHAR>(AtoT(pszText), nLength) {}
		c_string(LPCWSTR pszText, int nLength)	: c_string_t<TCHAR>(WtoT(pszText), nLength) {}
		c_string(int val)						: c_string_t<TCHAR>(val) {}
		c_string(double val)						: c_string_t<TCHAR>(val) {}

		c_string(char ch, int nLength = 1)
		{
			char str[2] = {0};
			str[0] = ch;
			AtoT tch(str);
			m_str.assign(nLength, ((LPCTSTR)tch)[0]);
		}

		c_string(WCHAR ch, int nLength = 1)
		{
			WCHAR str[2] = {0};
			str[0] = ch;
			WtoT tch(str);
			m_str.assign(nLength, ((LPCTSTR)tch)[0]);
		}

		c_string& operator = (const c_string& str)
		{
			m_str.assign(str.GetString());
			return *this;
		}

		c_string& operator = (const c_string_t<TCHAR>& str)
		{
			m_str.assign(str.GetString());
			return *this;
		}

		c_string& operator = (const char ch)
		{
			char str[2] = {0};
			str[0] = ch;
			AtoT tch(str);
			m_str.assign(1, ((LPCTSTR)tch)[0]);
			return *this;
		}

		c_string& operator = (const WCHAR ch)
		{
			WCHAR str[2] = {0};
			str[0] = ch;
			WtoT tch(str);
			m_str.assign(1, ((LPCTSTR)tch)[0]);
			return *this;
		}

		c_string& operator = (LPCSTR pszText)
		{
			m_str.assign(AtoT(pszText));
			return *this;
		}

		c_string& operator = (LPCWSTR pszText)
		{
			m_str.assign(WtoT(pszText));
			return *this;
		}

		c_string& operator += (LPCSTR szText)
		{
			m_str.append(AtoT(szText));
			return *this;
		}

		c_string& operator += (LPCWSTR szText)
		{
			m_str.append(WtoT(szText));
			return *this;
		}

		c_string& operator += (const char ch)
		{
			char str[2] = {0};
			str[0] = ch;
			AtoT tch(str);
			m_str.append(1, ((LPCTSTR)tch)[0]);
			return *this;
		}

		c_string& operator += (const WCHAR ch)
		{
			WCHAR str[2] = {0};
			str[0] = ch;
			WtoT tch(str);
			m_str.append(1, ((LPCTSTR)tch)[0]);
			return *this;
		}

		c_string& operator += (int val)
		{
			c_string str;
			str.Format(_T("%d"), val);
			m_str.append(str);
			return *this;
		}

		c_string& operator += (double val)
		{
			c_string str;
			str.Format(_T("%g"), val);
			m_str.append(str);
			return *this;
		}

		c_string Left(int nCount) const
		{
			c_string str;
			str = c_string_t<TCHAR>::Left(nCount);
			return str;
		}

		c_string Mid(int nFirst) const
		{
			c_string str;
			str = c_string_t<TCHAR>::Mid(nFirst);
			return str;
		}

		c_string Mid(int nFirst, int nCount) const
		{
			c_string str;
			str = c_string_t<TCHAR>::Mid(nFirst, nCount);
			return str;
		}

		c_string Right(int nCount) const
		{
			c_string str;
			str = c_string_t<TCHAR>::Right(nCount);
			return str;
		}

		c_string SpanExcluding(const TCHAR* pszText) const
		{
			c_string str;
		    str = c_string_t<TCHAR>::SpanExcluding(pszText);
		    return str;
		}

		c_string SpanIncluding(const TCHAR* pszText) const
		{
			c_string str;
			str = c_string_t<TCHAR>::SpanIncluding(pszText);
			return str;
		}

		c_string Tokenize(const TCHAR* pszTokens, int& iStart) const
		{
			c_string str;
			str = c_string_t<TCHAR>::Tokenize(pszTokens, iStart);
			return str;
		}

	};

} // namespace Win32xx


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

	/////////////////////////////////////////////
	// Definition of the CStringT class template
	//
	template <class T>
	inline c_string_t<T>::c_string_t()
	// Constructor.
	{
	}

	template <class T>
	inline c_string_t<T>::~c_string_t()
	// Destructor.
	{
	}

	template <class T>
	inline c_string_t<T>::c_string_t(const c_string_t& str)
	// Constructor. Assigns from a CStringT<T>.
	{
		m_str.assign(str.m_str);
	}

	template <class T>
	inline c_string_t<T>::c_string_t(const T* pszText)
	// Constructor. Assigns from from a const T* character array.
	{
		m_str.assign(pszText);
	}

	template <class T>
	inline c_string_t<T>::c_string_t(T ch, int nLength)
	// Constructor. Assigns from 1 or more T characters.
	{
		T str[2] = {0};
		str[0] = ch;
		m_str.assign(nLength, str[0]);
	}

	template <class T>
	inline c_string_t<T>::c_string_t(const T* pszText, int nLength)
	// Constructor. Assigns from a const T* possibly containing null characters.
	// Ensure the size of the text buffer holds nLength or more characters
	{
		memcpy(GetBuffer(nLength), pszText, nLength*sizeof(T));
		ReleaseBuffer(nLength);
	}

	template <class T>
	inline c_string_t<T>::c_string_t(int val)
	// Constructor. Assigns from an integer value.
	{
		Format(_T("%d"), val);
	}

	template <class T>
	// Constructor. Assigns from a double value.
	inline c_string_t<T>::c_string_t(double val)
	{
		Format(_T("%g"), val);
	}

	template <class T>
	// Assign from a const CStringT<T>.
	inline c_string_t<T>& c_string_t<T>::operator = (const c_string_t<T>& str)
	{
		m_str.assign(str.m_str);
		return *this;
	}

	template <class T>
	inline c_string_t<T>& c_string_t<T>::operator = (T ch)
	// Assign from a T character.
	{
		T str[2] = {0};
		str[0] = ch;
		m_str.assign(1, str[0]);
		return *this;
	}

	template <class T>
	// Assign from a const T* character array.
	inline c_string_t<T>& c_string_t<T>::operator = (const T* pszText)
	{
		m_str.assign(pszText);
		return *this;
	}

	template <>
	inline c_string_t<CHAR>& c_string_t<CHAR>::operator = (int val)
	// Assign from an int converted to CHAR array.
	{
		c_string_t str;
		str.Format("%d", val);
		m_str.assign(str.m_str);
		return *this;
	}

	template <>
	inline c_string_t<WCHAR>& c_string_t<WCHAR>::operator = (int val)
	// Assign from an int converted to a WCHAR array.
	{
		c_string_t str;
		str.Format(L"%d", val);
		m_str.assign(str.m_str);
		return *this;
	}

	template <>
	inline c_string_t<CHAR>& c_string_t<CHAR>::operator = (double val)
	// Assign from a double converted to a CHAR array.
	{
		c_string_t str;
		str.Format("%g", val);
		m_str.assign(str.m_str);
		return *this;
	}

	template <>
	inline c_string_t<WCHAR>& c_string_t<WCHAR>::operator = (double val)
	// Assign from a double converted to a WCHAR array.
	{
		c_string_t str;
		str.Format(L"%g", val);
		m_str.assign(str.m_str);
		return *this;
	}

	template <class T>
	inline bool c_string_t<T>::operator == (const T* pszText) const
	// Returns TRUE if the strings have the same content.
	{
		assert(pszText);
		return (0 == Compare(pszText));
	}

	template <class T>
	inline bool c_string_t<T>::operator == (const c_string_t& str) const
	// Returns TRUE if the strings have the same content.
	// Can compare CStringTs containing null characters.
	{
		return m_str == str.m_str;
	}

	template <class T>
	inline bool c_string_t<T>::operator != (const T* pszText) const
	// Returns TRUE if the strings have a different content.
	{
		assert(pszText);
		return Compare(pszText) != 0;
	}

	template <class T>
	inline bool c_string_t<T>::operator != (const c_string_t& str) const
	// Returns TRUE if the strings have a different content.
	// Can compares CStringTs containing null characters.
	{
		return m_str != str.m_str;
	}

	template <class T>
	inline c_string_t<T>::operator const T*() const
	// Function call operator. Returns a const T* character array.
	{
		return m_str.c_str();
	}

	template <class T>
	inline T& c_string_t<T>::operator [] (int nIndex)
	// Subscript operator. Returns the T character at the specified index.
	{
		assert(nIndex >= 0);
		assert(nIndex < GetLength());
		return m_str[nIndex];
	}

	template <class T>
	inline c_string_t<T>& c_string_t<T>::operator += (const c_string_t& str)
	// Addition assignment. Appends CStringT<T>.
	{
		m_str.append(str.m_str);
		return *this;
	}

	template <class T>
	inline c_string_t<T>& c_string_t<T>::operator += (const T* szText)
	// Addition assignment. Appends const T* character array.
	{
		m_str.append(szText);
		return *this;
	}

	template <class T>
	inline c_string_t<T>& c_string_t<T>::operator += (T ch)
	// Addition assignment. Appends a T character.
	{
		T str[2] = {0};
		str[0] = ch;
		m_str.append(1, str[0]);
		return *this;
	}

	template <>
	inline c_string_t<CHAR>& c_string_t<CHAR>::operator += (int val)
	// Addition assignment. Appends an int converted to a CHAR character array.
	{
		c_string_t str;
		str.Format("%d", val);
		m_str.append(str);
		return *this;
	}

	template <>
	inline c_string_t<WCHAR>& c_string_t<WCHAR>::operator += (int val)
	// Addition assignment. Appends an int converted to a WCHAR character array.
	{
		c_string_t str;
		str.Format(L"%d", val);
		m_str.append(str);
		return *this;
	}

	template <>
	inline c_string_t<CHAR>& c_string_t<CHAR>::operator += (double val)
	// Addition assignment. Appends a double converted to a CHAR character array.
	{
		c_string_t str;
		str.Format("%g", val);
		m_str.append(str);
		return *this;
	}

	template <>
	inline c_string_t<WCHAR>& c_string_t<WCHAR>::operator += (double val)
	// Addition assignment. Appends a double converted to a WCHAR character array.
	{
		c_string_t str;
		str.Format(L"%g", val);
		m_str.append(str);
		return *this;
	}

	template <>
	inline BSTR c_string_t<CHAR>::AllocSysString() const
	// Allocates a BSTR from the CStringT content.
	// Note: Ensure the returned BSTR is freed later with SysFreeString to avoid a memory leak.
	{
		BSTR bstr = ::SysAllocStringLen(AtoW(m_str.c_str()), (UINT)m_str.size());
		if (bstr == NULL)
			throw std::bad_alloc();

		return bstr;
	}

	template <>
	inline BSTR c_string_t<WCHAR>::AllocSysString() const
	// Allocates a BSTR from the CStringT content.
	// Note: Free the returned string later with SysFreeString to avoid a memory leak.
	{
		BSTR bstr = ::SysAllocStringLen(m_str.c_str(), (UINT)m_str.size());
		if (bstr == NULL)
			throw std::bad_alloc();

		return bstr;
	}

	template <class T>
	inline void c_string_t<T>::AppendFormat(const T* pszFormat,...)
	// Appends formatted data to an the CStringT content.
	{
		c_string_t str;

		va_list args;
		va_start(args, pszFormat);
		str.FormatV(pszFormat, args);
		va_end(args);

		m_str.append(str);
	}

	template <class T>
	inline void	c_string_t<T>::Assign(const T* pszText, int nCount)
	// Assigns the specified number of charaters from pszText to the CStringT.
	{
		m_str.assign(pszText, nCount);
	}

	template <>
	inline int c_string_t<CHAR>::Collate(const CHAR* pszText) const
	// Performs a case sensitive comparison of the two strings using locale-specific information.
	{
		assert(pszText);
		int res = CompareStringA(LOCALE_USER_DEFAULT, 0, m_str.c_str(), -1, pszText, -1);

		assert(res);
		if 		(res == CSTR_LESS_THAN) return -1;
		else if (res == CSTR_GREATER_THAN) return 1;

		return 0;
	}

	template <>
	inline int c_string_t<WCHAR>::Collate(const WCHAR* pszText) const
	// Performs a case sensitive comparison of the two strings using locale-specific information.
	{
		assert(pszText);
		int res = CompareStringW(LOCALE_USER_DEFAULT, 0, m_str.c_str(), -1, pszText, -1);

		assert(res);
		if 		(res == CSTR_LESS_THAN) return -1;
		else if (res == CSTR_GREATER_THAN) return 1;

		return 0;
	}

	template <>
	inline int c_string_t<CHAR>::CollateNoCase(const CHAR* pszText) const
	// Performs a case insensitive comparison of the two strings using locale-specific information.
	{
		assert(pszText);
		int res = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, m_str.c_str(), -1, pszText, -1);

		assert(res);
		if 		(res == CSTR_LESS_THAN) return -1;
		else if (res == CSTR_GREATER_THAN) return 1;

		return 0;
	}

	template <>
	inline int c_string_t<WCHAR>::CollateNoCase(const WCHAR* pszText) const
	// Performs a case insensitive comparison of the two strings using locale-specific information.
	{
		assert(pszText);
		int res = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, m_str.c_str(), -1, pszText, -1);

		assert(res);
		if 		(res == CSTR_LESS_THAN) return -1;
		else if (res == CSTR_GREATER_THAN) return 1;

		return 0;
	}

	template <>
	inline int c_string_t<CHAR>::Compare(const CHAR* pszText) const
	// Performs a case sensitive comparison of the two strings.
	{
		assert(pszText);
		return lstrcmpA(m_str.c_str(), pszText);
	}

	template <>
	inline int c_string_t<WCHAR>::Compare(const WCHAR* pszText) const
	// Performs a case sensitive comparison of the two strings.
	{
		assert(pszText);
		return lstrcmpW(m_str.c_str(), pszText);
	}

	template <>
	inline int c_string_t<CHAR>::CompareNoCase(const CHAR* pszText) const
	// Performs a case insensitive comparison of the two strings.
	{
		assert(pszText);
		return lstrcmpiA(m_str.c_str(), pszText);
	}

	template <>
	inline int c_string_t<WCHAR>::CompareNoCase(const WCHAR* pszText) const
	// Performs a case insensitive comparison of the two strings.
	{
		assert(pszText);
		return lstrcmpiW(m_str.c_str(), pszText);
	}

	template <class T>
	inline int c_string_t<T>::Delete(int nIndex, int nCount /* = 1 */)
	// Deletes a character or characters from the string.
	{
		assert(nIndex >= 0);
		assert(nCount >= 0);

		m_str.erase(nIndex, nCount);
		return static_cast<int>(m_str.size());
	}

	template <class T>
	inline void c_string_t<T>::Empty()
	// Erases the contents of the string.
	{
		m_str.erase();
	}

	template <class T>
	inline int c_string_t<T>::Find(T ch, int nIndex /* = 0 */) const
	// Finds a character in the string.
	{
		assert(nIndex >= 0);
		size_t s = m_str.find(ch, nIndex);

		if (s == std::string::npos) return -1;
		return static_cast<int>(s);
	}

	template <class T>
	inline int c_string_t<T>::Find(const T* pszText, int nIndex /* = 0 */) const
	// Finds a substring within the string.
	{
		assert(pszText);
		assert(nIndex >= 0);
		size_t s = m_str.find(pszText, nIndex);

		if (s == std::string::npos) return -1;
		return static_cast<int>(s);
	}

	template <class T>
	inline int c_string_t<T>::FindOneOf(const T* pszText) const
	// Finds the first matching character from a set.
	{
		assert(pszText);
		size_t s = m_str.find_first_of(pszText);

		if (s == std::string::npos) return -1;
		return static_cast<int>(s);
	}

	template <class T>
	inline void c_string_t<T>::Format(const T* pszFormat,...)
	// Formats the string as sprintf does.
	{
		va_list args;
		va_start(args, pszFormat);
		FormatV(pszFormat, args);
		va_end(args);
	}

	template <>
	inline void c_string_t<CHAR>::FormatV(const CHAR*  pszFormat, va_list args)
	// Formats the string using a variable list of arguments.
	{

		if (pszFormat)
		{
			int nResult = -1, nLength = 256;

			// A vector is used to store the CHAR array
			std::vector<CHAR> vBuffer;

			while (-1 == nResult)
			{
				vBuffer.assign( nLength+1, 0 );
				nResult = _vsnprintf(&vBuffer[0], nLength, pszFormat, args);
				nLength *= 2;
			}
			m_str.assign(&vBuffer[0]);
		}
	}

	template <>
	inline void c_string_t<WCHAR>::FormatV(const WCHAR*  pszFormat, va_list args)
	// Formats the string using a variable list of arguments.
	{

		if (pszFormat)
		{
			int nResult = -1, nLength = 256;

			// A vector is used to store the WCHAR array
			std::vector<WCHAR> vBuffer;

			while (-1 == nResult)
			{
				vBuffer.assign( nLength+1, 0 );
				nResult = _vsnwprintf(&vBuffer[0], nLength, pszFormat, args);
				nLength *= 2;
			}
			m_str.assign(&vBuffer[0]);
		}
	}

	template <class T>
	inline void c_string_t<T>::FormatMessage(const T* pszFormat,...)
	// Formats a message string.
	{
		va_list args;
		va_start(args, pszFormat);
		FormatMessageV(pszFormat, args);
		va_end(args);
	}

	template <>
	inline void c_string_t<CHAR>::FormatMessageV(const CHAR* pszFormat, va_list args)
	// Formats a message string using a variable argument list.
	{
		LPSTR pszTemp = 0;
		if (pszFormat)
		{
			DWORD dwResult = ::FormatMessageA(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER, pszFormat, 0, 0, (LPSTR)&pszTemp, 0, &args);

			if ( dwResult == 0 || pszTemp == 0 )
				throw std::bad_alloc();

			m_str = pszTemp;
			LocalFree(pszTemp);
		}
	}

	template <>
	inline void c_string_t<WCHAR>::FormatMessageV(const WCHAR* pszFormat, va_list args)
	// Formats a message string using a variable argument list.
	{
		LPWSTR pszTemp = 0;
		if (pszFormat)
		{
			DWORD dwResult = ::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER, pszFormat, 0, 0, (LPWSTR)&pszTemp, 0, &args);

			if ( dwResult == 0 || pszTemp == 0 )
				throw std::bad_alloc();

			m_str = pszTemp;
			LocalFree(pszTemp);
		}
	}

	template <class T>
	inline T c_string_t<T>::GetAt(int nIndex) const
	// Returns the character at the specified location within the string.
	{
		assert(nIndex >= 0);
		assert(nIndex < GetLength());
		return m_str[nIndex];
	}

	template <class T>
	inline T* c_string_t<T>::GetBuffer(int nMinBufLength)
	// Creates a buffer of nMinBufLength characters (+1 extra for NULL termination) and returns
	// a pointer to this buffer. This buffer can be used by any function which accepts a LPTSTR.
	// Care must be taken not to exceed the length of the buffer. Use ReleaseBuffer to safely
	// copy this buffer back to the CStringT object.
	//
	// Note: The buffer uses a vector. Vectors are required to be contiguous in memory under
	//       the current standard, whereas std::strings do not have this requirement.
	{
		assert (nMinBufLength >= 0);

		T ch = 0;
		m_buf.assign(nMinBufLength + 1, ch);
		typename std::basic_string<T>::iterator it_end;

		if (m_str.length() >= (size_t)nMinBufLength)
		{
			it_end = m_str.begin();
			std::advance(it_end, nMinBufLength);
		}
		else
			it_end = m_str.end();

		std::copy(m_str.begin(), it_end, m_buf.begin());

		return &m_buf[0];
	}

#ifndef _WIN32_WCE
	template <>
	inline bool c_string_t<CHAR>::GetEnvironmentVariable(const CHAR* pszVar)
	// Sets the string to the value of the specified environment variable.
	{
		assert(pszVar);
		Empty();

		int nLength = ::GetEnvironmentVariableA(pszVar, NULL, 0);
		if (nLength > 0)
		{
			std::vector<CHAR> vBuffer( nLength+1, 0 );
			::GetEnvironmentVariableA(pszVar, &vBuffer[0], nLength);
			m_str = &vBuffer[0];
		}

		return (nLength != 0);
	}

	template <>
	inline bool c_string_t<WCHAR>::GetEnvironmentVariable(const WCHAR* pszVar)
	// Sets the string to the value of the specified environment variable.
	{
		assert(pszVar);
		Empty();

		int nLength = ::GetEnvironmentVariableW(pszVar, NULL, 0);
		if (nLength > 0)
		{
			std::vector<WCHAR> vBuffer( nLength+1, 0 );
			::GetEnvironmentVariableW(pszVar, &vBuffer[0], nLength);
			m_str = &vBuffer[0];
		}

		return (nLength != 0);
	}
#endif // _WIN32_WCE

	template <>
	inline void c_string_t<CHAR>::GetWindowText(HWND hWndCtrl)
	{
		int nLength = ::GetWindowTextLengthA(hWndCtrl);
		if (nLength > 0)
		{
			std::vector<CHAR> vBuffer( nLength+1, 0 );
			::GetWindowTextA(hWndCtrl, &vBuffer[0], nLength+1);
			m_str = &vBuffer[0];
		}
	}

	template <>
	inline void c_string_t<WCHAR>::GetWindowText(HWND hWndCtrl)
	{
		int nLength = ::GetWindowTextLengthW(hWndCtrl);
		if (nLength > 0)
		{
			std::vector<WCHAR> vBuffer( nLength+1, 0 );
			::GetWindowTextW(hWndCtrl, &vBuffer[0], nLength+1);
			m_str = &vBuffer[0];
		}
	}

	template <>
	inline void c_string_t<CHAR>::GetErrorString(DWORD dwError)
	// Returns the error string for the specified System Error Code (e.g from GetLastErrror).
	{
		m_str.erase();

		CHAR* pTemp = 0;
		DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		::FormatMessageA(dwFlags, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&pTemp, 1, NULL);
		m_str.assign(pTemp);
		::LocalFree(pTemp);
	}

	template <>
	inline void c_string_t<WCHAR>::GetErrorString(DWORD dwError)
	// Returns the error string for the specified System Error Code (e.g from GetLastErrror).
	{
		m_str.erase();

		WCHAR* pTemp = 0;
		DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		::FormatMessageW(dwFlags, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pTemp, 1, NULL);
		m_str.assign(pTemp);
		::LocalFree(pTemp);
	}

	template <class T>
	inline int c_string_t<T>::Insert(int nIndex, T ch)
	// Inserts a single character or a substring at the given index within the string.
	{
		assert(nIndex >= 0);
		assert(ch);

		m_str.insert(nIndex, &ch, 1);
		return static_cast<int>(m_str.size());
	}

	template <class T>
	inline int c_string_t<T>::Insert(int nIndex, const c_string_t& str)
	// Inserts a single character or a substring at the given index within the string.
	{
		assert(nIndex >= 0);

		m_str.insert(nIndex, str);
		return static_cast<int>(m_str.size());
	}

	template <class T>
	inline bool c_string_t<T>::IsEmpty() const
	// Returns TRUE if the string is empty
	{
		return m_str.empty();
	}

	template <class T>
	inline c_string_t<T> c_string_t<T>::Left(int nCount) const
	// Extracts the left part of a string.
	{
		assert(nCount >= 0);

		c_string_t str;
		str.m_str.assign(m_str, 0, nCount);
		return str;
	}

	template <class T>
	inline void c_string_t<T>::MakeLower()
	// Converts all the characters in this string to lowercase characters.
	{
		std::transform(m_str.begin(), m_str.end(), m_str.begin(), &::tolower);
	}

	template <class T>
	inline void c_string_t<T>::MakeReverse()
	// Reverses the string.
	{
		// Error 2285 with Borland 5.5 occurs here unless option -tWM is used instead of -tW
		std::reverse(m_str.begin(), m_str.end());
	}

	template <class T>
	inline void c_string_t<T>::MakeUpper()
	// Converts all the characters in this string to uppercase characters.
	{
		// Error 2285 with Borland 5.5 occurs here unless option -tWM is used instead of -tW
		std::transform(m_str.begin(), m_str.end(), m_str.begin(), &::toupper);
	}

	template <class T>
	inline c_string_t<T> c_string_t<T>::Mid(int nFirst) const
	// Extracts the middle part of a string.
	{
		return Mid(nFirst, GetLength());
	}

	template <class T>
	inline c_string_t<T> c_string_t<T>::Mid(int nFirst, int nCount) const
	// Extracts the middle part of a string.
	{
		assert(nFirst >= 0);
		assert(nCount >= 0);

		c_string_t str;
		str.m_str.assign(m_str, nFirst, nCount);
		return str;
	}

	template <class T>
	inline int c_string_t<T>::ReverseFind(const T* pszText, int nIndex /* = -1 */) const
	// Search for a substring within the string, starting from the end.
	{
		assert(pszText);
		return static_cast<int>(m_str.rfind(pszText, nIndex));
	}

	template <class T>
	inline void c_string_t<T>::SetAt(int nIndex, T ch)
	// Sets the character at the specified position to the specified value.
	{
		assert(nIndex >= 0);
		assert(nIndex < GetLength());
		m_str[nIndex] = ch;
	}

	template <class T>
	inline void c_string_t<T>::ReleaseBuffer( int nNewLength /*= -1*/ )
	// This copies the contents of the buffer (acquired by GetBuffer) to this CStringT,
	// and releases the contents of the buffer. The default length of -1 copies from the
	// buffer until a null terminator is reached. If the buffer doesn't contain a null
	// terminator, you must specify the buffer's length.
	{
		assert (nNewLength <= (int)m_buf.size());

		if (-1 == nNewLength)
		{
			nNewLength = lstrlenT(&m_buf[0]);
		}

		T ch = 0;
		m_str.assign(nNewLength, ch);

		typename std::vector<T>::iterator it_end = m_buf.begin();
		std::advance(it_end, nNewLength);

		std::copy(m_buf.begin(), it_end, m_str.begin());
		m_buf.clear();
	}

	template <class T>
	inline int c_string_t<T>::Remove(const T* pszText)
	// Removes each occurrence of the specified substring from the string.
	{
		assert(pszText);

		int nCount = 0;
		size_t pos = 0;
		size_t len = lstrlenT(pszText);
		while ((pos = m_str.find(pszText, pos)) != std::string::npos)
		{
			m_str.erase(pos, len);
			++nCount;
		}
		return nCount;
	}

	template <class T>
	inline int c_string_t<T>::Replace(T chOld, T chNew)
	// Replaces each occurrence of the old character with the new character.
	{
		int nCount = 0;
		typename std::basic_string<T>::iterator it;
		it = m_str.begin();
		while (it != m_str.end())
		{
			if (*it == chOld)
			{
				*it = chNew;
				++nCount;
			}
			++it;
		}
		return nCount;
	}

	template <class T>
	inline int c_string_t<T>::Replace(const T* pszOld, const T* pszNew)
	// Replaces each occurrence of the old substring with the new substring.
	{
		assert(pszOld);
		assert(pszNew);

		int nCount = 0;
		size_t pos = 0;
		size_t lenOld = lstrlenT(pszOld);
		size_t lenNew = lstrlenT(pszNew);
		while ((pos = m_str.find(pszOld, pos)) != std::string::npos)
		{
			m_str.replace(pos, lenOld, pszNew);
			pos += lenNew;
			++nCount;
		}
		return nCount;
	}

	template <class T>
	inline c_string_t<T> c_string_t<T>::Right(int nCount) const
	// Extracts the right part of a string.
	{
		assert(nCount >= 0);

		c_string_t str;
		str.m_str.assign(m_str, m_str.size() - nCount, nCount);
		return str;
	}

	template <>
	inline BSTR c_string_t<CHAR>::SetSysString(BSTR* pBstr) const
	// Sets an existing BSTR object to the string.
	// Note: Ensure the returned BSTR is freed later with SysFreeString to avoid a memory leak.
	{
		assert(pBstr);

		if ( !::SysReAllocStringLen(pBstr, AtoW(m_str.c_str()), (UINT)m_str.length()) )
			throw std::bad_alloc();

		return *pBstr;
	}

	template <>
	inline BSTR c_string_t<WCHAR>::SetSysString(BSTR* pBstr) const
	// Sets an existing BSTR object to the string.
	{
		assert(pBstr);

		if ( !::SysReAllocStringLen(pBstr, m_str.c_str(), (UINT)m_str.length()) )
			throw std::bad_alloc();

		return *pBstr;
	}

	template <class T>
	inline c_string_t<T> c_string_t<T>::SpanExcluding(const T* pszText) const
	// Extracts characters from the string, starting with the first character,
	// that are not in the set of characters identified by pszCharSet.
	{
		assert (pszText);

		c_string_t str;
		size_t pos = 0;

		while ((pos = m_str.find_first_not_of(pszText, pos)) != std::string::npos)
		{
			str.m_str.append(1, m_str[pos++]);
		}

		return str;
	}

	template <class T>
	inline c_string_t<T> c_string_t<T>::SpanIncluding(const T* pszText) const
	// Extracts a substring that contains only the characters in a set.
	{
		assert (pszText);

		c_string_t str;
		size_t pos = 0;

		while ((pos = m_str.find_first_of(pszText, pos)) != std::string::npos)
		{
			str.m_str.append(1, m_str[pos++]);
		}

		return str;
	}

	template <class T>
	inline c_string_t<T> c_string_t<T>::Tokenize(const T* pszTokens, int& iStart) const
	// Extracts specified tokens in a target string.
	{
		assert(pszTokens);

		c_string_t str;
		if(iStart >= 0)
		{
		size_t pos1 = m_str.find_first_not_of(pszTokens, iStart);
		size_t pos2 = m_str.find_first_of(pszTokens, pos1);

		iStart = (int)pos2 + 1;
		if (pos2 == m_str.npos)
			iStart = -1;

		if (pos1 != m_str.npos)
			str.m_str = m_str.substr(pos1, pos2-pos1);
		}
		return str;
	}

	template <class T>
	inline void c_string_t<T>::Trim()
	// Trims all leading and trailing whitespace characters from the string.
	{
		TrimLeft();
		TrimRight();
	}

	template <class T>
	inline void c_string_t<T>::TrimLeft()
	// Trims leading whitespace characters from the string.
	{
		// This method is supported by the Borland 5.5 compiler
		typename std::basic_string<T>::iterator iter;
		for (iter = m_str.begin(); iter != m_str.end(); ++iter)
		{
			if (!_istspace(*iter))
				break;
		}

		m_str.erase(m_str.begin(), iter);
	}

	template <class T>
	inline void c_string_t<T>::TrimLeft(T chTarget)
	// Trims the specified character from the beginning of the string.
	{
		m_str.erase(0, m_str.find_first_not_of(chTarget));
	}

	template <class T>
	inline void c_string_t<T>::TrimLeft(const T* pszTargets)
	// Trims the specified set of characters from the beginning of the string.
	{
		assert(pszTargets);
		m_str.erase(0, m_str.find_first_not_of(pszTargets));
	}

	template <class T>
	inline void c_string_t<T>::TrimRight()
	// Trims trailing whitespace characters from the string.
	{
		// This method is supported by the Borland 5.5 compiler
		typename std::basic_string<T>::reverse_iterator riter;
		for (riter = m_str.rbegin(); riter < m_str.rend(); ++riter)
		{
			if (!_istspace(*riter))
				break;
		}

		m_str.erase(riter.base(), m_str.end());
	}

	template <class T>
	inline void c_string_t<T>::TrimRight(T chTarget)
	// Trims the specified character from the end of the string.
	{
		size_t pos = m_str.find_last_not_of(chTarget);
		if (pos != std::string::npos)
			m_str.erase(++pos);
	}

	template <class T>
	inline void c_string_t<T>::TrimRight(const T* pszTargets)
	// Trims the specified set of characters from the end of the string.
	{
		assert(pszTargets);

		size_t pos = m_str.find_last_not_of(pszTargets);
		if (pos != std::string::npos)
			m_str.erase(++pos);
	}

	template <class T>
	inline void c_string_t<T>::Truncate(int nNewLength)
	// Reduces the length of the string to the specified amount.
	{
		if (nNewLength < GetLength())
		{
			assert(nNewLength >= 0);
			m_str.erase(nNewLength);
		}
	}


	//////////////////////////////////////
	// CStringT global operator functions
	//  These functions are declared as friends of CStringT.
	//
	inline c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, const c_string_t<CHAR>& string2)
	// Addition operator.
	{
		c_string_t<CHAR> str(string1);
		str.m_str.append(string2.m_str);
		return str;
	}

	inline c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, const c_string_t<WCHAR>& string2)
	// Addition operator.
	{
		c_string_t<WCHAR> str(string1);
		str.m_str.append(string2.m_str);
		return str;
	}

	inline c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, const CHAR* pszText)
	// Addition operator.
	{
		c_string_t<CHAR> str(string1);
		str.m_str.append(pszText);
		return str;
	}

	inline c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, const WCHAR* pszText)
	// Addition operator.
	{
		c_string_t<WCHAR> str(string1);
		str.m_str.append(pszText);
		return str;
	}

	inline c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, CHAR ch)
	// Addition operator.
	{
		c_string_t<CHAR> str(string1);
		str.m_str.append(1, ch);
		return str;
	}

	inline c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, WCHAR ch)
	// Addition operator.
	{
		c_string_t<WCHAR> str(string1);
		str.m_str.append(1, ch);
		return str;
	}

	inline c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, int val)
	// Addition operator.
	{
		c_string_t<CHAR> str;
		str.Format("%s%a", string1.c_str(), val);
		return str;
	}

	inline c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, int val)
	// Addition operator.
	{
		c_string_t<WCHAR> str;
		str.Format(L"%s%d", string1.c_str(), val);
		return str;
	}

	inline c_string_t<CHAR> operator + (const c_string_t<CHAR>& string1, double val)
	// Addition operator.
	{
		c_string_t<CHAR> str;
		str.Format("%s%g", string1.c_str(), val);
		return str;
	}

	inline c_string_t<WCHAR> operator + (const c_string_t<WCHAR>& string1, double val)
	// Addition operator.
	{
        c_string_t<WCHAR> str;
        str.Format(L"%s%g", string1.c_str(), val);
        return str;
	}

	inline c_string_t<CHAR> operator + (const CHAR* pszText, const c_string_t<CHAR>& string1)
	// Addition operator.
	{
		c_string_t<CHAR> str(pszText);
		str.m_str.append(string1);
		return str;
	}

	inline c_string_t<WCHAR> operator + (const WCHAR* pszText, const c_string_t<WCHAR>& string1)
	// Addition operator.
	{
		c_string_t<WCHAR> str(pszText);
		str.m_str.append(string1);
		return str;
	}

	inline c_string_t<CHAR> operator + (CHAR ch, const c_string_t<CHAR>& string1)
	// Addition operator.
	{
		c_string_t<CHAR> str(ch);
		str.m_str.append(string1);
		return str;
	}

	inline c_string_t<WCHAR> operator + (WCHAR ch, const c_string_t<WCHAR>& string1)
	// Addition operator.
	{
		c_string_t<WCHAR> str(ch);
		str.m_str.append(string1);
		return str;
	}

	inline c_string_t<CHAR> operator + (int val, const c_string_t<CHAR>& string1)
	// Addition operator.
	{
		c_string_t<CHAR> str;
		str.Format("%d%s", val, string1.c_str());
		return str;
	}

	inline c_string_t<WCHAR> operator + (int val, const c_string_t<WCHAR>& string1)
	// Addition operator.
	{
		c_string_t<WCHAR> str;
		str.Format(L"%d%s", val, string1.c_str());
		return str;
	}

	inline c_string_t<CHAR> operator + (double val, const c_string_t<CHAR>& string1)
	// Addition operator.
	{
		c_string_t<CHAR> str;
		str.Format("%g%s", val, string1.c_str());
		return str;
	}

	inline c_string_t<WCHAR> operator + (double val, const c_string_t<WCHAR>& string1)
	// Addition operator.
	{
		c_string_t<WCHAR> str;
		str.Format(L"%g%s", val, string1.c_str());
		return str;
	}


	template <class T>
	inline bool operator < (const c_string_t<T>& string1, const c_string_t<T>& string2)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(string2) < 0);	// boolean expression
	}

	template <class T>
	inline bool operator > (const c_string_t<T>& string1, const c_string_t<T>& string2)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(string2) > 0);	// boolean expression
	}

	template <class T>
	inline bool operator <= (const c_string_t<T>& string1, const c_string_t<T>& string2)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(string2) <= 0);	// boolean expression
	}

	template <class T>
	inline bool operator >= (const c_string_t<T>& string1, const c_string_t<T>& string2)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(string2) >= 0);	// boolean expression
	}

	template <class T>
	inline bool	operator < (const c_string_t<T>& string1, const T* pszText)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(pszText) < 0);	// boolean expression
	}

	template <class T>
	inline bool	operator > (const c_string_t<T>& string1, const T* pszText)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(pszText) > 0);	// boolean expression
	}

	template <class T>
	inline bool operator <= (const c_string_t<T>& string1, const T* pszText)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(pszText) <= 0);	// boolean expression
	}

	template <class T>
	inline bool operator >= (const c_string_t<T>& string1, const T* pszText)
	// Performs a case sensitive comparison of the two strings.
	{
		return (string1.Compare(pszText) >= 0);	// boolean expression
	}



	//////////////////////////////////////////////
	// CString global operator functions
	//  These functions are declared friends of CString
	//
	inline c_string operator + (const c_string& string1, const c_string& string2)
	{
		c_string str(string1);
		str.m_str.append(string2.m_str);
		return str;
	}

	inline c_string operator + (const c_string& string1, const TCHAR* pszText)
	{
		c_string str(string1);
		str.m_str.append(pszText);
		return str;
	}

	inline c_string operator + (const c_string& string1, TCHAR ch)
	{
		c_string str(string1);
		str.m_str += ch;
		return str;
	}

	inline c_string operator + (const c_string& string1, int val)
	{
		c_string str;
		str.Format(_T("%s%d"), string1.c_str(), val);
		return str;
	}

	inline c_string operator + (const c_string& string1, double val)
	{
		c_string str;
		str.Format(_T("%s%g"), string1.c_str(), val);
		return str;
	}

	inline c_string operator + (const TCHAR* pszText, const c_string& string1)
	{
		c_string str(pszText);
		str.m_str.append(string1);
		return str;
	}

	inline c_string operator + (TCHAR ch, const c_string& string1)
	{
		c_string str(ch);
		str.m_str.append(string1);
		return str;
	}

	inline c_string operator + (int val, const c_string& string1)
	{
		c_string str;
		str.Format(_T("%d%s"), val, string1.c_str());
		return str;
	}

	inline c_string operator + (double val, const c_string& string1)
	{
		c_string str;
		str.Format(_T("%g%s"), val, string1.c_str());
		return str;
	}

	//////////////////////////////////////////////
	// CString global functions
	//
	inline c_string GetAppDataPath()
	// Returns the path to the AppData folder. Returns an empty CString if
	// the Operating System doesn't support the use of an AppData folder.
	// The AppData folder is available in Windows 2000 and above.
	{
		c_string AppData;

#ifndef _WIN32_WCE

		HMODULE hShell = ::LoadLibrary(_T("Shell32.dll"));
		if (hShell)
		{
			typedef HRESULT(WINAPI *MYPROC)(HWND, int, HANDLE, DWORD, LPTSTR);

			// Get the function pointer of the SHGetFolderPath function
  #ifdef UNICODE
			MYPROC pSHGetFolderPath = (MYPROC)GetProcAddress(hShell, "SHGetFolderPathW");
  #else
			MYPROC pSHGetFolderPath = (MYPROC)GetProcAddress(hShell, "SHGetFolderPathA");
  #endif

  #ifndef CSIDL_APPDATA
    #define CSIDL_APPDATA     0x001a
  #endif

  #ifndef CSIDL_FLAG_CREATE
    #define CSIDL_FLAG_CREATE 0x8000
  #endif

			if (pSHGetFolderPath)
			{
				// Call the SHGetFolderPath function to retrieve the AppData folder
				pSHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, AppData.GetBuffer(MAX_PATH));
				AppData.ReleaseBuffer();
			}

			FreeLibrary(hShell);
		}

#endif // _WIN32_WCE

		return AppData;
	}

	inline std::vector<c_string> GetCommandLineArgs()
	// Retrieves the command line arguments and stores them in a vector of CString.
	// Similar to CommandLineToArgvW, but supports all versions of Windows,
	// supports ANSI and Unicode, and doesn't require the user to use LocalFree.
	{
		std::vector<c_string> CommandLineArgs;
		c_string CommandLine = GetCommandLine();
		int index = 0;
		int endPos = 0;

		while (index < CommandLine.GetLength() )
		{
			// Is the argument quoted?
			bool IsQuoted = (CommandLine[index] == _T('\"'));

			if (IsQuoted)
			{
				// Find the terminating token (quote followed by space)
				endPos = CommandLine.Find( _T("\" ") , index);
				if (endPos == -1) endPos = CommandLine.GetLength()-1;

				// Store the argument in the CStringT vector without the quotes.
				c_string s;
				if (endPos - index < 2)
					s = _T("\"\"");		// "" for a single quote or double quote argument
				else
					s = CommandLine.Mid(index +1, endPos - index -1);

				CommandLineArgs.push_back(s);
				index = endPos + 2;
			}
			else
			{
				// Find the terminating token (space character)
				endPos = CommandLine.Find( _T(' ') , index);
				if (endPos == -1) endPos = CommandLine.GetLength();

				// Store the argument in the CStringT vector.
				c_string s = CommandLine.Mid(index, endPos - index);
				CommandLineArgs.push_back(s);
				index = endPos + 1;
			}

			// skip excess space characters
			while (index < CommandLine.GetLength() && CommandLine[index] == _T(' '))
				index++;
		}

		// CommandLineArgs is a vector of CStringT
		return CommandLineArgs;
	}


}	// namespace Win32xx

#endif//_WIN32XX_CSTRING_H_
