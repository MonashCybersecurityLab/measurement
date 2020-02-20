//
// Created by shangqi on 8/2/20.
//

#ifndef MEASUREMENT_RINGBUFFER_H
#define MEASUREMENT_RINGBUFFER_H

#include <atomic>
#include <cstddef>

using namespace std;

template<typename Element, size_t Size>
class Ringbuffer {
public:
    enum { Capacity = Size + 1 };

    Ringbuffer() : _tail(0), _head(0){}
    virtual ~Ringbuffer() {}

    bool push(const Element& item) {
        const auto current_tail = _tail.load();
        const auto next_tail = increment(current_tail);
        if(next_tail != _head.load())
        {
            _array[current_tail] = item;
            _tail.store(next_tail);
            return true;
        }

        return false;  // full queue
    }

    bool pop(Element& item)
    {
        const auto current_head = _head.load();
        if(current_head == _tail.load())
            return false;   // empty queue

        item = _array[current_head];
        _head.store(increment(current_head));
        return true;
    }

    bool isEmpty() const {
        return (_head.load() == _tail.load());
    }

    bool isFull() const {
        const auto next_tail = increment(_tail.load());
        return (next_tail == _head.load());
    }

    bool isLockFree() const {
        return (_tail.is_lock_free() && _head.is_lock_free());
    }

private:
    size_t increment(size_t idx) const {
        return (idx + 1) % Capacity;
    }

    atomic <size_t>  _tail;  // tail(input) index
    Element    _array[Capacity];
    atomic<size_t>   _head; // head(output) index

};


#endif //MEASUREMENT_RINGBUFFER_H
