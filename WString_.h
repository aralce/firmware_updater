/*
 WString.h - String library for Wiring & Arduino
 ...mostly rewritten by Paul Stoffregen...
 Copyright (c) 2009-10 Hernando Barragan.  All right reserved.
 Copyright 2011, Paul Stoffregen, paul@pjrc.com

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef String__class_h
#define String__class_h
#ifdef __cplusplus

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pgmspace.h>
#include <stdint.h>

// An inherited class for holding the result of a concatenation.  These
// result objects are assumed to be writable by subsequent concatenations.
class StringSumHelper_;

// an abstract class used as a means to proide a unique pointer type
// but really has no body
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

// The string class
class String_ {
        // use a function pointer to allow for "if (s)" without the
        // complications of an operator bool(). for more information, see:
        // http://www.artima.com/cppsource/safebool.html
        typedef void (String_::*StringIfHelperType)() const;
        void StringIfHelper() const {
        }

    public:
        // constructors
        // creates a copy of the initial value.
        // if the initial value is null or invalid, or if memory allocation
        // fails, the string will be marked as invalid (i.e. "if (s)" will
        // be false).
        String_(const char *cstr = "");
        String_(const char *cstr, unsigned int length);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
        String_(const uint8_t *cstr, unsigned int length) : String_((const char*)cstr, length) {}
#endif
        String_(const String_ &str);
        String_(const __FlashStringHelper *str);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
        String_(String_ &&rval);
        String_(StringSumHelper_ &&rval);
#endif
        explicit String_(char c);
        explicit String_(unsigned char, unsigned char base = 10);
        explicit String_(int, unsigned char base = 10);
        explicit String_(unsigned int, unsigned char base = 10);
        explicit String_(long, unsigned char base = 10);
        explicit String_(unsigned long, unsigned char base = 10);
        explicit String_(float, unsigned char decimalPlaces = 2);
        explicit String_(double, unsigned char decimalPlaces = 2);
        ~String_(void);

        // memory management
        // return true on success, false on failure (in which case, the string
        // is left unchanged).  reserve(0), if successful, will validate an
        // invalid string (i.e., "if (s)" will be true afterwards)
        unsigned char reserve(unsigned int size);
        inline unsigned int length(void) const {
            if(buffer()) {
                return len();
            } else {
                return 0;
            }
        }
        inline void clear(void) {
            setLen(0);
        }
        inline bool isEmpty(void) const {
            return length() == 0;
        }

        // creates a copy of the assigned value.  if the value is null or
        // invalid, or if the memory allocation fails, the string will be
        // marked as invalid ("if (s)" will be false).
        String_ & operator =(const String_ &rhs);
        String_ & operator =(const char *cstr);
        String_ & operator = (const __FlashStringHelper *str);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
        String_ & operator =(String_ &&rval);
        String_ & operator =(StringSumHelper_ &&rval);
#endif

        // concatenate (works w/ built-in types)

        // returns true on success, false on failure (in which case, the string
        // is left unchanged).  if the argument is null or invalid, the
        // concatenation is considered unsuccessful.
        unsigned char concat(const String_ &str);
        unsigned char concat(const char *cstr);
        unsigned char concat(const char *cstr, unsigned int length);
        unsigned char concat(const uint8_t *cstr, unsigned int length) {return concat((const char*)cstr, length);}
        unsigned char concat(char c);
        unsigned char concat(unsigned char c);
        unsigned char concat(int num);
        unsigned char concat(unsigned int num);
        unsigned char concat(long num);
        unsigned char concat(unsigned long num);
        unsigned char concat(float num);
        unsigned char concat(double num);
        unsigned char concat(const __FlashStringHelper * str);

        // if there's not enough memory for the concatenated value, the string
        // will be left unchanged (but this isn't signalled in any way)
        String_ & operator +=(const String_ &rhs) {
            concat(rhs);
            return (*this);
        }
        String_ & operator +=(const char *cstr) {
            concat(cstr);
            return (*this);
        }
        String_ & operator +=(char c) {
            concat(c);
            return (*this);
        }
        String_ & operator +=(unsigned char num) {
            concat(num);
            return (*this);
        }
        String_ & operator +=(int num) {
            concat(num);
            return (*this);
        }
        String_ & operator +=(unsigned int num) {
            concat(num);
            return (*this);
        }
        String_ & operator +=(long num) {
            concat(num);
            return (*this);
        }
        String_ & operator +=(unsigned long num) {
            concat(num);
            return (*this);
        }
        String_ & operator +=(float num) {
            concat(num);
            return (*this);
        }
        String_ & operator +=(double num) {
            concat(num);
            return (*this);
        }
        String_ & operator += (const __FlashStringHelper *str){
            concat(str);
            return (*this);
        }

        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, const String_ &rhs);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, const char *cstr);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, char c);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, unsigned char num);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, int num);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, unsigned int num);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, long num);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, unsigned long num);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, float num);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, double num);
        friend StringSumHelper_ & operator +(const StringSumHelper_ &lhs, const __FlashStringHelper *rhs);

        // comparison (only works w/ Strings and "strings")
        operator StringIfHelperType() const {
            return buffer() ? &String_::StringIfHelper : 0;
        }
        int compareTo(const String_ &s) const;
        unsigned char equals(const String_ &s) const;
        unsigned char equals(const char *cstr) const;
        unsigned char operator ==(const String_ &rhs) const {
            return equals(rhs);
        }
        unsigned char operator ==(const char *cstr) const {
            return equals(cstr);
        }
        unsigned char operator !=(const String_ &rhs) const {
            return !equals(rhs);
        }
        unsigned char operator !=(const char *cstr) const {
            return !equals(cstr);
        }
        unsigned char operator <(const String_ &rhs) const;
        unsigned char operator >(const String_ &rhs) const;
        unsigned char operator <=(const String_ &rhs) const;
        unsigned char operator >=(const String_ &rhs) const;
        unsigned char equalsIgnoreCase(const String_ &s) const;
        unsigned char equalsConstantTime(const String_ &s) const;
        unsigned char startsWith(const String_ &prefix) const;
        unsigned char startsWith(const char *prefix) const {
            return this->startsWith(String_(prefix));
        }
        unsigned char startsWith(const __FlashStringHelper *prefix) const {
            return this->startsWith(String_(prefix));
        }
        unsigned char startsWith(const String_ &prefix, unsigned int offset) const;
        unsigned char endsWith(const String_ &suffix) const;
        unsigned char endsWith(const char *suffix) const {
            return this->endsWith(String_(suffix));
        }
        unsigned char endsWith(const __FlashStringHelper * suffix) const {
            return this->endsWith(String_(suffix));
        }

        // character access
        char charAt(unsigned int index) const;
        void setCharAt(unsigned int index, char c);
        char operator [](unsigned int index) const;
        char& operator [](unsigned int index);
        void getBytes(unsigned char *buf, unsigned int bufsize, unsigned int index = 0) const;
        void toCharArray(char *buf, unsigned int bufsize, unsigned int index = 0) const {
            getBytes((unsigned char *) buf, bufsize, index);
        }
        const char* c_str() const { return buffer(); }
        char* begin() { return wbuffer(); }
        char* end() { return wbuffer() + length(); }
        const char* begin() const { return c_str(); }
        const char* end() const { return c_str() + length(); }

        // search
        int indexOf(char ch) const;
        int indexOf(char ch, unsigned int fromIndex) const;
        int indexOf(const String_ &str) const;
        int indexOf(const String_ &str, unsigned int fromIndex) const;
        int lastIndexOf(char ch) const;
        int lastIndexOf(char ch, unsigned int fromIndex) const;
        int lastIndexOf(const String_ &str) const;
        int lastIndexOf(const String_ &str, unsigned int fromIndex) const;
        String_ substring(unsigned int beginIndex) const {
            return substring(beginIndex, len());
        }
        ;
        String_ substring(unsigned int beginIndex, unsigned int endIndex) const;

        // modification
        void replace(char find, char replace);
        void replace(const String_ &find, const String_ &replace);
        void replace(const char *find, const String_ &replace) {
            this->replace(String_(find), replace);
        }
        void replace(const __FlashStringHelper *find, const String_ &replace) {
            this->replace(String_(find), replace);
        }
        void replace(const char *find, const char *replace) {
            this->replace(String_(find), String_(replace));
        }
        void replace(const __FlashStringHelper *find, const char *replace) {
            this->replace(String_(find), String_(replace));
        }
        void replace(const __FlashStringHelper *find, const __FlashStringHelper *replace) {
            this->replace(String_(find), String_(replace));
        }
        void remove(unsigned int index);
        void remove(unsigned int index, unsigned int count);
        void toLowerCase(void);
        void toUpperCase(void);
        void trim(void);

        // parsing/conversion
        long toInt(void) const;
        float toFloat(void) const;
	double toDouble(void) const;

    protected:
        // Contains the string info when we're not in SSO mode
        struct _ptr { 
            char *   buff;
            uint32_t cap;
            uint32_t len;
        };
        // This allows strings up up to 11 (10 + \0 termination) without any extra space.
        enum { SSOSIZE = sizeof(struct _ptr) + 4 - 1 }; // Characters to allocate space for SSO, must be 12 or more
        struct _sso {
            char     buff[SSOSIZE];
            unsigned char len   : 7; // Ensure only one byte is allocated by GCC for the bitfields
            unsigned char isSSO : 1;
        } __attribute__((packed)); // Ensure that GCC doesn't expand the flag byte to a 32-bit word for alignment issues
#ifdef BOARD_HAS_PSRAM
        enum { CAPACITY_MAX = 3145728 }; 
#else
        enum { CAPACITY_MAX = 65535 }; 
#endif
        union {
            struct _ptr ptr;
            struct _sso sso;
        };
        // Accessor functions
        inline bool isSSO() const { return sso.isSSO; }
        inline unsigned int len() const { return isSSO() ? sso.len : ptr.len; }
        inline unsigned int capacity() const { return isSSO() ? (unsigned int)SSOSIZE - 1 : ptr.cap; } // Size of max string not including terminal NUL
        inline void setSSO(bool set) { sso.isSSO = set; }
        inline void setLen(int len) {
            if (isSSO()) {
                sso.len = len;
                sso.buff[len] = 0;
            } else {
                ptr.len = len;
                if (ptr.buff) {
                    ptr.buff[len] = 0;
                }
            }
        }
        inline void setCapacity(int cap) { if (!isSSO()) ptr.cap = cap; }
        inline void setBuffer(char *buff) { if (!isSSO()) ptr.buff = buff; }
        // Buffer accessor functions
        inline const char *buffer() const { return (const char *)(isSSO() ? sso.buff : ptr.buff); }
        inline char *wbuffer() const { return isSSO() ? const_cast<char *>(sso.buff) : ptr.buff; } // Writable version of buffer

    protected:
        void init(void);
        void invalidate(void);
        unsigned char changeBuffer(unsigned int maxStrLen);

        // copy and move
        String_ & copy(const char *cstr, unsigned int length);
        String_ & copy(const __FlashStringHelper *pstr, unsigned int length);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
        void move(String_ &rhs);
#endif
};

class StringSumHelper_: public String_ {
    public:
        StringSumHelper_(const String_ &s) :
                String_(s) {
        }
        StringSumHelper_(const char *p) :
                String_(p) {
        }
        StringSumHelper_(char c) :
                String_(c) {
        }
        StringSumHelper_(unsigned char num) :
                String_(num) {
        }
        StringSumHelper_(int num) :
                String_(num) {
        }
        StringSumHelper_(unsigned int num) :
                String_(num) {
        }
        StringSumHelper_(long num) :
                String_(num) {
        }
        StringSumHelper_(unsigned long num) :
                String_(num) {
        }
        StringSumHelper_(float num) :
                String_(num) {
        }
        StringSumHelper_(double num) :
                String_(num) {
        }
};

extern const String_ emptyString_;

#endif  // __cplusplus
#endif  // String_class_h
