#ifndef WHITEBELLCLIENT_HPP_INCLUDED
#define WHITEBELLCLIENT_HPP_INCLUDED

#include <cstdint>
#include "cstring_t.hpp"

#include "DisconnectedException.hpp"
#include "UnsupportedProtocolException.hpp"


namespace WhiteBell
{
    typedef void(*event_cb_t)(cstring_t);
    typedef void(*counter_cb_t)(uint32_t);

    class Client
    {
        public:
            Client(cstring_t const& path);
            Client(cstring_t const& ip, uint16_t const& port);
            Client(Client&) = delete;
            virtual ~Client();


            void fetch();
            void run();
            void stop(); // will stop after next received message.


            void set_namespace(cstring_t const& = "");


            void track_event(cstring_t const&, event_cb_t);
            void untrack_event(cstring_t const&, event_cb_t);
            void untrack_event(cstring_t const&);

            void add_event_listener(cstring_t const& name, event_cb_t callback)
            {
                this->track_event(name, callback);
            }
            void remove_event_listener(cstring_t const& name, event_cb_t callback)
            {
                this->untrack_event(name, callback);
            }
            void remove_event_listener(cstring_t const& name)
            {
                this->untrack_event(name);
            }


            void trigger_event(cstring_t const& name, cstring_t const& payload);
            void dispatch_event(cstring_t const& name, cstring_t const& payload)
            {
                this->trigger_event(name, payload);
            }


            void track_counter(cstring_t const&, counter_cb_t);
            void untrack_counter(cstring_t const&, counter_cb_t);
            void untrack_counter(cstring_t const&);

            void add_counter_listener(cstring_t const& name, counter_cb_t callback)
            {
                this->track_counter(name, callback);
            }
            void remove_counter_listener(cstring_t const& name, counter_cb_t callback)
            {
                this->untrack_counter(name, callback);
            }
            void remove_counter_listener(cstring_t const& name)
            {
                this->untrack_counter(name);
            }


            void join_counter(cstring_t const&);
            void leave_counter(cstring_t const&);

        private:
            struct Details;
            Details* details;
    };
}


#endif // WHITEBELLCLIENT_HPP_INCLUDED
