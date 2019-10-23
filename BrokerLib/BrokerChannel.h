#pragma once

#include <string>
#include <memory>
#include <queue>
#include <shared_mutex>

#define ZMQ_STATIC
#include "zmq.hpp"

#include "FileRepository.h"
#include "BrokerMessage.h"
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
				bool Enqueue(const std::vector<char>& raw_data) const;
			
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
				const std::string& topic_;
				std::shared_ptr<repositories::FileRepository> file_repo_;
				const std::string& path_;
				int subscriber_count_;
				zmq::socket_t* backend_socket_;

				std::thread th_channel_;
				bool terminate_channel_;
				std::shared_mutex& mutex_;

				std::unique_ptr<std::queue<std::vector<char>>> realtime_buffer_;

				bool Dequeue(std::vector<char> raw_data) const
				{
					return file_repo_->Dequeue(raw_data);
				}

				void SendDataToBackend(const std::vector<char>& raw_data) const;
			};
		}
	}
}
