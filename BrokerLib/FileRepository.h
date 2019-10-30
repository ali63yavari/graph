#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <shared_mutex>
#include "RepositoryHeader.h"

namespace fs = std::experimental::filesystem;

#define MAX_FILE_CHUNK (100*1024*1024) //500MBytes

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace repositories
			{				
				class FileRepository
				{
				public:
					FileRepository(const std::string& repo_path, const std::string& file_name);
					~FileRepository() = default;
					bool Enqueue(const char* raw_data, int len);
					uint32_t Dequeue(std::vector<char>& raw_data);
					bool Empty() const;

				private:
					std::unique_ptr<std::fstream> repo_handle_;
					RepositoryHeader repo_header_;
					mutable std::shared_mutex mutex_;
					std::string repo_full_path_{};

					void UpdateRepositoryHeader();
				};
			}
		}
	}
}




