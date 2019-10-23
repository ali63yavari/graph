#include "stdafx.h"
#include "ZmqPublisherClient.h"
#include "StringBuilder.h"

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace clients
			{
				ZmqPublisherClient::ZmqPublisherClient(const std::string& broker_ip, uint16_t broker_port)
				{
					assert(!broker_ip.empty());
					assert(broker_port > 0);

					context_ = std::make_shared<zmq::context_t>(1);
					publisher_socket_ = std::make_shared<zmq::socket_t>(*(context_.get()), ZMQ_PUB);
					const std::string endpoint = 
						extensions::StringBuilder() << "tcp://" << broker_ip << ":" << broker_port;
					
					publisher_socket_->connect(endpoint);
				}

				void ZmqPublisherClient::PublishMessage(const std::string& topic, models::BrokerMessage& msg) const
				{
					zmq::message_t message(topic.length());
					memcpy(message.data(), topic.c_str(), topic.length());

					int len = msg.GetSize();
					zmq::message_t data_message(len);
					std::vector<char> v(len);
					msg.GetBytes(&(v.data()[0]));
					memcpy(data_message.data(), reinterpret_cast<char*>(&(v.data()[0])), len);

					auto rc = publisher_socket_->send(message, zmq::send_flags::sndmore);
					rc = publisher_socket_->send(data_message, zmq::send_flags::none);
				}
			}
		}
	}
}
