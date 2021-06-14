#ifndef INPUTBUFFER_H
#define INPUTBUFFER_H

#include <sys/socket.h>
#include <cstdint>
#include <vector>

using std::vector;

namespace WhiteBell
{
    /** The same class that the server uses. */
    class InputBuffer
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

            void receive(ssize_t fd, uint32_t flags = 0);

        protected:

        private:
            size_t size = 512;
            uint8_t* buffer = nullptr;
            uint8_t* read_ptr = nullptr;
            uint8_t* write_ptr = nullptr;

            size_t empty_space();
            void resize(size_t);
            void collect_garbage();

            void ensure_some_free_space();
            void receive_input(ssize_t fd, uint32_t);
            void shrink_if_mostly_unused();
    };
}

#endif // INPUTBUFFER_H
