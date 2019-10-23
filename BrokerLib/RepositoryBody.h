#pragma once
#include <cstdint>
#include <cassert>
#include <cstring>
#include <vector>

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace repositories
			{
#pragma pack(push, 1)
				struct RepositoryBody
				{
					uint32_t body_len{ 0 };
					std::vector<char> body_data;

					RepositoryBody() = default;

					RepositoryBody(const char* data, uint32_t len)
					{
						assert(len > 0);
						assert(data != nullptr);

						body_data.resize(len);
						memcpy(&(body_data.data()[0]), data, len);
						body_len = len;
					}

					uint32_t GetSize() const
					{
						return sizeof(body_len) + body_len;
					}

					uint32_t GetByte(char* raw_data) const
					{
						int pos = 0;
						memcpy(raw_data + pos, (char*)(&body_len), sizeof(body_len));
						pos += sizeof(body_len);
						memcpy(raw_data + pos, &(body_data.data()[0]), body_len);
						pos += sizeof(body_len);

						return pos;
					}					
				};
#pragma pack(pop)
			}
		}
	}
}
