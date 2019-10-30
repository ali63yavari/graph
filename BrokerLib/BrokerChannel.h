#pragma once

#include <string>
#include <memory>
#include <queue>
#include <shared_mutex>

#define ZMQ_STATIC
#include "zmq.hpp"

#include "FileRepository.h"

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			class BrokerChannel
			{
			public:
				BrokerChannel(const std::string& path,
					const std::string& topic,
					zmq::socket_t* backend_socket,
					std::shared_mutex& mutex);
				~BrokerChannel();
				bool IsSync() const;
				void Enqueue(const std::vector<char>& raw_msg);
			
				void IncreaseSubscriber()
				{
					subscriber_count_++;
				}

				void DecreaseSubscriber()
				{
					if (subscriber_count_ > 0)
						subscriber_count_--;
				}

				bool HasSubscriber() const
				{
					return subscriber_count_ > 0;
				}

			private:
				std::string topic_;
				std::shared_ptr<repositories::FileRepository> file_repo_;
				std::string path_;
				int subscriber_count_;
				zmq::socket_t* backend_socket_;

				std::thread th_channel_;
				std::thread th_persist_;
				bool terminate_channel_;
				std::shared_mutex& mutex_;
				std::mutex mm_;
				std::shared_mutex q_mutex_;
				std::queue<std::vector<char>> raw_msgs;

				//std::unique_ptr<std::queue<std::vector<char>>> realtime_buffer_;
				bool Persist(const char* raw_data, int len) const;

				bool Dequeue(std::vector<char>& raw_data) const
				{
					return file_repo_->Dequeue(raw_data);
				}

				void SendDataToBackend(const std::vector<char>& raw_data) const;
			};
		}
	}
}
