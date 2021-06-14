#include "../include/cstring_t.hpp"
#include <cstring>
#include <memory>

using std::copy;

namespace WhiteBell
{
    cstring_t::cstring_t() :
        _size(0),
        text(nullptr)
    {

    }

    cstring_t::cstring_t(const char* const& _text, size_t __size) :
        _size(__size),
        text(new char[_size + 1])
    {
        copy(_text, _text + __size, this->text);
        this->text[_size] = '\0';
    }

    cstring_t::cstring_t(const char* const& _text) :
        _size(strlen(_text)),
        text(new char[_size + 1])
    {
        copy(_text, _text + _size, this->text);
        this->text[_size] = '\0';
    }

    cstring_t::cstring_t(cstring_t const& other) :
        _size(other.size()),
        text(new char[_size + 1])
    {
        copy(other.begin(), other.end(), this->text);
        this->text[_size] = '\0';
    }

    void cstring_t::operator = (cstring_t const& other)
    {
        delete this->text;
        this->_size = other.size();
        this->text = new char[_size + 1];
        copy(other.begin(), other.end(), this->text);
        this->text[_size] = '\0';
    }

    cstring_t::~cstring_t()
    {
        delete this->text;
    }

    const char* cstring_t::begin() const
    {
        return this->text;
    }

    const char* cstring_t::end() const
    {
        return this->text + this->_size;
    }

    size_t cstring_t::size() const
    {
        return this->_size;
    }

    cstring_t::operator const char*() const
    {

        return this->text;
    }
}
