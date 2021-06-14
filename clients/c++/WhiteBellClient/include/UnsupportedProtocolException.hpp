#ifndef UNSUPPORTEDPROTOCOLEXCEPTION_H
#define UNSUPPORTEDPROTOCOLEXCEPTION_H

#include <string>
#include <exception>

using std::string;

namespace WhiteBell
{
    class UnsupportedProtocolException : std::exception
    {
        public:
            UnsupportedProtocolException() = default;
            virtual ~UnsupportedProtocolException() = default;
            virtual string what(){
                return "Server uses unsupported protocol.";
            };

        protected:

        private:
    };
}
#endif // UNSUPPORTEDPROTOCOLEXCEPTION_H
