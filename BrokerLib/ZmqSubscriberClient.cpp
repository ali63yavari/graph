#include "stdafx.h"
#include "ZmqSubscriberClient.h"
#include "StringBuilder.h"
#include <iostream>

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
					//assert(callback != nullptr);

					context_ = std::make_shared<zmq::context_t>(1);
					subscriber_socket_ = std::make_shared<zmq::socket_t>(*context_, ZMQ_SUB);
					std::string str = topic;
					size_t len = str.size() * sizeof(str[0]);

					const std::string endpoint =
						extensions::StringBuilder() << "tcp://" << broker_ip << ":" << broker_port;

					int buffer_size = 1024 * 1024 * 1024;
					subscriber_socket_->setsockopt(ZMQ_RCVBUF, &buffer_size, sizeof(buffer_size));
					subscriber_socket_->setsockopt(ZMQ_SNDBUF, &buffer_size, sizeof(buffer_size));

					subscriber_socket_->connect(endpoint);
					const int rc = zmq_setsockopt(subscriber_socket_->handle(), ZMQ_SUBSCRIBE, str.c_str(), str.length());
					if (rc != 0)
						throw zmq::error_t();

					th_read = std::thread([&]()
					{
						ReadThread();
					});
				}

				void ZmqSubscriberClient::ReadThread()
				{
					zmq::pollitem_t items[1] = {
						{static_cast<void*>(*subscriber_socket_), 0, ZMQ_POLLIN, 0},
					};

					long long last_id = 0;
					long long msg_counter = 0;

					while (!terminate_reader_)
					{
						if (zmq::poll(items, 1, 10) == -1)
							break; //  Interrupted

						if (items[0].revents & ZMQ_POLLIN)
						{
							zmq::message_t msg;
							subscriber_socket_->recv(msg);
							std::string topic = std::string(static_cast<char*>(msg.data()), msg.size());

							subscriber_socket_->recv(msg);
							auto data = static_cast<char*>(msg.data());

							if (topic.empty())
								continue;
							

							models::BrokerMessage br_msg{};
							int pos = 0;
							br_msg.SetBytes(data, pos);
							
							last_id = br_msg.msg_id;
							msg_counter++;

							SetData(last_id, msg_counter);
						}
					}
				}
			}
		}
	}
}
