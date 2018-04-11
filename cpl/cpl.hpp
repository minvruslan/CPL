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

#define CPL_PLATFORM_WINDOWS_NT   1
#define CPL_PLATFORM_LINUX_KERNEL 2

#if defined(_WIN32)
	//#define CPL_PLATFORM CPL_PLATFORM_WINDOWS_NT
	#error New functions are not supported by Windows yet.
#elif defined(__linux__)
	#define CPL_PLATFORM CPL_PLATFORM_LINUX_KERNEL
#else
	#error Unknown platform.
#endif

#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
	#include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
	// smth
#else
	#error Unknown platform!
#endif

namespace cpl {
	bool initialize(void);
	void shutdown(void);

	inline bool initialize() {
	#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
		WORD wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;
		int err = WSAStartup(wVersionRequested, &wsaData);
		
		if (err != NO_ERROR)
			return false;
		else
			return true;
	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
        return true;
	#endif
	}

	inline void shutdown() {
    #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
		WSACleanup();
	#elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
		return;
	#endif
	}
}
