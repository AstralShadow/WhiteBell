#include "../include/DisconnectedException.hpp"
#include <string>

using std::to_string;

namespace WhiteBell
{
    DisconnectedException::DisconnectedException()
    {
        // ctor
    }

    string DisconnectedException::what()
    {
        return "Whitebell lost connection.";
    }
}
