#pragma once
#include <cstdint>

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace repositories
			{
#pragma pack(push, 1)
				struct RepositoryHeader
				{
					char topic[100]; //null terminated string
					uint64_t last_write_pos;
					uint64_t last_read_pos;

					RepositoryHeader() :
						topic{ 0x00 },
						last_write_pos(0),
						last_read_pos(0)
					{

					}
				};
#pragma pack(pop)
			}
		}
	}
}
