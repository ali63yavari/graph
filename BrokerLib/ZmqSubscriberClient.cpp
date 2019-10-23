#include "stdafx.h"
#include "ZmqSubscriberClient.h"
namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace clients
			{
				ZmqSubscriberClient::ZmqSubscriberClient(const std::string& broker_ip, uint16_t broker_port,
					const std::string& topic,
					std::function<void(const models::BrokerMessage&)>
					callback) :
					callback_(std::move(callback)),
					terminate_reader_(false)
				{
					assert(!broker_ip.empty());
					assert(!topic.empty());
					assert(broker_port > 0);
					assert(callback != nullptr);

					context_ = std::make_shared<zmq::context_t>(1);
					subscriber_socket_ = std::make_shared<zmq::socket_t>(*(context_.get()), ZMQ_SUB);
					size_t len = topic.length();
					subscriber_socket_->getsockopt(ZMQ_SUBSCRIBE, (void*)topic.data(), &len);
				}

				void ZmqSubscriberClient::ReadThread() const
				{
					zmq::pollitem_t items[2] = {
						{static_cast<void*>(*(subscriber_socket_.get())), 0, ZMQ_POLLIN, 0},
					};

					while (!terminate_reader_)
					{
						if (zmq::poll(items, 1, 50) == -1)
							break; //  Interrupted

						if (items[0].revents & ZMQ_POLLIN)
						{
							zmq::message_t msg;
							subscriber_socket_->recv(msg);
							std::string topic = std::string(static_cast<char*>(msg.data()), msg.size());

							subscriber_socket_->recv(msg);
							auto data = static_cast<char*>(msg.data());
							auto len = msg.size();

							if (topic.empty())
								continue;						

							try
							{
								//convert raw data to BrokerMessage and call callback method
								models::BrokerMessage br_msg{};
								int pos = 0;
								br_msg.SetBytes(data, pos);
								callback_(br_msg);
							}
							catch (...)
							{
							}
						}
					}
				}
			}
		}
	}
}
