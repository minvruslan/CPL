// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// Copyright (c) 2017-2018 Minnibaev Ruslan <minvruslan@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CPL_EVENT_QUEUE_HPP
#define CPL_EVENT_QUEUE_HPP

#include "cpl.hpp"

NS_CPL_START

// ==================================================
// ========== Class EventQueue declaration ==========
// ==================================================

template<typename T>
class EventQueue {
public:
    EventQueue();
    ~EventQueue();

    EventQueue& operator=( const EventQueue& ) = delete;

    void push( T value );
    bool tryPop( T& value );
    std::shared_ptr<T> tryPop();
    bool isEmpty() const;
    uint64_t size() const;
    Event* getEventHandle();
private:
    mutable std::mutex queueMutex_;
    std::queue<T> queue_;
    Event* newElementEvent_;
};


// =================================================
// ========== Class EventQueue definition ==========
// =================================================

template<typename T>
inline EventQueue<T>::EventQueue() {
    newElementEvent_ = new Event;
    newElementEvent_->initializeEvent();
}

template<typename T>
inline EventQueue<T>::~EventQueue() {
    delete newElementEvent_; // How to check? Handle may be anywhere.
}

template<typename T>
inline void EventQueue<T>::push( T value ) {
    std::lock_guard<std::mutex> lock( queueMutex_ );
    queue_.push( value );
    if ( !newElementEvent_->isSignaled() ) {
        newElementEvent_->setEvent();
    }
}

template<typename T>
inline bool EventQueue<T>::tryPop( T& value ) {
    if ( !newElementEvent_->isSignaled() ) {
        return false;
    }
    else {
        std::lock_guard<std::mutex> lock( queueMutex_ );
        if ( !queue_.empty() ) {
            value = queue_.front();
            queue_.pop();
        }
        if ( queue_.empty() ) {
            newElementEvent_->resetEvent();
        }
        return true;
    }
}

template<typename T>
inline std::shared_ptr<T> EventQueue<T>::tryPop() {
    std::shared_ptr<T> result = NULL;

    if ( !newElementEvent_->isSignaled() ) {
        return result;
    }
    else {
        std::lock_guard<std::mutex> lock( queueMutex_ );
        if ( !queue_.empty() ) {
            result = std::make_shared<T>( queue_.front() );
            queue_.pop();

            if ( queue_.empty() ) {
                newElementEvent_->resetEvent();
            }

            return result;
        }
    }
}

template<typename T>
inline bool EventQueue<T>::isEmpty() const {
    return ( !newElementEvent_->isSignaled() );
}

template<typename T>
inline uint64_t EventQueue<T>::size() const {
    std::lock_guard<std::mutex> lock( queueMutex_ );
    return ( queue_.size() );
}

template<typename T>
inline Event* EventQueue<T>::getEventHandle() {
    return newElementEvent_;
}

NS_CPL_END

#endif // CPL_EVENT_QUEUE_HPP
