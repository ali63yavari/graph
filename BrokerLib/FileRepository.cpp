#include "stdafx.h"
#include "FileRepository.h"
#include "BrokerMessageBase.h"
#include "RepositoryBody.h"
#include <iostream>

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace repositories
			{
				FileRepository::FileRepository(const std::string& repo_path, const std::string& file_name)
				{
					assert((!repo_path.empty()));
					assert((!file_name.empty()));

					const auto len = std::min(sizeof(repo_header_.topic), file_name.length());
					memcpy(repo_header_.topic, file_name.c_str(), len);

					fs::path path(repo_path);
					const auto is_exist = fs::exists(path);
					assert(is_exist);

					auto repo_full_path = path.append(file_name);
					repo_full_path_ = repo_full_path.string();

					if (fs::exists(repo_full_path))
					{
						repo_handle_ = std::make_unique<std::fstream>(repo_full_path, std::ios_base::out | std::ios_base::in | std::ios_base::binary);
						assert(!repo_handle_->bad());
						repo_handle_->read(reinterpret_cast<char*>(&repo_header_), sizeof(repo_header_));
					}
					else
					{
						auto ofs = std::make_unique<std::ofstream>(repo_full_path, std::ios_base::out | std::ios_base::binary);
						ofs->flush();
						ofs->close();

						repo_header_.last_write_pos = sizeof(repo_header_);
						repo_header_.last_read_pos = sizeof(repo_header_);

						repo_handle_ = std::make_unique<std::fstream>(repo_full_path, std::ios_base::out | std::ios_base::in | std::ios_base::binary);
						repo_handle_->write(reinterpret_cast<char*>(&repo_header_), sizeof(repo_header_));
						repo_handle_->flush();					
					}
				}			

				bool FileRepository::Enqueue(const char* raw_data, int len)
				{
					RepositoryBody body(raw_data, len);

					std::vector<char> pbody(body.GetSize());
					body.GetByte((char*)(&pbody.data()[0]));

					std::unique_lock<std::shared_mutex> lock(mutex_);			

					repo_handle_->seekg(repo_header_.last_write_pos, repo_handle_->beg);
					repo_handle_->write((char*)(&pbody.data()[0]), pbody.size());
					repo_header_.last_write_pos = repo_handle_->tellg();

					UpdateRepositoryHeader();

					return true;
				}

				uint32_t FileRepository::Dequeue(std::vector<char>& raw_data)
				{					
					if (Empty())
					{
						return 0;
					}

					std::unique_lock<std::shared_mutex> lock(mutex_);

					uint32_t body_len = 0;

					repo_handle_->seekg(repo_header_.last_read_pos, repo_handle_->beg);
					repo_handle_->read(reinterpret_cast<char*>(&(body_len)), sizeof(body_len));
					raw_data.resize(body_len);
					repo_handle_->read(reinterpret_cast<char*>(&(raw_data.data()[0])), body_len);
					repo_header_.last_read_pos = repo_handle_->tellg();

					UpdateRepositoryHeader();

					return body_len;
				}

				bool FileRepository::Empty() const
				{
					return repo_header_.last_read_pos == repo_header_.last_write_pos;
				}

				void FileRepository::UpdateRepositoryHeader()
				{				
					repo_handle_->seekg(0, repo_handle_->beg);
					repo_handle_->write(reinterpret_cast<char*>(&repo_header_), sizeof(repo_header_));
					repo_handle_->flush();
				}
			}
		}
	}
}
