#include "config.h"
#include <cstdint>
#include <string>

using std::string;

namespace config{
    const bool quiet_mode = false;
    const uint8_t max_pending_connections = 10;


    string get_sock_location(){
        return "./whitebell.sock";
    }
}
