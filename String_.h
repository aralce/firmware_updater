#pragma once
#include <string>
#include <cstring>

class String_: public std::string {
public:
    String_();
    String_ (std::__cxx11::basic_string<char> s);
    String_ (char const* s);
    String_(const unsigned char value, unsigned char base = 10);
    String_(const int value, unsigned char base = 10);
    String_(const unsigned int value, unsigned char base = 10);
    String_(const long value, unsigned char base = 10);

    bool concat (const char* s);

    template<typename S>
    bool concat (S s);

    int indexOf (const char c, size_t from = 0) const;

    template<typename S>
    int indexOf (S s, size_t from = 0) const;

    template<typename S>
    int lastIndexOf (S s, int from = npos) const;

    char charAt(int n) const;

    String_ substring (size_t pos = 0, size_t len = npos) const;

    String_ trim ();

    void replace (const String_& to_replace_with, const String_& to_replace);

    bool startsWith(const char* s) const;

    bool startsWith(const String_ s) const;

    template<typename S>
    bool startsWith(S s) const;

    bool endsWith(const char* s) const;
    bool endsWith(const String_ s) const;

    template<typename S>
    bool endsWith(S s) const;
    
    template<typename S>
    bool equals(S s) const;

    bool equalsIgnoreCase(const String_& s) const;

    void toUpperCase();

    long toInt() const;

    String_ operator+(String_& s) const;

    template<typename S>
    String_ operator+(S& s);

    bool operator!() const;
};