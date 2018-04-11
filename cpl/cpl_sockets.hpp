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
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "cpl.hpp"

namespace cpl {
	namespace sockets {
		class PortNum {
		public:
			PortNum();
			explicit PortNum(uint16_t portNum);
			bool setPortNum(uint16_t portNum);
			uint16_t getPortNum() const;
		private:
			uint16_t portNum_;
		};

		class IpAddress {
		public:
			IpAddress();
			bool setIp(std::string ip);
			bool setPortNum(PortNum portNum);
			std::string getIp() const;
			PortNum getPortNum() const;
		private:
			std::string ip_;
			PortNum portNum_;
		};

		constexpr uint32_t CPL_UDP_MESSAGE_MAX_SIZE = 65507;
		constexpr int32_t  CPL_INVALID_SOCKET = -1;
		constexpr int32_t  CPL_SOCKET_ERROR = -2;
		constexpr int32_t  CPL_INVALID_BUFFER = -3;

		class UdpSocket {
		public:
			UdpSocket();
			~UdpSocket();
			bool open(const uint16_t& port, const bool& nonBlockungModeFlag);
			bool isOpen() const;
			void close();
			int32_t receiveFrom(uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress);
			int32_t sendTo(uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress);

		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			SOCKET getHandle() const;
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            int32_t getHandle() const;
		#endif
		private:
        #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			SOCKET  socketHandle_;
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            int32_t socketHandle_;
        #endif
		};

		inline PortNum::PortNum() {
			portNum_ = 0;
		}

		inline PortNum::PortNum(uint16_t portNum) {
			portNum_ = portNum;
		}

		inline bool PortNum::setPortNum(uint16_t portNum) {
			if (portNum >= 49152 && portNum <= 65535) {
				portNum_ = portNum;
				return true;
			}
			else {
				return false;
			}
		}

		inline uint16_t PortNum::getPortNum() const {
			return portNum_;
		}

		inline IpAddress::IpAddress() :
			ip_(std::string("")),
			portNum_(0)
		{}

		inline bool IpAddress::setIp(std::string ip) {
            sockaddr_in sa;
            int32_t result =  inet_pton(AF_INET, ip.data(), &(sa.sin_addr));
            if (result != 0) {
				ip_ = ip;
				return true;
			}
            else {
				return false;
			}
		}

		inline bool IpAddress::setPortNum(PortNum portNum) {
			return portNum_.setPortNum(portNum.getPortNum());
		}

		inline std::string IpAddress::getIp() const {
			return ip_;
		}

		inline PortNum IpAddress::getPortNum() const {
			return portNum_;
		}

		inline UdpSocket::UdpSocket() {
        #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			socketHandle_ = INVALID_SOCKET;
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            socketHandle_ = 0;
        #endif
		}

		inline UdpSocket::~UdpSocket() {
			this->close();
		}

		inline bool UdpSocket::open(const uint16_t& port, const bool& nonBlockungModeFlag) {
        #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
            socketHandle_ = socket(AF_INET, SOCK_DGRAM, 0);

			if (socketHandle_ == INVALID_SOCKET)
				return false;

			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons((uint16_t)port);

			if (::bind(socketHandle_, (const sockaddr*)&address, sizeof(sockaddr_in)) == SOCKET_ERROR) {
				this->close();
				return false;
			}

			if (nonBlockungModeFlag) {
				DWORD nonBlocking = 1;
				if (ioctlsocket(socketHandle_, FIONBIO, &nonBlocking) != 0) {
					this->close();
					return false;
				}
			}

			return true;
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            socketHandle_ = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

            if ( socketHandle_ <= 0 )
                return false;

            sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons((uint16_t)port);

            if (::bind( socketHandle_, (const sockaddr*)&address, sizeof(sockaddr_in) ) < 0) {
                this->close();
                return false;
            }

            if (nonBlockungModeFlag) {
                int32_t nonBlocking = 1;
                if ( fcntl( socketHandle_, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 ) {
                    this->close();
                    return false;
                }
            }

            return true;
        #endif
		}

		inline bool UdpSocket::isOpen() const {
        #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			return (socketHandle_ != INVALID_SOCKET);
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            return socketHandle_ != 0;
        #endif
		}

		inline void UdpSocket::close() {
        #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			if (socketHandle_ != INVALID_SOCKET) {
				::closesocket(socketHandle_);
				socketHandle_ = INVALID_SOCKET;
			}
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            if (socketHandle_ != 0) {
                ::close(socketHandle_);
                socketHandle_ = 0;
            }
        #endif
		}

		inline int32_t UdpSocket::receiveFrom(uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress) {
        #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			if (socketHandle_ == INVALID_SOCKET)
				return CPL_INVALID_SOCKET;

			if (bufSize <= 0)
				return CPL_INVALID_BUFFER;

			sockaddr_in senderAddr;
			int senderAddrSize = sizeof(senderAddr);

			int32_t receivedBytes = recvfrom(socketHandle_,
											 reinterpret_cast<char*>(bufPtr),
											 bufSize,
											 0,
											 (SOCKADDR*)& senderAddr,
											 &senderAddrSize);

			if (receivedBytes != SOCKET_ERROR && receivedBytes >= 0)
				return receivedBytes;
			else
				return CPL_SOCKET_ERROR;
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            if (socketHandle_ == 0)
                return CPL_INVALID_SOCKET;

            if (bufSize <= 0)
                return CPL_INVALID_BUFFER;

            sockaddr_in senderAddr;
            socklen_t senderAddrSize = sizeof(senderAddr);

            int32_t receivedBytes = recvfrom(socketHandle_,
                                             reinterpret_cast<char*>(bufPtr),
                                             bufSize,
                                             0,
                                             (sockaddr*)&senderAddr,
                                             &senderAddrSize);

            if (receivedBytes >= 0)
                return receivedBytes;
            else
                return CPL_SOCKET_ERROR;
        #endif
		}

		inline int32_t UdpSocket::sendTo(uint8_t* bufPtr, uint16_t bufSize, IpAddress& ipAddress) {
        #if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
			if (socketHandle_ == INVALID_SOCKET)
				return CPL_INVALID_SOCKET;

			if (bufSize <= 0 || bufSize > CPL_UDP_MESSAGE_MAX_SIZE)
				return CPL_INVALID_BUFFER;
			
			HOSTENT *hostent;
			::sockaddr_in dest_addr_;
			dest_addr_.sin_family = AF_INET;
			dest_addr_.sin_port = htons((ipAddress.getPortNum()).getPortNum());
			if (inet_addr((ipAddress.getIp()).data()))
				dest_addr_.sin_addr.s_addr = inet_addr((ipAddress.getIp()).data());
			else if (hostent = gethostbyname((ipAddress.getIp()).data()))
				dest_addr_.sin_addr.s_addr = ((unsigned long**)hostent->h_addr_list)[0][0];

			int32_t bytesSend = ::sendto(socketHandle_,
										 reinterpret_cast<const char*>(bufPtr),
										 bufSize,
										 0,
										 (sockaddr*)&dest_addr_,
										 sizeof(sockaddr_in));
			
			if (bytesSend != SOCKET_ERROR && bytesSend >= 0)
				return bytesSend;
			else
				return CPL_SOCKET_ERROR;
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
            if (socketHandle_ == 0)
                return CPL_INVALID_SOCKET;

            if (bufSize <= 0 || bufSize > CPL_UDP_MESSAGE_MAX_SIZE)
                return CPL_INVALID_BUFFER;

            sockaddr_in destAddress;
            destAddress.sin_family = AF_INET;
            inet_aton(ipAddress.getIp().data(), &destAddress.sin_addr);
            destAddress.sin_port = htons(ipAddress.getPortNum().getPortNum());

            int32_t bytesSend = sendto(socketHandle_,
                                       reinterpret_cast<const char*>(bufPtr),
                                       bufSize,
                                       0,
                                       (sockaddr*)&destAddress,
                                       sizeof(sockaddr_in));

            if (bytesSend >= 0)
                return bytesSend;
            else
                return CPL_SOCKET_ERROR;
        #endif
		}

		#if CPL_PLATFORM == CPL_PLATFORM_WINDOWS_NT
		inline SOCKET UdpSocket::getHandle() const {
			return socketHandle_;
		}
        #elif CPL_PLATFORM == CPL_PLATFORM_LINUX_KERNEL
        inline int32_t UdpSocket::getHandle() const {
			return socketHandle_;
		}
		#endif
	}
}