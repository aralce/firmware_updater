#include "String_.h"
#include <stdlib.h>

String_::String_() : std::string() {}

String_::String_ (std::__cxx11::basic_string<char> s) : std::string(s) {}

String_::String_ (char const* s) : std::string(s) {}

String_::String_(const unsigned char value, unsigned char base) {
    char buf[1 + 8 * sizeof(unsigned char)];
    utoa(value, buf, base);
    *this = buf;
}

String_::String_(const int value, unsigned char base) {
    char buf[2 + 8 * sizeof(int)];
    if (base == 10) {
        sprintf(buf, "%d", value);
    } else {
        itoa(value, buf, base);
    }
    *this = buf;
}

String_::String_(const unsigned int value, unsigned char base) {
    char buf[1 + 8 * sizeof(unsigned int)];
    utoa(value, buf, base);
    *this = buf;
}

static char* ltoa(long value, char* result, int base);

String_::String_(const long value, unsigned char base) {
    char buf[2 + 8 * sizeof(long)];
    if (base==10) {
        sprintf(buf, "%ld", value);
    } else {
        ltoa(value, buf, base);
    }
    *this = buf;
}

static void reverse(char* begin, char* end);

static char* ltoa(long value, char* result, int base) {
    if(base < 2 || base > 16) {
        *result = 0;
        return result;
    }

    char* out = result;
    long quotient = abs(value);

    do {
        const long tmp = quotient / base;
        *out = "0123456789abcdef"[quotient - (tmp * base)];
        ++out;
        quotient = tmp;
    } while(quotient);

    // Apply negative sign
    if(value < 0)
        *out++ = '-';

    reverse(result, out);
    *out = 0;
    return result;
}

static void reverse(char* begin, char* end) {
    char *is = begin;
    char *ie = end - 1;
    while(is < ie) {
        char tmp = *ie;
        *ie = *is;
        *is = tmp;
        ++is;
        --ie;
    }
}

bool String_::concat (const char* s) {
    return concat(s);
}

template<typename S>
bool String_::concat (S s) {
    this->append(s);
    return true;
}

int String_::indexOf (const char c, size_t from) const{
    return indexOf(c, from);
}

template<typename S>
int String_::indexOf (S s, size_t from) const{
    int index = this->find(s, from); 
    return index == npos ? -1 : index;
}

template<typename S>
int String_::lastIndexOf(S s, int from) const{
    int index = this->find_last_of(s, from);
    return index == npos ? -1 : index;
}

char String_::charAt(int n) const {
    return this->at(n);
}

String_ String_::substring (size_t pos, size_t len) const{
    std::string s = this->substr(pos, len);
    String_ str(s);
    return str;
}

String_ String_::trim () {
    int i = 0;
    for (; i < this->length(); i++) {
        if (not isblank(this->at(i)))
            break;
    }
    String_ new_string;
    for (; i < this->length(); i++) {
        new_string += this->at(i);
    }
    
    int count_of_trailing_spaces = 0;
    for (int i = this->length() - 1; i >= 0; --i) {
        if(isblank(this->at(i)))
            count_of_trailing_spaces += 1;
        else
            break;
    }
    String_ string_without_trailing_spaces(new_string.substring(0, count_of_trailing_spaces));
    *this = string_without_trailing_spaces;
}

void String_::replace (const String_& to_replace, const String_& replace_with) {
    size_t starting_index = this->find(to_replace);
    while(starting_index != std::string::npos) {
        std::string::replace(starting_index, to_replace.length(), replace_with);
        starting_index  = this->find(to_replace);
    }
}

bool String_::startsWith(const char* s) const{
    return startsWith(s);
}

bool String_::startsWith(const String_ s) const{
    return startsWith(s);
}

template<typename S>
bool String_::startsWith(S s) const{
    int index = this->find_first_of(s);
    return index == 0 ? true : false;
}

bool String_::endsWith(const char* s) const{
    return endsWith(s);
}

bool String_::endsWith(const String_ s) const{
    return endsWith(s);
}

template<typename S>
bool String_::endsWith(S s) const{
    int index = lastIndexOf(s);
    return index == this->length() - 1 ? true : false;
}

template<typename S>
bool String_::equals(S s) const{
    return *this == s;
}

bool String_::equalsIgnoreCase(const String_& s) const{
    String_ a = (*this);
    String_ b = s;
    
    if (a.length() != b.length())
        return false;
    
    for (int i = 0; i < a.length(); i++) {
        tolower(a[i]);
        tolower(b[i]);
    }

    return a == b;
}

void String_::toUpperCase() {
    for (int i = 0; i < this->length(); i++)
        (*this)[i] = toupper(this->at(i));
}

long String_::toInt() const{
    return std::stoi((*this));
}

String_ String_::operator+(String_& s) const {
    return operator+(s);
}

template<typename S>
String_ String_::operator+(S& s) {
    concat(s);
    return (*this);
}

bool String_::operator!() const {
    return not this->empty();
}