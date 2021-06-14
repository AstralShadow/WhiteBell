#ifndef DISCONNECTEDEXCEPTION_H
#define DISCONNECTEDEXCEPTION_H

#include <string>
#include <exception>

using std::string;

namespace WhiteBell
{
    class DisconnectedException : std::exception
    {
        public:
            DisconnectedException() = default;
            virtual ~DisconnectedException() = default;
            virtual string what(){
                return "Lost connection to server.";
            }

        protected:

        private:
    };
}
#endif // DISCONNECTEDEXCEPTION_H
