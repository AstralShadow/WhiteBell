#ifndef CLIENT_INPUTBUFFER_H
#define CLIENT_INPUTBUFFER_H

#include "Client.h"
#include <sys/socket.h>
#include <cstdint>
#include <vector>

using std::vector;

class Client::InputBuffer
{
    public:
        InputBuffer();
        InputBuffer(const InputBuffer&) = delete;
        void operator = (const InputBuffer&) = delete;
        virtual ~InputBuffer();

        size_t avaliable();
        vector<uint8_t> get();
        vector<uint8_t> get(size_t);
        vector<uint8_t> peek();
        vector<uint8_t> peek(size_t);

        void receive(ssize_t fd);

    protected:

    private:
        size_t size;
        uint8_t* buffer;
        uint8_t* read_ptr;
        uint8_t* write_ptr;

        size_t empty_space();
        void resize(size_t);
        void collect_garbage();

        void ensure_some_free_space();
        void receive_input(ssize_t fd);
        void shrink_if_mostly_unused();
};

#endif // CLIENT_INPUTBUFFER_H
