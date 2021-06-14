#ifndef CSTRING_T_HPP_INCLUDED
#define CSTRING_T_HPP_INCLUDED

#include <cstdint>
#include <cstddef>
#include <string>

using std::string;


namespace WhiteBell
{
    struct cstring_t
    {
        public:
            cstring_t();
            cstring_t(const char* const&, size_t);
            cstring_t(const char* const&);
            cstring_t(cstring_t const&);
            void operator = (cstring_t const&);
            virtual  ~cstring_t();

            const char* begin() const;
            const char* end() const;
            size_t size() const;

            operator const char*() const;
            operator string() const
            {
                return string(this->begin(), this->end());
            }

            cstring_t(string const& input) :
                _size(input.size()),
                text(new char[_size + 1])
            {
                copy(input.begin(), input.end(), this->text);
                this->text[_size] = '\0';
            }

        private:
            size_t _size;
            char* text;
    };
}

#endif // TYPES_HPP_INCLUDED
