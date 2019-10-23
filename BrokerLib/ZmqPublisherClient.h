#pragma once
#include "BrokerMessage.h"
#include <string>

#define ZMQ_STATIC
#include "zmq.hpp"

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace clients
			{
				class ZmqPublisherClient
				{
				public:
					ZmqPublisherClient(const std::string& broker_ip, uint16_t broker_port);
					~ZmqPublisherClient() = default;

					void PublishMessage(const std::string& topic, models::BrokerMessage& msg) const;

				private:
					std::shared_ptr<zmq::socket_t> publisher_socket_;
					std::shared_ptr<zmq::context_t> context_;
				};
			}
		}
	}
}
