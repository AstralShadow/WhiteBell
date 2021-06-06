#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <string>
#include <cstdint>

using std::string;

namespace config{
    extern const bool quiet_mode;
    extern const uint8_t max_pending_connections;

    string get_sock_location();
    string get_host_ip();
    uint16_t get_host_port();
}

#endif // CONFIG_H_INCLUDED
