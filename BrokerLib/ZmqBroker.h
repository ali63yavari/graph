#pragma once
#include <string>
#include <unordered_map>
#include <queue>
#include <chrono>

#define ZMQ_STATIC
#include "zmq.hpp"

#include "BrokerChannel.h"
#include "FileRepository.h"

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			class ZmqBroker
			{
			public:
				ZmqBroker(const std::string& backend_ip,
				          uint16_t backend_port,
				          const std::string& frontend_ip,
				          uint16_t frontend_port,
				          const std::string& repository_path);
				void Start();
				void Stop();
				~ZmqBroker();

			private:
				std::string backend_ip_;
				uint16_t backend_port_;
				std::string frontend_ip_;
				uint16_t frontend_port_;
				std::string repository_path_;
				std::thread th_proxy_;
				bool terminate_proxy_;
				std::shared_mutex mutex_;

				std::shared_ptr<zmq::context_t> context_;
				std::shared_ptr<zmq::socket_t> frontend_socket_;
				std::shared_ptr<zmq::socket_t> backend_socket_;
				std::unordered_map<std::string, std::shared_ptr<BrokerChannel>> broker_channels_;

				BrokerChannel* GetChannel(const std::string& topic);
				void ProxyThread();
			};
		}
	}
}

