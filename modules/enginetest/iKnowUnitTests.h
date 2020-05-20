#pragma once
namespace testing {
	class iKnowUnitTests
	{
	public:
		static void runUnitTests(void);

	private:
		void test1(const char *); // Japanese text should produce entity vectors
	};
}