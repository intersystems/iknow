#pragma once
namespace testing {
	class iKnowUnitTests
	{
	public:
		static void runUnitTests(void);

	private:
		void test1(const char *); // Japanese text should produce entity vectors
		void test2(const char *); // Measurement attributes in English test. 
		void test3(const char *); // Attribute markers must be lowercase.
		void test4(const char *); // Test SBegin/SEnd labels.
		void test5(const char *); // User DCT test
		void test6(const char *); // Text Normalizer test
		void test7(const char *); // Issue#31 : https://github.com/intersystems/iknow/issues/31
		void test8(const char *); // Issue#37 : https://github.com/intersystems/iknow/issues/37 
		void Issue41(const char *); // Issue#41 : https://github.com/intersystems/iknow/issues/41
		void Issue39(const char*); // Issue#41 : https://github.com/intersystems/iknow/issues/39
		void Issue42(const char*); // Issue#41 : https://github.com/intersystems/iknow/issues/42

	};
}