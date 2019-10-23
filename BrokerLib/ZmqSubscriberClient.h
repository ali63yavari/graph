#pragma once

#define ZMQ_STATIC
#include <zmq.hpp>
#include <functional>
#include "BrokerMessage.h"
#include <thread>

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

				private:
					std::shared_ptr<zmq::context_t> context_;
					std::shared_ptr<zmq::socket_t> subscriber_socket_;
					std::function<void(const models::BrokerMessage&)> callback_;
					bool terminate_reader_;
					std::thread th_read;

					void ReadThread() const;
				};
			}
		}
	}
}

