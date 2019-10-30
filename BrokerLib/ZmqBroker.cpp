#include "stdafx.h"
#include "ZmqBroker.h"
#include "StringBuilder.h"
#include <iostream>
#include <future>
#include <windows.h>
#include "BrokerMessage.h"

#define DEBUG_BROKER

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			ZmqBroker::ZmqBroker(const std::string& backend_ip,
				uint16_t backend_port,
				const std::string& frontend_ip,
				uint16_t frontend_port,
				const std::string& repository_path) :
				backend_ip_(backend_ip),
				backend_port_(backend_port),
				frontend_ip_(frontend_ip),
				frontend_port_(frontend_port),
				repository_path_(repository_path),
				terminate_proxy_(false)
			{
				assert(backend_port > 0);
				assert(frontend_port > 0);
				assert(frontend_port != backend_port);
				assert(!backend_ip.empty());
				assert(!frontend_ip.empty());
				assert(!repository_path.empty());
			}

			void ZmqBroker::Start()
			{
				//context_ = std::make_shared<zmq::context_t>(1);
				context_ = std::make_shared<zmq::context_t>(1);
				frontend_socket_ = std::make_shared<zmq::socket_t>(*context_, ZMQ_SUB);
				backend_socket_ = std::make_shared<zmq::socket_t>(*context_, ZMQ_XPUB);

				const std::string frontend_endpoint =
					extensions::StringBuilder() << "tcp://" << frontend_ip_ << ":" << frontend_port_;
				const std::string backend_endpoint =
					extensions::StringBuilder() << "tcp://" << backend_ip_ << ":" << backend_port_;

				frontend_socket_->bind(frontend_endpoint);
				backend_socket_->bind(backend_endpoint);

				//  Subscribe to every single topic from publisher
				frontend_socket_->setsockopt(ZMQ_SUBSCRIBE, "", 0);


				int buffer_size = 1024 * 1024 * 1024;				
				int rcv_hwm = 0;
				frontend_socket_->setsockopt(ZMQ_RCVBUF, &buffer_size, sizeof(buffer_size));
				frontend_socket_->setsockopt(ZMQ_RCVHWM, &rcv_hwm, sizeof(rcv_hwm));
				backend_socket_->setsockopt(ZMQ_SNDBUF, &buffer_size, sizeof(buffer_size));

				//proxy thread
				th_proxy_ = std::thread(&ZmqBroker::ProxyThread, this);
				SetThreadPriority(th_proxy_.native_handle(), THREAD_PRIORITY_HIGHEST);
			}

			void ZmqBroker::Stop()
			{
				terminate_proxy_ = true;
				th_proxy_.join();

				frontend_socket_->close();
				backend_socket_->close();
				context_->close();
			}

			ZmqBroker::~ZmqBroker() = default;

			BrokerChannel* ZmqBroker::GetChannel(const std::string& topic)
			{
				if (broker_channels_.count(topic))
					return broker_channels_[topic].get();

				broker_channels_.insert(
					std::make_pair(topic, std::make_shared<BrokerChannel>(repository_path_, topic, backend_socket_.get(), mutex_)));
				return broker_channels_[topic].get();
			}

			void ZmqBroker::ProxyThread()
			{
				zmq::pollitem_t items[2] = {
					{static_cast<void*>(*frontend_socket_), 0, ZMQ_POLLIN, 0},
					{static_cast<void*>(*backend_socket_), 0, ZMQ_POLLIN, 0}
				};

#ifdef DEBUG_BROKER
				long long last_id = 0;
#endif

				while (!terminate_proxy_)
				{
					if (zmq::poll(items, 2, 0) == -1)
						break; //  Interrupted
					
					if (items[1].revents & ZMQ_POLLIN)
					{
						zmq::message_t msg;

						backend_socket_->recv(msg);
						if (msg.empty())
							break;

						//  Event is one byte 0=unsub or 1=sub, followed by topic
						const auto event = static_cast<uint8_t *>(msg.data());

						std::string topic(reinterpret_cast<char *>(event + 1), msg.size() - 1);
						auto channel = GetChannel(topic);

						if (event[0] == 1)
						{
							channel->IncreaseSubscriber();
						}
						else
						{
							channel->DecreaseSubscriber();
						}
					}

					if (!(items[0].revents & ZMQ_POLLIN))
					{
						continue;
					}
					
					zmq::message_t msg;
					frontend_socket_->recv(msg);
					std::string topic = std::string(static_cast<char*>(msg.data()), msg.size());

					frontend_socket_->recv(msg);					

					if (topic.empty())
						continue;

#ifdef DEBUG_BROKER
					models::BrokerMessage br_msg{};
					int pos = 0;
					br_msg.SetBytes((char*)msg.data(), pos);

					if (last_id != br_msg.msg_id - 1)
					{
						std::cout << "ERROR:\t\t data lost at index: " << br_msg.msg_id << ", " << last_id << "\t# " << br_msg.msg_id - last_id << std::endl << std::endl;
					}

					last_id = br_msg.msg_id;
//#else

					std::vector<char> v(msg.size());
					memcpy(&(v.data()[0]), (char*)msg.data(), msg.size());

					auto t1 = std::chrono::high_resolution_clock::now();
					std::async(std::launch::async, [&](const std::vector<char>& data)
					{
						const auto channel = this->GetChannel(topic);
						channel->Enqueue(data);
					}, v);

					auto t2 = std::chrono::high_resolution_clock::now();
					auto d = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
#endif
				}
			}
		}
	}
}
