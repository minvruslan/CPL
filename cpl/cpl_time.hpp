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

#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>

namespace cpl {
    namespace time {
        inline std::string getCurrentIso8601Time() {
            std::time_t now;
            std::time(&now);
            struct tm* timeinfo = localtime(&now);

            char buf[sizeof("YYYY-MM-DDThh:mm:ss+hh:mm")];

            strftime(buf, sizeof(buf),"%FT%T%z", timeinfo);
            buf[24] = buf[23];
            buf[23] = buf[22];
            buf[22] = ':';

            return std::string(buf, sizeof(buf) - 1);
        }

        inline std::time_t getCurrentUnixTime() {
            std::time_t now;
            std::time(&now);
            return now;
        }

        inline int64_t getCurrentUnixTimeMilliseconds() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        inline void convertUnixToIso8601Time(std::time_t unixTime, std::string& iso8601Time) {
            iso8601Time.clear();

            struct tm* timeinfo = localtime(&unixTime);

            char buf[sizeof("YYYY-MM-DDThh:mm:ss+hh:mm")];

            strftime(buf, sizeof(buf),"%FT%T%z", timeinfo);
            buf[24] = buf[23];
            buf[23] = buf[22];
            buf[22] = ':';

            iso8601Time = std::string(buf, sizeof(buf) - 1);
        }

        inline void convertIso8601ToUnixTime(std::string iso8601Time, std::time_t& unixTime) {
            iso8601Time[22] = iso8601Time[23];
            iso8601Time[23] = iso8601Time[24];
            iso8601Time.pop_back();

            struct tm t;
            memset((void*)&t, 0, sizeof(t));
            strptime(iso8601Time.data(), "%FT%T%z", &t);
            unixTime = mktime(&t);
        }
    }
}
