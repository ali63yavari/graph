#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <shared_mutex>
#include "RepositoryHeader.h"

namespace fs = std::experimental::filesystem;

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
					bool Enqueue(const std::vector<char>& raw_data);
					uint32_t Dequeue(std::vector<char>& raw_data);
					bool Empty() const;

				private:
					std::unique_ptr<std::fstream> repo_handle_;
					RepositoryHeader repo_header_;
					mutable std::shared_mutex mutex_;

					void UpdateRepositoryHeader();
				};
			}
		}
	}
}




