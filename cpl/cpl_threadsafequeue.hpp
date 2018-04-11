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

#include <memory>
#include <mutex>
#include <queue>

#include "cpl_events.hpp"

namespace cpl {
	namespace threadsafequeue {
		template<typename T>
		class ThreadsafeQueue {
		public:
			ThreadsafeQueue();
			ThreadsafeQueue& operator=(const ThreadsafeQueue&) = delete;
			~ThreadsafeQueue();
			void push(T newValue);
			bool tryPop(T& value);
			std::shared_ptr<T> tryPop();
			bool isEmpty() const;
			uint32_t size() const;
			cpl::events::Event* getEventHandle();
		private:
			mutable std::mutex queueMutex_;
			std::queue<T> dataQueue_;
			cpl::events::Event* newElementEvent_;
		};

		template<typename T>
		inline ThreadsafeQueue<T>::ThreadsafeQueue() {
			newElementEvent_ = new cpl::events::Event;
			newElementEvent_->initializeEvent();
		}

		template<typename T>
		inline ThreadsafeQueue<T>::~ThreadsafeQueue() {
			delete newElementEvent_; // How to check? Handle may be anywhere.
		}

		template<typename T>
		inline void ThreadsafeQueue<T>::push(T newValue) {
			std::lock_guard<std::mutex> lock(queueMutex_);
			dataQueue_.push(newValue);
			if (!newElementEvent_->isSignaled()) {
				newElementEvent_->setEvent();
			}
		}

		template<typename T>
		inline bool ThreadsafeQueue<T>::tryPop(T& value) {
			if (!newElementEvent_->isSignaled()) {
				return false;
			}
			else {
				std::lock_guard<std::mutex> lock(queueMutex_);
				if (!dataQueue_.empty()) {	// for sure
					value = dataQueue_.front();
					dataQueue_.pop();
				}
				if (dataQueue_.empty())
					newElementEvent_->resetEvent();
				return true;
			}
		}

		template<typename T>
		inline std::shared_ptr<T> ThreadsafeQueue<T>::tryPop() {
			std::shared_ptr<T> result = NULL;
			if (!newElementEvent_->isSignaled()) {
				return result;
			}
			else {
				std::lock_guard<std::mutex> lock(queueMutex_);
				if (!dataQueue_.empty()) {	// for sure
					result = std::make_shared<T>(dataQueue_.front());
					dataQueue_.pop();
					if (dataQueue_.empty())
						newElementEvent_->resetEvent();
					return result;
				}
			}
		}

		template<typename T>
		inline bool ThreadsafeQueue<T>::isEmpty() const {
			return newElementEvent_->isSignaled() ? false : true;
		}

		template<typename T>
		inline uint32_t ThreadsafeQueue<T>::size() const {
			std::lock_guard<std::mutex> lock(queueMutex_);
			return dataQueue_.size();
		}

		template<typename T>
		inline cpl::events::Event* ThreadsafeQueue<T>::getEventHandle() {
			return newElementEvent_;
		}
	}
}

