#pragma once
#include <string>
#include <sstream>

namespace graph
{
	namespace qc
	{
		namespace extensions
		{
			struct StringBuilder
			{
				std::stringstream ss;
				template<typename T>
				StringBuilder& operator << (const T &data)
				{
					ss << data;
					return *this;
				}

				operator std::string()
				{
					return ss.str();
				}
			};
		}
	}
}
