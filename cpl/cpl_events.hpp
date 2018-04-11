/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2017-2018 Minnibaev Ruslan <minvruslan@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <vector>
#include <cstdint>
#include <iostream>

#if CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
    #include <sys/eventfd.h>
    #include <sys/epoll.h>
#endif

#include "cpl_sockets.hpp"

namespace cpl {
	namespace events {
		class Event {
		public:
			Event();
			~Event();
			bool initializeEvent();
            		bool initializeEventFromSocketHandle(int32_t socketHandle) {
                		handle_ = socketHandle;
            		}
	    		bool initializeEvent(cpl::sockets::UdpSocket& udpSocket);
			bool setEvent();
			bool isSignaled() const;
			bool resetEvent();

		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			WSAEVENT getHandle() const;
        	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		int32_t getHandle() const;
		#endif
		private:
            		bool signaled_;
		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			WSAEVENT handle_;
		#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		int32_t handle_;
		#endif
		};

        	#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
        	constexpr WSAEVENT CPL_INVALID_EVENT = WSA_INVALID_EVENT;
		constexpr uint32_t CPL_WFE_INFINITE_WAIT = INFINITE;
		constexpr uint32_t CPL_WFE_EVENT_0_SIGNALED = WAIT_OBJECT_0;
		constexpr uint32_t CPL_WFE_TIME_IS_UP = WAIT_TIMEOUT;
		constexpr uint32_t CPL_WFE_FAILED = WAIT_FAILED;
        	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
        	constexpr int32_t  CPL_INVALID_EVENT = -1;
        	constexpr uint32_t CPL_WFE_INFINITE_WAIT = 0xffffffff;
        	constexpr uint32_t CPL_WFE_EVENT_0_SIGNALED = 0;
        	constexpr uint32_t CPL_WFE_TIME_IS_UP = 1001;
        	constexpr uint32_t CPL_WFE_FAILED = 1000;
        	#endif

		uint32_t waitForEvent(Event* event, uint32_t milliseconds = 0);
		uint32_t waitForEvents(std::vector<Event *> *events, bool waitAll, uint32_t milliseconds = 0);

		inline Event::Event() :
            		signaled_(false),
            		handle_(CPL_INVALID_EVENT)
        	{}

		inline Event::~Event() {
			if (handle_ != CPL_INVALID_EVENT) {
            	#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
                		::WSACloseEvent(handle_);
            	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
                		close(handle_);
            	#endif
            		}
		}


		inline bool Event::initializeEvent() {
		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
            		handle_ = ::WSACreateEvent();
        	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		handle_ = eventfd(0, EFD_NONBLOCK);
        	#endif
            		return handle_ != CPL_INVALID_EVENT;
		}

		// FIX
		inline bool Event::initializeEvent(cpl::sockets::UdpSocket& udpSocket) {
		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			if (!initializeEvent())
				return false;
			if (::WSAEventSelect(udpSocket.getHandle(), handle_, FD_READ))
				return false;
			return true;
		#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		if(!udpSocket.isOpen())
				return false;
			handle_ = udpSocket.getHandle();
			return true;
		#endif
		}

		inline bool Event::setEvent() {
            		if (handle_ == CPL_INVALID_EVENT) {
                		signaled_ = false;
                		return false;
            		}
            		signaled_ = true;
		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
            		uint32_t result = ::WSASetEvent(handle_);
			if (result == false)
				signaled_ = false;
			return result;
        	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		int32_t result =  eventfd_write(handle_, 1);
            		if (result == -1) {
                		signaled_ = false;
                		return false;
            		}
            		else {
                		return true;
            		}
		#endif
		}

		inline bool Event::isSignaled() const {
			return signaled_;
		}

		inline bool Event::resetEvent() {
            		if (!signaled_)
                		return true;
            		signaled_ = false;
            		if (handle_ == CPL_INVALID_EVENT)
                		return false;
		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			return ::WSAResetEvent(handle_);
		#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		eventfd_t eventfdt;
            		return eventfd_read(handle_, &eventfdt) != -1;
		#endif
		}

		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
		inline WSAEVENT Event::getHandle() const {
			return handle_;
		}
		#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
        	inline int32_t Event::getHandle() const {
            		return handle_;
        		}
		#endif

		inline uint32_t waitForEvent(Event* event, uint32_t milliseconds) {
		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			WSAEVENT* tempEvents = new WSAEVENT[1];
			tempEvents[0] = event->getHandle();

			uint32_t result;
			if (milliseconds == CPL_WFE_INFINITE_WAIT)
				result = ::WSAWaitForMultipleEvents(1, tempEvents, true, WSA_INFINITE, false);
			else
				result = ::WSAWaitForMultipleEvents(1, tempEvents, true, milliseconds, false);

			delete[] tempEvents;

			switch (result) {
			case WSA_WAIT_TIMEOUT:
				return CPL_WFE_TIME_IS_UP;
			case WSA_WAIT_FAILED:
				return CPL_WFE_FAILED;
			default:
				if ((result >= WSA_WAIT_EVENT_0) && (result <= WSA_WAIT_EVENT_0))
					return result;
				else
					return CPL_WFE_FAILED;
			}
		#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		int32_t epfd = epoll_create(1);
            		if (epfd < 0)
                		return 1000;

            		epoll_event epEvent;
            		epEvent.data.fd = event->getHandle();
            		epEvent.events = EPOLLIN;
            		epoll_ctl(epfd, EPOLL_CTL_ADD, event->getHandle(), &epEvent);
            		epoll_event* epEvents = new epoll_event[1];
            		int32_t result = epoll_wait(epfd, epEvents, 1, -1);

            		close(epfd);
            		delete[] epEvents;

            		if (result == -1)
                		return 1000;
            		else
                		return 1;
		#endif
		}

		inline uint32_t waitForEvents(std::vector<Event*>* events, bool waitAll, uint32_t milliseconds) {
		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			if (events->size() + 1 > WSA_MAXIMUM_WAIT_EVENTS)
				return CPL_WFE_FAILED;

			WSAEVENT* tempEvents = new WSAEVENT[events->size()];
			uint8_t i = 0;
			for (auto& h : *events) {
				tempEvents[i] = h->getHandle();
				i++;
			}

			uint32_t result;
			if (milliseconds == CPL_WFE_INFINITE_WAIT)
				result = ::WSAWaitForMultipleEvents(events->size(), tempEvents, waitAll, WSA_INFINITE, false);
			else
				result = ::WSAWaitForMultipleEvents(events->size(), tempEvents, waitAll, milliseconds, false);

			delete[] tempEvents;

			switch (result) {
			case WSA_WAIT_TIMEOUT:
				return CPL_WFE_TIME_IS_UP;
			case WSA_WAIT_FAILED:
				return CPL_WFE_FAILED;
			default:
				if ((result >= WSA_WAIT_EVENT_0) && (result <= WSA_WAIT_EVENT_0 + events->size() - 1))
					return result;
				else
					return CPL_WFE_FAILED;			
			}
        	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            		if(events->size() + 1 > 64)
                		return 1000;

            		int32_t epfd = epoll_create(events->size());

            		if (epfd < 0)
                		return 1000;

            		for(auto event : *events) {
                		epoll_event epEvent;
                		epEvent.data.fd = event->getHandle();
                		epEvent.events = EPOLLIN | EPOLLRDHUP;
                		epoll_ctl(epfd, EPOLL_CTL_ADD, event->getHandle(), &epEvent);
            		}

            		epoll_event* epEvents = new epoll_event[events->size()];
            		int32_t result;
            		if(milliseconds == CPL_WFE_INFINITE_WAIT)
                		result = epoll_wait(epfd, epEvents, events->size(), -1);
            		else
                		result = epoll_wait(epfd, epEvents, events->size(), milliseconds);
            		close(epfd);


            		if (result == -1) {
                		delete[] epEvents;
                		return CPL_WFE_FAILED;
            		}
            		else if(milliseconds != CPL_WFE_INFINITE_WAIT && result == 0) {
                		delete[] epEvents;
                		return CPL_WFE_TIME_IS_UP;
           		}
            		else {
                		uint32_t i = 0;
                		uint32_t eventNum = 0;
                		bool breakLoop = false;
                		for(auto event : *events) {
                    			for (i = 0; i < result; i++) {
                        			if (epEvents[i].data.fd == event->getHandle()) {
                            				breakLoop = true;
                            				break;
                        			}
                    			}

                    			if (breakLoop)
                        			break;
                    			else
                        			eventNum++;
                		}
                		delete[] epEvents;
                		return eventNum;
            		}
		#endif
		}
	}
}
