#pragma once

#include "OutputFunction.h"

namespace iknow {
	namespace AHO {

		class LexrepOutputFunction :
			public OutputFunction
		{
		public:
			LexrepOutputFunction();
			~LexrepOutputFunction();

			void ToC(std::string dir = "");
			std::string IndexListToOutputList(labels_Type indexList);

		};

	}
}
