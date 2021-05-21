#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <string>
#include <cstdint>

using std::string;

namespace config{
    extern const bool quiet_mode;
    extern const uint8_t max_pending_connections;

    string get_sock_location();
}

#endif // CONFIG_H_INCLUDED
