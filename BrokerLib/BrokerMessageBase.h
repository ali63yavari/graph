#pragma once

namespace graph
{
	namespace qc
	{
		namespace broker
		{
			namespace models
			{
				class BrokerMessageBase
				{
				public:

					BrokerMessageBase()= default;
					virtual ~BrokerMessageBase() = default;

					virtual int GetSize() = 0;
					virtual void SetBytes(const char* raw_data, int& start_pos) = 0;
					virtual int GetBytes(char* raw_data) = 0;				
				};
			}
		}
	}
}
