#pragma once
#include <cstdint>
#include <chrono>
#include <string>

#include "BrokerMessageBase.h"

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace models
			{
				class BrokerMessage : public BrokerMessageBase
				{
				public:
					BrokerMessage() = default;
					~BrokerMessage()
					{
						//try
						{
							delete[] data;
						}
						//catch (...)
						{
						}
					}
					

					void UpdateId()
					{
						std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(
							std::chrono::system_clock::now().time_since_epoch());
						msg_id = us.count();
					}

					void IncreaseCounter()
					{
						msg_id++;
					}

					int GetSize() override
					{
						return sizeof(long long) + sizeof(uint32_t) + topic.length() + sizeof(uint32_t) + data_len;
					}
					void SetBytes(const char* raw_data, int& start_pos ) override
					{
						memcpy(reinterpret_cast<char*>(&msg_id), raw_data + start_pos, sizeof(long long));
						start_pos += sizeof(long long);

						memcpy(reinterpret_cast<char*>(&data_len), raw_data + start_pos, sizeof(uint32_t));
						start_pos += sizeof(uint32_t);

						uint32_t topic_len = 0;
						memcpy(reinterpret_cast<char*>(&topic_len), raw_data + start_pos, sizeof(uint32_t));
						start_pos += sizeof(uint32_t);

						topic = "";

						if (topic_len > 0)
						{
							char* ptopic = new char[topic_len];
							memcpy(ptopic, raw_data + start_pos, topic_len);
							start_pos += topic_len;

							topic = std::string(ptopic, topic_len);
						}										

						data = nullptr;

						if (data_len > 0)
						{
							data = new char[data_len];
							memcpy(data, raw_data + start_pos, data_len);
							start_pos += data_len;
						}
					}

					int GetBytes(char* pdata) override
					{
						const auto len = GetSize();

						if (len <=0)
						{
							pdata = nullptr;
							return 0;
						}
						
						int pos = 0;

						memcpy(pdata + pos, reinterpret_cast<char*>(&msg_id), sizeof(long long));
						pos += sizeof(long long);

						memcpy(pdata + pos, reinterpret_cast<char*>(&data_len), sizeof(uint32_t));
						pos += sizeof(uint32_t);

						int topic_len = topic.length() >= 0 ? topic.length() : 0;
						memcpy(pdata + pos, reinterpret_cast<char*>(&topic_len), sizeof(uint32_t));
						pos += sizeof(uint32_t);

						if (topic_len>0)
						{
							memcpy(pdata + pos, topic.data(), topic_len);
							pos += topic_len;
						}									

						if (data_len > 0)
						{
							memcpy(pdata + pos, data, data_len);
							pos += data_len;
						}

						return pos;
					}

					long long msg_id{0};
					uint32_t data_len{0};
					std::string topic{};
					char* data{};					
				};
			}
		}
	}
}
