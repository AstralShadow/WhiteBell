#ifndef OPCODE_HPP_INCLUDED
#define OPCODE_HPP_INCLUDED

namespace WhiteBell
{
    namespace OPCode
    {
        enum client_code
        {
            set_namespace = 0,
            track_event = 1,
            untrack_event = 2,
            trigger_event = 3,
            track_counter = 4,
            untrack_counter = 5,
            join_counter = 6,
            leave_counter = 7
        };

        enum server_code
        {
            protocol_version = 0,
            event_triggered = 1,
            counter_changed = 2
        };
    }

}

#endif // OPCODE_HPP_INCLUDED
