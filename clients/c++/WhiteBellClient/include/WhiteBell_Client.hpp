#ifndef WHITEBELLCLIENT_HPP_INCLUDED
#define WHITEBELLCLIENT_HPP_INCLUDED

#include "Client.hpp"
#include "cstring_t.hpp"
#include "DisconnectedException.hpp"
#include "UnsupportedProtocolException.hpp"


namespace WhiteBell{
    class cstring_t;
    class Client;
    class DisconnectedException;
    class UnsupportedProtocolException;

    typedef void(*event_cb_t)(cstring_t);
    typedef void(*counter_cb_t)(uint32_t);

};


#endif // WHITEBELLCLIENT_HPP_INCLUDED
