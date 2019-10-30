#include "stdafx.h"
#include "BrokerChannel.h"
#include <iostream>

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			BrokerChannel::BrokerChannel(const std::string& path, 
				const std::string& topic, 
				zmq::socket_t* backend_socket,
				std::shared_mutex& mutex) :
				topic_(topic),
				path_(path),
				subscriber_count_(0),
				backend_socket_(backend_socket),
				terminate_channel_(false),
				mutex_(mutex)
			{
				file_repo_ = std::make_shared<repositories::FileRepository>(path, topic + ".bin");				

				
				th_persist_ = std::thread([this]()
				{
					std::chrono::milliseconds timestamp(1);

					while (true)
					{
						if (raw_msgs.empty())
						{
							std::this_thread::sleep_for(timestamp);
							continue;
						}

						std::unique_lock<std::shared_mutex> lock(q_mutex_);
						auto msg = raw_msgs.front();
						Persist((char*)(&msg.data()[0]), msg.size());
						raw_msgs.pop();
					}
				});
				th_channel_ = std::thread([this]()
				{
					const std::chrono::duration<double, std::milli> delay(1);

					while (!terminate_channel_)
					{
						if (!HasSubscriber())
						{
							std::this_thread::sleep_for(delay);

							continue;
						}

						std::vector<char> buff;
						if (Dequeue(buff))
						{
							SendDataToBackend(buff);
							continue;
						}

					}
				});
			}

			BrokerChannel::~BrokerChannel()
			{
				terminate_channel_ = true;
				th_channel_.join();
			}

			bool BrokerChannel::IsSync() const
			{
				return file_repo_->Empty();
			}

			bool BrokerChannel::Persist(const char* raw_data, int len) const
			{				
				return file_repo_->Enqueue(raw_data, len);				
			}

			void BrokerChannel::Enqueue(const std::vector<char>& raw_msg)
			{				
				auto len = raw_msg.size();
				if (len <= 0)
					return;

				std::unique_lock<std::shared_mutex> lock(q_mutex_);				
				raw_msgs.push(raw_msg);
			}


			void BrokerChannel::SendDataToBackend(const std::vector<char>& raw_data) const
			{
				std::unique_lock<std::shared_mutex> lock(mutex_);

				zmq::message_t message(topic_.length());
				memcpy(message.data(), topic_.c_str(), topic_.length());
				zmq::message_t data_message(raw_data.size());
				memcpy(data_message.data(), (char*)(&raw_data.data()[0]), raw_data.size());

				auto rc = backend_socket_->send(message, zmq::send_flags::sndmore);
				rc = backend_socket_->send(data_message, zmq::send_flags::none);
			}
		}
	}
}
