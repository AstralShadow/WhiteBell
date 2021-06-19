#ifndef CLIENT_HPP_INCLUDED
#define CLIENT_HPP_INCLUDED

#include <cstdint>
#include <string>

using std::string;

namespace WhiteBell
{
    typedef void(*event_cb_t)(string);
    typedef void(*counter_cb_t)(uint32_t);

    class Client
    {
        public:
            Client(string const& ip, uint16_t const& port);
            Client(Client&) = delete;
            virtual ~Client();


            void fetch();
            //void run();
            //void stop();


            void set_namespace(string const& = "");


            void track_event(string const&, event_cb_t);
            void untrack_event(string const&, event_cb_t);
            void untrack_event(string const&);

            void add_event_listener(string const& name, event_cb_t callback)
            {
                this->track_event(name, callback);
            }
            void remove_event_listener(string const& name, event_cb_t callback)
            {
                this->untrack_event(name, callback);
            }
            void remove_event_listener(string const& name)
            {
                this->untrack_event(name);
            }


            void trigger_event(string const& name, string const& payload);
            void dispatch_event(string const& name, string const& payload)
            {
                this->trigger_event(name, payload);
            }


            void track_counter(string const&, counter_cb_t);
            void untrack_counter(string const&, counter_cb_t);
            void untrack_counter(string const&);

            void add_counter_listener(string const& name, counter_cb_t callback)
            {
                this->track_counter(name, callback);
            }
            void remove_counter_listener(string const& name, counter_cb_t callback)
            {
                this->untrack_counter(name, callback);
            }
            void remove_counter_listener(string const& name)
            {
                this->untrack_counter(name);
            }


            void join_counter(string const&);
            void leave_counter(string const&);

        private:
            struct Details;
            Details* details;
    };
}


#endif // CLIENT_HPP_INCLUDED
