#pragma once

#define ZMQ_STATIC
#include <zmq.hpp>
#include <functional>
#include "BrokerMessage.h"
#include <thread>
#include <shared_mutex>

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace clients
			{
				class ZmqSubscriberClient
				{
				public:
					ZmqSubscriberClient(const std::string& broker_ip,
					                    uint16_t broker_port,
					                    const std::string& topic,
					                    std::function<void(const models::BrokerMessage&)> callback);

					~ZmqSubscriberClient() = default;

					void GetData(uint64_t& last_id, uint64_t& msg_counter)
					{
						std::unique_lock<std::shared_mutex> lock(mutex_);
						last_id = last_id_;
						msg_counter = msg_counter_;
					};

					bool WaitForDataChanged(int ms_timeout)
					{
						std::unique_lock<std::mutex> lk(m_);
						std::chrono::milliseconds dur(ms_timeout);
						std::chrono::time_point<std::chrono::system_clock> dt(std::chrono::system_clock::now() + dur);

						return cv_.wait_until(lk, dt) != std::cv_status::timeout;
					}
					

				private:
					std::shared_ptr<zmq::context_t> context_;
					std::shared_ptr<zmq::socket_t> subscriber_socket_;
					std::function<void(const models::BrokerMessage&)> callback_;
					bool terminate_reader_;
					std::thread th_read;
					uint64_t last_id_{0};
					uint64_t msg_counter_{ 0 };
					std::shared_mutex mutex_;
					std::mutex m_;
					std::condition_variable cv_;

					void SetData(uint64_t last_id, uint64_t msg_counter)
					{
						std::unique_lock<std::shared_mutex> lock(mutex_);

						last_id_ = last_id;
						msg_counter_ = msg_counter;

						cv_.notify_one();
					}

					void ReadThread();

				};
			}
		}
	}
}

