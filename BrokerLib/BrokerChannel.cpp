#include "stdafx.h"
#include "BrokerChannel.h"

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
				realtime_buffer_ = std::make_unique<std::queue<std::vector<char>>>();

				th_channel_ = std::thread([this]()
				{
					const std::chrono::duration<double, std::milli> delay(10);

					while (!terminate_channel_)
					{
						if (!HasSubscriber())
						{
							std::this_thread::sleep_for(delay);
							continue;
						}

						const auto len = sizeof(models::BrokerMessage);
						std::vector<char> buff;
						if (Dequeue(buff))
						{
							SendDataToBackend(buff);
							continue;
						}

						if (realtime_buffer_->empty())
						{
							std::this_thread::sleep_for(delay);
							continue;
						}

						auto real_buff = realtime_buffer_->front();
						SendDataToBackend(real_buff);
						realtime_buffer_->pop();
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

			bool BrokerChannel::Enqueue(const std::vector<char>& raw_data) const
			{
				if (!IsSync() || !HasSubscriber())
					return file_repo_->Enqueue(raw_data);
				realtime_buffer_->push(raw_data);

				return true;
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
