// 
// basetypes.h - basic types that C++ lacks
// 
// <copyright file="basetypes.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
#pragma once
#ifndef _BASETYPES_
#define _BASETYPES_

#if 0
#ifndef UNDER_CE    // fixed-buffer overloads not available for wince
#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES  // fixed-buffer overloads for strcpy() etc.
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif

#pragma warning (push)
#pragma warning (disable: 4793)    // caused by varargs

// disable certain parts of basetypes for wince compilation
#ifdef UNDER_CE
#define BASETYPES_NO_UNSAFECRTOVERLOAD // disable unsafe CRT overloads (safe functions don't exist in wince)
#define BASETYPES_NO_STRPRINTF         // dependent functions here are not defined for wince
#endif

#ifndef OACR    // dummies when we are not compiling under Office
#define OACR_WARNING_SUPPRESS(x, y)
#define OACR_WARNING_DISABLE(x, y)
#define OACR_WARNING_PUSH
#define OACR_WARNING_POP
#endif
#ifndef OACR_ASSUME    // this seems to be a different one
#define OACR_ASSUME(x)
#endif

// following oacr warnings are not level1 or level2-security
// in currect stage we want to ignore those warnings
// if necessay this can be fixed at later stage

// not a bug
OACR_WARNING_DISABLE(EXC_NOT_CAUGHT_BY_REFERENCE, "Not indicating a bug or security threat.");
OACR_WARNING_DISABLE(LOCALDECLHIDESLOCAL, "Not indicating a bug or security threat.");

// not reviewed
OACR_WARNING_DISABLE(MISSING_OVERRIDE, "Not level1 or level2_security.");
OACR_WARNING_DISABLE(EMPTY_DTOR, "Not level1 or level2_security.");
OACR_WARNING_DISABLE(DEREF_NULL_PTR, "Not level1 or level2_security.");
OACR_WARNING_DISABLE(INVALID_PARAM_VALUE_1, "Not level1 or level2_security.");
OACR_WARNING_DISABLE(VIRTUAL_CALL_IN_CTOR, "Not level1 or level2_security.");
OACR_WARNING_DISABLE(POTENTIAL_ARGUMENT_TYPE_MISMATCH, "Not level1 or level2_security.");

// determine WIN32 api calling convention
// it seems this is normally stdcall?? but when compiling as /clr:pure or /clr:Safe
// this is not supported, so in this case, we need to use the 'default' calling convention
// TODO: can we reuse the #define of WINAPI??
#ifdef _M_CEE_SAFE 
#define WINAPI_CC __clrcall
#elif _M_CEE
#define WINAPI_CC __clrcall
#else
#define WINAPI_CC __stdcall
#endif

// fix some warnings in STL
#if !defined(_DEBUG) || defined(_CHECKED) || defined(_MANAGED)
#pragma warning(disable : 4702) // unreachable code
#endif
#endif

#include "Platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // include here because we redefine some names later
#include <errno.h>
#include <string>
#include <vector>
#include <cmath>        // for HUGE_VAL
#include <assert.h>
#include <stdarg.h>
#include <map>
#include <stdexcept>
#include <locale>       // std::wstring_convert
#include <string>
#include <algorithm>    // for transform()
#include <unordered_map>
#include <chrono>
#include <thread>
#include <stack>
#include <mutex>
#include <memory>
#ifdef _MSC_VER
#include <codecvt>      // std::codecvt_utf8
#endif
#ifdef _WIN32
#include <windows.h>    // for CRITICAL_SECTION and Unicode conversion functions   --TODO: is there a portable alternative?
#endif
#if __unix__
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <sys/time.h>
typedef unsigned char byte;
#endif

#if 0
#ifdef _WIN32
#pragma push_macro("STRSAFE_NO_DEPRECATE")
#define STRSAFE_NO_DEPRECATE    // deprecation managed elsewhere, not by strsafe
#include <strsafe.h>    // for strbcpy() etc templates
#pragma pop_macro("STRSAFE_NO_DEPRECATE")
#endif
#endif

using namespace std;

#if 0
// CRT error handling seems to not be included in wince headers
// so we define our own imports
#ifdef UNDER_CE

// TODO: is this true - is GetLastError == errno?? - also this adds a dependency on windows.h
#define errno GetLastError() 

// strerror(x) - x here is normally errno - TODO: make this return errno as a string
#define strerror(x) "strerror error but can't report error number sorry!"
#endif

#ifdef _WIN32
#ifndef __in // dummies for sal annotations if compiler does not support it
#define __in
#define __inout_z
#define __in_count(x)
#define __inout_cap(x)
#define __inout_cap_c(x)
#endif
#ifndef __out_z_cap    // non-VS2005 annotations
#define __out_cap(x)
#define __out_z_cap(x)
#define __out_cap_c(x)
#endif

#ifndef __override      // and some more non-std extensions required by Office
#define __override virtual
#endif
#endif

// disable warnings for which fixing would make code less readable
#pragma warning(disable : 4290) //  throw() declaration ignored
#pragma warning(disable : 4244) // conversion from typeA to typeB, possible loss of data
#endif

// ----------------------------------------------------------------------------
// (w)cstring -- helper class like std::string but with auto-cast to char*
// ----------------------------------------------------------------------------

namespace msra { namespace strfun {
    // a class that can return a std::string with auto-convert into a const char*
    template<typename C> struct basic_cstring : public std::basic_string<C>
    {
        template<typename S> basic_cstring (S p) : std::basic_string<C> (p) { }
        operator const C * () const { return this->c_str(); }
    };
    typedef basic_cstring<char> cstring;
    typedef basic_cstring<wchar_t> wcstring;
}}

static inline wchar_t*GetWC(const char *c)
{
    const size_t cSize = strlen(c)+1;
    wchar_t* wc = new wchar_t[cSize];
#ifdef _WIN32
    size_t retVal;
    mbstowcs_s(&retVal, wc, cSize, c, cSize);
#else
    mbstowcs(wc, c, cSize);
#endif // _WIN32

    return wc;
}
#if 0   // needed with gcc because some regex function is not implemented properly there
struct MatchPathSeparator
{
    bool operator()( char ch ) const
    {
        return ch == '\\' || ch == '/';
    }
};
static inline std::string basename( std::string const& pathname)
{
    return std::string (std::find_if(pathname.rbegin(), pathname.rend(),MatchPathSeparator()).base(), pathname.end()); 
}

static inline std::string removeExtension (std::string const& filename)
{
    //std::string::const_reverse_iterator pivot = std::find(filename.rbegin(), filename.rend(), '.');
    //return pivot == filename.rend() ? filename: std::string(filename.begin(), pivot.base()-1);
    size_t lastindex = filename.find_last_of(".");
    return filename.substr(0, lastindex);
}
static inline std::wstring basename( std::wstring const& pathname)
{
    return std::wstring (std::find_if(pathname.rbegin(), pathname.rend(),MatchPathSeparator()).base(), pathname.end()); 
}

static inline std::wstring removeExtension (std::wstring const& filename)
{
    //std::wstring::const_reverse_iterator pivot = std::find(filename.rbegin(), filename.rend(), '.');
    //return pivot == filename.rend() ? filename: std::wstring(filename.begin(), pivot.base()-1);
    size_t lastindex = filename.find_last_of(L".");
    return filename.substr(0, lastindex);
}
#endif

// ----------------------------------------------------------------------------
// some mappings for non-Windows builds
// ----------------------------------------------------------------------------

#ifndef _MSC_VER    // add some functions that are VS-only
// --- basic file functions
// convert a wchar_t path to what gets passed to CRT functions that take narrow characters
// This is needed for the Linux CRT which does not accept wide-char strings for pathnames anywhere.
// Always use this function for mapping the paths.
static inline msra::strfun::cstring charpath (const std::wstring & p)
{
#ifdef _WIN32
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(p);
#else   // old version, delete once we know it works
    size_t len = p.length();
    std::vector<char> buf(2 * len + 1, 0); // max: 1 wchar => 2 mb chars
    ::wcstombs(buf.data(), p.c_str(), 2 * len + 1);
    return msra::strfun::cstring (&buf[0]);
#endif
}
static inline FILE* _wfopen (const wchar_t * path, const wchar_t * mode) { return fopen(charpath(path), charpath(mode)); }
static inline int _wunlink (const wchar_t * p) { return unlink (charpath (p)); }
static inline int _wmkdir (const wchar_t * p) { return mkdir (charpath (p), 0777/*correct?*/); }
// --- basic string functions
static inline wchar_t* wcstok_s (wchar_t* s, const wchar_t* delim, wchar_t** ptr) { return ::wcstok(s, delim, ptr); }
static inline int _stricmp  (const char * a, const char * b)                 { return ::strcasecmp (a, b); }
static inline int _strnicmp (const char * a, const char * b, size_t n)       { return ::strncasecmp (a, b, n); }
static inline int _wcsicmp  (const wchar_t * a, const wchar_t * b)           { return ::wcscasecmp (a, b); }
static inline int _wcsnicmp (const wchar_t * a, const wchar_t * b, size_t n) { return ::wcsncasecmp (a, b, n); }
static inline int64_t  _strtoi64  (const char * s, char ** ep, int r) { return strtoll (s, ep, r); }    // TODO: check if correct
static inline uint64_t _strtoui64 (const char * s, char ** ep, int r) { return strtoull (s, ep, r); }   // TODO: correct for size_t?
// -- other
//static inline void memcpy_s(void * dst, size_t dstsize, const void * src, size_t maxcount) { assert (maxcount <= dstsize); memcpy (dst, src, maxcount); }
static inline void Sleep (size_t ms) { std::this_thread::sleep_for (std::chrono::milliseconds (ms)); }
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

// ----------------------------------------------------------------------------
// basic macros   --TODO: do we need those? delete what we dont' need
// ----------------------------------------------------------------------------

#if 0
#ifndef assert
#ifdef _CHECKED // basetypes.h expects this function to be defined (it is in message.h)
extern void _CHECKED_ASSERT_error(const char * file, int line, const char * exp);
#define assert(exp) ((exp)||(_CHECKED_ASSERT_error(__FILE__,__LINE__,#exp),0))
#else
#define assert assert
#endif
#endif
#endif
#define UNUSED(x) (void)(x)

// ----------------------------------------------------------------------------
// basic data types
// ----------------------------------------------------------------------------

namespace msra { namespace basetypes {

    #ifdef __unix__
    typedef timeval LARGE_INTEGER;
    #endif
    class auto_timer
    {
        LARGE_INTEGER freq, start;
        auto_timer (const auto_timer &); void operator= (const auto_timer &);
    public:
        auto_timer()
        {
    #ifdef _WIN32
            if (!QueryPerformanceFrequency (&freq)) // count ticks per second
                RuntimeError("auto_timer: QueryPerformanceFrequency failure");
            QueryPerformanceCounter (&start);
    #endif
    #ifdef __unix__
            gettimeofday (&start, NULL);
    #endif
        }
        operator double() const     // each read gives time elapsed since start, in seconds
        {
            LARGE_INTEGER end;
    #ifdef _WIN32
            QueryPerformanceCounter (&end);
            return (end.QuadPart - start.QuadPart) / (double) freq.QuadPart;
    #endif
    #ifdef __unix__
            gettimeofday (&end,NULL);
            return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/(1000*1000);
    #endif
        }
        void show (const std::string & msg) const
        {
            double elapsed = *this;
            fprintf(stderr, "%s: %.6f ms\n", msg.c_str(), elapsed * 1000.0/*to ms*/);
        }
    };

// class std::vector -- std::vector with array-bounds checking
// VS 2008 and above do this, so there is no longer a need for this.

#pragma warning(push)
#pragma warning(disable : 4555) // expression has no affect, used so retail won't be empty

#if 0
template<class _ElemType>
class std::vector : public std::vector<_ElemType>
{
#if defined (_DEBUG) || defined (_CHECKED)    // debug version with range checking
    static void throwOutOfBounds()
    {   // (moved to separate function hoping to keep inlined code smaller
        OACR_WARNING_PUSH;
        OACR_WARNING_DISABLE(IGNOREDBYCOMMA, "Reviewd OK. Special trick below to show a message when assertion fails"
            "[rogeryu 2006/03/24]");
        OACR_WARNING_DISABLE(BOGUS_EXPRESSION_LIST, "This is intentional. [rogeryu 2006/03/24]");
        //assert ("std::vector::operator[] out of bounds", false);
        OACR_WARNING_POP;
    }
#endif

public:

    std::vector() : std::vector<_ElemType> () { }
    std::vector (int size) : std::vector<_ElemType> (size) { }

#if defined (_DEBUG) || defined (_CHECKED)    // debug version with range checking
    // ------------------------------------------------------------------------
    // operator[]: with array-bounds checking
    // ------------------------------------------------------------------------

    inline _ElemType & operator[] (int index)            // writing
    {
        if (index < 0 || index >= size()) throwOutOfBounds();
        return (*(std::vector<_ElemType>*) this)[index];
    }

    // ------------------------------------------------------------------------

    inline const _ElemType & operator[] (int index) const    // reading
    {
        if (index < 0 || index >= size()) throwOutOfBounds();
        return (*(std::vector<_ElemType>*) this)[index];
    }
#endif

    // ------------------------------------------------------------------------
    // size(): same as base class, but returning an 'int' instead of 'size_t'
    // to allow for better readable code
    // ------------------------------------------------------------------------

    inline int size() const
    {
        size_t siz = ((std::vector<_ElemType>*) this)->size();
        return (int) siz;
    }
};
// overload swap(), otherwise we'd fallback to 3-way assignment & possibly throw
template<class _T> inline void swap (std::vector<_T> & L, std::vector<_T> & R)  throw()
{ swap ((std::vector<_T> &) L, (std::vector<_T> &) R); }
#endif

// class fixed_vector - non-resizable vector

template<class _T> class fixed_vector
{
    _T * p;                 // pointer array
    size_t n;               // number of elements
    void check (int index) const 
    { 
        assert (index >= 0 && (size_t) index < n);
#ifdef NDEBUG
        UNUSED(index);
#endif
    }
    void check (size_t index) const 
    {
        assert (index < n); 
#ifdef NDEBUG
        UNUSED(index);
#endif
    }
    // ... TODO: when I make this public, LinearTransform.h acts totally up but I cannot see where it comes from.
    //fixed_vector (const fixed_vector & other) : n (0), p (NULL) { *this = other; }
public:
    fixed_vector() : n (0), p (NULL) { }
    void resize (int size) { clear(); if (size > 0) { p = new _T[size]; n = size; } }
    void resize (size_t size) { clear(); if (size > 0) { p = new _T[size]; n = size; } }
    fixed_vector (int size) : n (size), p (size > 0 ? new _T[size] : NULL) { }
    fixed_vector (size_t size) : n ((int) size), p (size > 0 ? new _T[size] : NULL) { }
    ~fixed_vector() { delete[] p; }
    int size() const { return (int) n; }
    inline int capacity() const { return (int) n; }
    bool empty() const { return n == 0; }
    void clear() { delete[] p; p = NULL; n = 0; }
    _T *       begin()       { return p; }
    const _T * begin() const { return p; }
    _T * end()   { return p + n; } // note: n == 0 so result is NULL
    inline       _T & operator[] (int index)          { check (index); return p[index]; }  // writing
    inline const _T & operator[] (int index) const    { check (index); return p[index]; }  // reading
    inline       _T & operator[] (size_t index)       { check (index); return p[index]; }  // writing
    inline const _T & operator[] (size_t index) const { check (index); return p[index]; }  // reading
    inline int indexof (const _T & elem) const { assert (&elem >= p && &elem < p + n); return &elem - p; }
    void swap (fixed_vector & other)  throw() { std::swap (other.p, p); std::swap (other.n, n); }
    template<class VECTOR> fixed_vector & operator= (const VECTOR & other)
    {
        int other_n = (int) other.size();
        fixed_vector tmp (other_n);
        for (int k = 0; k < other_n; k++) tmp[k] = other[k];
        swap (tmp);
        return *this;
    }
    fixed_vector & operator= (const fixed_vector & other)
    {
        int other_n = (int) other.size();
        fixed_vector tmp (other_n);
        for (int k = 0; k < other_n; k++) tmp[k] = other[k];
        swap (tmp);
        return *this;
    }
    template<class VECTOR> fixed_vector (const VECTOR & other) : n (0), p (NULL) { *this = other; }
};
template<class _T> inline void swap (fixed_vector<_T> & L, fixed_vector<_T> & R)  throw() { L.swap (R); }

#pragma warning(pop)    // pop off waring: expression has no effect

// class matrix - simple fixed-size 2-dimensional array, access elements as m(i,j)
// stored as concatenation of rows

#if 1
template<class T> class matrix : fixed_vector<T>
{
    size_t numcols;
    size_t locate(size_t i, size_t j) const { assert(i < rows() && j < cols()); return i * cols() + j; }
public:
    typedef T elemtype;
    matrix() : numcols(0) {}
    matrix(size_t n, size_t m) { resize(n, m); }
    void resize(size_t n, size_t m) { numcols = m; fixed_vector<T>::resize(n * m); }
    size_t cols() const { return numcols; }
    size_t rows() const { return empty() ? 0 : size() / cols(); }
    size_t size() const { return fixed_vector<T>::size(); }    // use this for reading and writing... not nice!
    bool empty() const { return fixed_vector<T>::empty(); }
    T &       operator() (size_t i, size_t j)       { return (*this)[locate(i, j)]; }
    const T & operator() (size_t i, size_t j) const { return (*this)[locate(i, j)]; }
    void swap(matrix & other) throw() { std::swap(numcols, other.numcols); fixed_vector<T>::swap(other); }
};
template<class _T> inline void swap(matrix<_T> & L, matrix<_T> & R) throw() { L.swap(R); }

// TODO: get rid of these
typedef std::string STRING;
typedef std::wstring WSTRING;
typedef std::basic_string<TCHAR> TSTRING;    // wide/narrow character string
#endif

// derive from this for noncopyable classes (will get you private unimplemented copy constructors)
// ... TODO: change all of basetypes classes/structs to use this
class noncopyable
{
    noncopyable & operator= (const noncopyable &);
    noncopyable (const noncopyable &);
public:
    noncopyable(){}
};

class CCritSec
{
    CCritSec (const CCritSec &) = delete;
    CCritSec & operator= (const CCritSec &) = delete;
    std::mutex m_CritSec;
public:
    CCritSec() {};
    ~CCritSec() {};
    void Lock() { m_CritSec.lock(); };
    void Unlock() { m_CritSec.unlock(); };
};


// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class CAutoLock
{
    CAutoLock(const CAutoLock &refAutoLock); CAutoLock &operator=(const CAutoLock &refAutoLock);
    CCritSec & m_rLock;
public:
    CAutoLock(CCritSec & rLock) : m_rLock (rLock) { m_rLock.Lock(); };
    ~CAutoLock() { m_rLock.Unlock(); };
};


};};    // namespace

// ----------------------------------------------------------------------------
// frequently missing string functions
// ----------------------------------------------------------------------------

namespace msra { namespace strfun {

#ifndef BASETYPES_NO_STRPRINTF

// [w]strprintf() -- like sprintf() but resulting in a C++ string
template<class _T> struct _strprintf : public std::basic_string<_T>
{   // works for both wchar_t* and char*
    _strprintf (const _T * format, ...)
    {
        va_list args; 
		va_start (args, format);  // varargs stuff
        size_t n = _cprintf (format, args);     // num chars excl. '\0'
		va_end(args);
		va_start(args, format);
        const int FIXBUF_SIZE = 128;            // incl. '\0'
        if (n < FIXBUF_SIZE)
        {
            _T fixbuf[FIXBUF_SIZE];
            this->assign (_sprintf (&fixbuf[0], sizeof (fixbuf)/sizeof (*fixbuf), format, args), n);
        }
        else    // too long: use dynamically allocated variable-size buffer
        {
            std::vector<_T> varbuf (n + 1);     // incl. '\0'
            this->assign (_sprintf (&varbuf[0], varbuf.size(), format, args), n);
        }
    }
private:
    // helpers
    inline size_t _cprintf (const wchar_t * format, va_list args) 
	{ 
#ifdef __WINDOWS__
		return vswprintf (nullptr, 0, format, args);
#elif defined(__UNIX__)
        // TODO: Really??? Write to file in order to know the length of a string?
		FILE *dummyf = fopen("/dev/null", "w");
		if (dummyf == NULL)
			perror("The following error occurred in basetypes.h:cprintf");
		int n = vfwprintf (dummyf, format, args);
		if (n < 0)
			perror("The following error occurred in basetypes.h:cprintf");
		fclose(dummyf);
		return n;
#endif
	}
    inline size_t _cprintf(const  char   * format, va_list args)
	{ 
#ifdef __WINDOWS__
		return vsprintf_s(nullptr, 0, format, args);
#elif defined(__UNIX__)
        // TODO: Really??? Write to file in order to know the length of a string?
		FILE *dummyf = fopen("/dev/null", "wb");
		if (dummyf == NULL)
			perror("The following error occurred in basetypes.h:cprintf");
		int n = vfprintf (dummyf, format, args);
		if (n < 0)
			perror("The following error occurred in basetypes.h:cprintf");
		fclose(dummyf);
		return n;
#endif
	}
    inline const wchar_t * _sprintf(wchar_t * buf, size_t bufsiz, const wchar_t * format, va_list args) { vswprintf(buf, bufsiz, format, args); return buf; }
    inline const  char   * _sprintf ( char   * buf, size_t bufsiz, const char * format, va_list args) 
    {
#ifdef __WINDOWS__
        vsprintf_s(buf, bufsiz, format, args);
#else
        vsprintf(buf, format, args);
#endif
        return buf;
    }
};
typedef strfun::_strprintf<char>    strprintf;  // char version
typedef strfun::_strprintf<wchar_t> wstrprintf; // wchar_t version

#endif

#ifdef _WIN32
// string-encoding conversion functions
struct utf8 : std::string { utf8 (const std::wstring & p)    // utf-16 to -8
{
    size_t len = p.length();
    if (len == 0) { return;}    // empty string
    msra::basetypes::fixed_vector<char> buf (3 * len + 1);   // max: 1 wchar => up to 3 mb chars
    // ... TODO: this fill() should be unnecessary (a 0 is appended)--but verify
    std::fill (buf.begin (), buf.end (), 0);
    int rc = WideCharToMultiByte (CP_UTF8, 0, p.c_str(), (int) len,
                                  &buf[0], (int) buf.size(), NULL, NULL);
    if (rc == 0) RuntimeError("WideCharToMultiByte");
    (*(std::string*)this) = &buf[0];
}};
struct utf16 : std::wstring { utf16 (const std::string & p)  // utf-8 to -16
{
    size_t len = p.length();
    if (len == 0) { return;}    // empty string
    msra::basetypes::fixed_vector<wchar_t> buf (len + 1);
    // ... TODO: this fill() should be unnecessary (a 0 is appended)--but verify
    std::fill (buf.begin (), buf.end (), (wchar_t) 0);
    int rc = MultiByteToWideChar (CP_UTF8, 0, p.c_str(), (int) len,
                                  &buf[0], (int) buf.size());
    if (rc == 0) RuntimeError("MultiByteToWideChar");
    assert (rc < buf.size ());
    (*(std::wstring*)this) = &buf[0];
}};
#endif

// Note: generally, 8-bit strings in this codebase are UTF-8.
// One exception are functions that take 8-bit pathnames. Those will be interpreted by the OS as MBS. Best use wstring pathnames for all file accesses.
#pragma warning(push)
#pragma warning(disable : 4996) // Reviewed by Yusheng Li, March 14, 2006. depr. fn (wcstombs, mbstowcs)
static inline std::string wcstombs(const std::wstring & p)  // output: MBCS
{
    size_t len = p.length();
    msra::basetypes::fixed_vector<char> buf(2 * len + 1); // max: 1 wchar => 2 mb chars
    std::fill(buf.begin(), buf.end(), 0);
    ::wcstombs(&buf[0], p.c_str(), 2 * len + 1);
    return std::string(&buf[0]);
}
static inline std::wstring mbstowcs(const std::string & p)  // input: MBCS
{
    size_t len = p.length();
    msra::basetypes::fixed_vector<wchar_t> buf(len + 1); // max: >1 mb chars => 1 wchar
    std::fill(buf.begin(), buf.end(), (wchar_t)0);
    //OACR_WARNING_SUPPRESS(UNSAFE_STRING_FUNCTION, "Reviewed OK. size checked. [rogeryu 2006/03/21]");
    ::mbstowcs(&buf[0], p.c_str(), len + 1);
    return std::wstring(&buf[0]);
}
#pragma warning(pop)

#ifdef _WIN32
static inline  cstring  utf8 (const std::wstring & p) { return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(p); }     // utf-16 to -8
static inline wcstring utf16 (const  std::string & p) { return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(p); } // utf-8 to -16
#else   // BUGBUG: we cannot compile the above on Cygwin GCC, so for now fake it using the mbs functions, which will only work for 7-bit ASCII strings
static inline std::string utf8 (const std::wstring & p) { return msra::strfun::wcstombs (p.c_str()); }   // output: UTF-8... not really
static inline std::wstring utf16 (const std::string & p) { return msra::strfun::mbstowcs(p.c_str()); }   // input: UTF-8... not really
#endif
static inline  cstring  utf8 (const  std::string & p) { return p; }     // no conversion (useful in templated functions)
static inline wcstring utf16 (const std::wstring & p) { return p; }

// convert a string to lowercase  --TODO: currently only correct for 7-bit ASCII
template<typename CHAR>
static inline void tolower_ascii (std::basic_string<CHAR> & s) { std::transform(s.begin(), s.end(), s.begin(), [] (CHAR c) { return (c >= 0 && c < 128) ? ::tolower(c) : c; }); }

// split and join -- tokenize a string like strtok() would, join() strings together
template<class _T> static inline std::vector<std::basic_string<_T>> split (const std::basic_string<_T> & s, const _T * delim)
{
    std::vector<std::basic_string<_T>> res;
    for (size_t st = s.find_first_not_of (delim); st != std::basic_string<_T>::npos; )
    {
        size_t en = s.find_first_of (delim, st +1);
        if (en == std::basic_string<_T>::npos) en = s.length();
        res.push_back (s.substr (st, en-st));
        st = s.find_first_not_of (delim, en +1);    // may exceed
    }
    return res;
}

template<class _T> static inline std::basic_string<_T> join (const std::vector<std::basic_string<_T>> & a, const _T * delim)
{
    std::basic_string<_T> res;
    for (int i = 0; i < (int) a.size(); i++)
    {
        if (i > 0) res.append (delim);
        res.append (a[i]);
    }
    return res;
}

#if 1
// parsing strings to numbers
static inline int toint (const wchar_t * s)
{
#ifdef _WIN32
    return _wtoi (s);   // ... TODO: check it
#else
    return (int)wcstol(s, 0, 10);
#endif
}
static inline int toint (const char * s)
{
    return atoi (s);    // ... TODO: check it
}
static inline int toint (const std::wstring & s) { return toint (s.c_str()); }

static inline double todouble (const char * s)
{
    char * ep;          // will be set to point to first character that failed parsing
    double value = strtod (s, &ep);
    if (*s == 0 || *ep != 0)
        RuntimeError("todouble: invalid input string");
    return value;
}

// TODO: merge this with todouble(const char*) above
static inline double todouble (const std::string & s)
{
    s.size();       // just used to remove the unreferenced warning
    
    double value = 0.0;

    // stod supposedly exists in VS2010, but some folks have compilation errors
    // If this causes errors again, change the #if into the respective one for VS 2010.
#if _MSC_VER > 1400 // VS 2010+
    size_t * idx = 0;
    value = std::stod (s, idx);
    if (idx) RuntimeError("todouble: invalid input string");
#else
    char *ep = 0;   // will be updated by strtod to point to first character that failed parsing
    value = strtod (s.c_str(), &ep);

    // strtod documentation says ep points to first unconverted character OR 
    // return value will be +/- HUGE_VAL for overflow/underflow
    if (ep != s.c_str() + s.length() || value == HUGE_VAL || value == -HUGE_VAL)
        RuntimeError("todouble: invalid input string");
#endif
    
    return value;
}

static inline double todouble (const std::wstring & s)
{
    wchar_t * endptr;
    double value = wcstod (s.c_str(), &endptr);
    if (*endptr) RuntimeError("todouble: invalid input string");
    return value;
}
#endif

// ----------------------------------------------------------------------------
// tokenizer -- utility for white-space tokenizing strings in a character buffer
// This simple class just breaks a string, but does not own the string buffer.
// ----------------------------------------------------------------------------

#if 1
class tokenizer : public std::vector<char*>
{
    const char * delim;
public:
    tokenizer (const char * delim, size_t cap) : delim (delim) { reserve (cap); }
    // Usage: tokenizer tokens (delim, capacity); tokens = buf; tokens.size(), tokens[i]
    void operator= (char * buf)
    {
        resize (0);

        // strtok_s not available on all platforms - so backoff to strtok on those
#ifdef _WIN32
        char * context; // for strtok_s()
        for (char * p = strtok_s (buf, delim, &context); p; p = strtok_s (NULL, delim, &context))
            push_back (p);
#else
        for (char * p = strtok (buf, delim); p; p = strtok (NULL, delim))
            push_back (p);
#endif   
    }
};
#endif

};};    // namespace

// ----------------------------------------------------------------------------
// wrappers for some basic types (files, handles, timer)
// ----------------------------------------------------------------------------

namespace msra { namespace basetypes {

// FILE* with auto-close; use auto_file_ptr instead of FILE*.
// Warning: do not pass an auto_file_ptr to a function that calls fclose(),
// except for fclose() itself.
class auto_file_ptr
{
    FILE * f;
    FILE * operator= (auto_file_ptr &); // can't ref-count: no assignment
    auto_file_ptr (auto_file_ptr &);
    // implicit close (destructor, assignment): we ignore error
    void close()  throw() { if (f) try { if (f != stdin && f != stdout && f != stderr) ::fclose (f); } catch (...) { } f = NULL; }
#pragma warning(push)
#pragma warning(disable : 4996)
    void openfailed (const std::string & path) { RuntimeError("auto_file_ptr: error opening file '" + path + "': " + strerror (errno)); }
#pragma warning(pop)
protected:
    friend int fclose (auto_file_ptr&); // explicit close (note: may fail)
    int fclose() { int rc = ::fclose (f); if (rc == 0) f = NULL; return rc; }
public:
    auto_file_ptr() : f (NULL) { }
    ~auto_file_ptr() { close(); }
#pragma warning(push)
#pragma warning(disable : 4996)
    auto_file_ptr(const char * path, const char * mode) { f = fopen(path, mode); if (f == NULL) openfailed(path); }
    auto_file_ptr (const wchar_t * wpath, const char * mode) { f = _wfopen (wpath, msra::strfun::utf16 (mode).c_str()); if (f == NULL) openfailed (msra::strfun::utf8 (wpath)); }
#pragma warning(pop)
    FILE * operator= (FILE * other) { close(); f = other; return f; }
    auto_file_ptr (FILE * other) : f (other) { }
    operator FILE * () const { return f; }
    FILE * operator->() const { return f; }
    void swap (auto_file_ptr & other)  throw() { std::swap (f, other.f); }
};
inline int fclose (auto_file_ptr & af) { return af.fclose(); }

#if 0
#ifdef _MSC_VER
// auto-closing container for Win32 handles.
// Pass close function if not CloseHandle(), e.g.
// auto_handle h (FindFirstFile(...), FindClose);
// ... TODO: the close function should really be a template parameter
template<class _H> class auto_handle_t
{
    _H h;
    BOOL (WINAPI_CC * close) (HANDLE);  // close function
    auto_handle_t operator= (const auto_handle_t &);
    auto_handle_t (const auto_handle_t &);
public:
    auto_handle_t (_H p_h, BOOL (WINAPI_CC * p_close) (HANDLE) = ::CloseHandle) : h (p_h), close (p_close) {}
    ~auto_handle_t() { if (h != INVALID_HANDLE_VALUE) close (h); }
    operator _H () const { return h; }
};
typedef auto_handle_t<HANDLE> auto_handle;
#endif

// like auto_ptr but calls freeFunc_p (type free_func_t) instead of delete to clean up
// minor difference - wrapped object is T, not T *, so to wrap a 
// T *, use auto_clean<T *>
// TODO: can this be used for simplifying those other classes?
template<class T,class FR = void> class auto_clean
{
    T it;
    typedef FR (*free_func_t)(T); 
    free_func_t freeFunc;                           // the function used to free the pointer
    void free() { if (it) freeFunc(it); it = 0; }
    auto_clean operator= (const auto_clean &);      // hide to prevent copy
    auto_clean (const auto_clean &);                // hide to prevent copy
public:
    auto_clean (T it_p, free_func_t freeFunc_p) : it (it_p), freeFunc (freeFunc_p) {}
    ~auto_clean() { free(); }
    operator T () { return it; }
    operator const T () const { return it; }
    T detach () { T tmp = it; it = 0; return tmp; } // release ownership of object
};
#endif

};};

namespace msra { namespace files {

// ----------------------------------------------------------------------------
// textreader -- simple reader for text files --we need this all the time!
// Currently reads 8-bit files, but can return as wstring, in which case
// they are interpreted as UTF-8 (without BOM).
// Note: Not suitable for pipes or typed input due to readahead (fixable if needed).
// ----------------------------------------------------------------------------

class textreader
{
    msra::basetypes::auto_file_ptr f;
    std::vector<char> buf;  // read buffer (will only grow, never shrink)
    int ch;                 // next character (we need to read ahead by one...)
    char getch() { char prevch = (char) ch; ch = fgetc (f); return prevch; }
public:
    textreader (const std::wstring & path) : f (path.c_str(), "rb") { buf.reserve (10000); ch = fgetc (f); }
    operator bool() const { return ch != EOF; } // true if still a line to read
    std::string getline()                       // get and consume the next line
    {
        if (ch == EOF) LogicError("textreader: attempted to read beyond EOF");
        assert (buf.empty());
        // get all line's characters --we recognize UNIX (LF), DOS (CRLF), and Mac (CR) convention
        while (ch != EOF && ch != '\n' && ch != '\r') buf.push_back (getch());
        if (ch != EOF && getch() == '\r' && ch == '\n') getch();    // consume EOLN char
        std::string line (buf.begin(), buf.end());
        buf.clear();
        return line;
    }
    std::wstring wgetline() { return msra::strfun::utf16 (getline()); }
};

};};

// ----------------------------------------------------------------------------
// functional-programming style helper macros (...do this with templates?)
// ----------------------------------------------------------------------------

#define foreach_index(_i,_dat) for (int _i = 0; _i < (int) (_dat).size(); _i++)
#define map_array(_x,_expr,_y) { _y.resize (_x.size()); foreach_index(_i,_x) _y[_i]=_expr(_x[_i]); }
#define reduce_array(_x,_expr,_y) { foreach_index(_i,_x) _y = (_i==0) ? _x[_i] : _expr(_y,_x[_i]); }

// ----------------------------------------------------------------------------
// frequently missing utility functions
// ----------------------------------------------------------------------------

namespace msra { namespace util {

#if 0
// to (slightly) simplify processing of command-line arguments.
// command_line args (argc, argv);
// while (args.has (1) && args[0][0] == '-') { option = args.shift(); process (option); }
// for (const wchar_t * arg = args.shift(); arg; arg = args.shift()) { process (arg); }
class command_line
{
    int num;
    const wchar_t ** args;
public:
    command_line (int argc, wchar_t * argv[]) : num (argc), args ((const wchar_t **) argv) { shift(); }
    inline int size() const { return num; }
    inline bool has (int left) { return size() >= left; }
    const wchar_t * shift() { if (size() == 0) return NULL; num--; return *args++; }
    const wchar_t * operator[] (int i) const { return (i < 0 || i >= size()) ? NULL : args[i]; }
};
#endif

// byte-reverse a variable --reverse all bytes (intended for integral types and float)
template<typename T> static inline void bytereverse (T & v)  throw()
{   // note: this is more efficient than it looks because sizeof (v[0]) is a constant
    char * p = (char *) &v;
    const size_t elemsize = sizeof (v);
    for (int k = 0; k < elemsize / 2; k++)  // swap individual bytes
        swap (p[k], p[elemsize-1 - k]);
}

// byte-swap an entire array
template<class V> static inline void byteswap (V & v)  throw()
{
    foreach_index (i, v)
        bytereverse (v[i]);
}

// execute a block with retry
// Block must be restartable.
// Use this when writing small files to those unreliable Windows servers.
// TODO: This will fail to compile under VS 2008--we need an #ifdef around this
template<typename FUNCTION> static void attempt (int retries, const FUNCTION & body)
{
    for (int attempt = 1; ; attempt++)
    {
        try
        {
            body();
            if (attempt > 1) fprintf (stderr, "attempt: success after %d retries\n", attempt);
            break;
        }
        catch (const std::exception & e)
        {
            if (attempt >= retries)
                throw;      // failed N times --give up and rethrow the error
            fprintf (stderr, "attempt: %s, retrying %d-th time out of %d...\n", e.what(), attempt+1, retries);
            ::Sleep (1000); // wait a little, then try again
        }
    }
}

};};    // namespace

template<class S> static inline void ZeroStruct (S & s) { memset (&s, 0, sizeof (s)); }

// ----------------------------------------------------------------------------
// machine dependent
// ----------------------------------------------------------------------------

#if 0
#define MACHINE_IS_BIG_ENDIAN (false)
#endif

using namespace msra::basetypes;    // for compatibility

//#pragma warning (pop)

#define EPSILON 1e-5
#define ISCLOSE(a, b, threshold) (abs(a - b) < threshold)?true:false

// why is this in basetypes.h?
template<class F>
static inline bool comparator(const pair<int, F>& l, const pair<int, F>& r)
{
    return l.second > r.second;
}

#ifdef _WIN32
// ----------------------------------------------------------------------------
// frequently missing Win32 functions
// ----------------------------------------------------------------------------

// strerror() for Win32 error codes
static inline std::wstring FormatWin32Error(DWORD error)
{
	wchar_t buf[1024] = { 0 };
	::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, "", error, 0, buf, sizeof (buf) / sizeof (*buf) - 1, NULL);
	std::wstring res(buf);
	// eliminate newlines (and spaces) from the end
	size_t last = res.find_last_not_of(L" \t\r\n");
	if (last != std::string::npos) res.erase(last + 1, res.length());
	return res;
}
#endif // _WIN32

// Very simple version of thread-safe stack. Add other functions as needed.
template<typename T>
class conc_stack
{
public:
    typedef typename std::stack<T>::value_type value_type;

    conc_stack() {}

    value_type pop_or_create(std::function<value_type()> factory)
    {
        std::lock_guard<std::mutex> g(m_locker);
        if (m_stack.size() == 0)
            return factory();
        auto res = std::move(m_stack.top());
        m_stack.pop();
        return res;
    }

    void push(const value_type& item)
    {
        std::lock_guard<std::mutex> g(m_locker);
        m_stack.push(item);
    }

    void push(value_type&& item)
    {
        std::lock_guard<std::mutex> g(m_locker);
        m_stack.push(std::forward<value_type>(item));
    }

public:
    conc_stack(const conc_stack&) = delete;
    conc_stack& operator=(const conc_stack&) = delete;
    conc_stack(conc_stack&&) = delete;
    conc_stack& operator=(conc_stack&&) = delete;

private:
    std::stack<value_type> m_stack;
    std::mutex m_locker;
};

namespace std
{
#if defined(__GNUC__)
// make_unique was added in GCC 4.9.0. Requires using -std=c++11.
#if !defined(__cpp_lib_make_unique)
    template<typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args)
    {
        return unique_ptr<T>(new T(forward<Args>(args)...));
    }
#endif
#endif
}

#endif    // _BASETYPES_
