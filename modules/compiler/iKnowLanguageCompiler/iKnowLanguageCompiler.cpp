// iKnowLanguageCompiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CSV_DataGenerator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>

using namespace std;
using namespace iknow::csvdata;

#ifdef WIN32 // current working directory on Windows
#include <Windows.h>
void workingdir(string& work_dir) {
	char buffer[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, buffer);
	work_dir = std::string(buffer) + '\\';
}
#else // on Linux
#include <unistd.h>
#include <stdio.h>
#include <limits.h>

void workingdir(string& work_dir) {
   char cwd[PATH_MAX];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
	work_dir = string(cwd);
   } else {
       work_dir = "";
   }
}
#endif

set<string> SetOfLanguages = { "en", "de", "ru", "es", "fr", "ja", "nl", "pt", "sv", "uk", "cs" };

int main(int argc, char* argv[])
{
	string exe_path(argv[0]);
	cout << "exe_path: " << exe_path << endl;
	
#ifdef WIN32
	string repo_root("C:/Users/jdenys/source/repos/iknow/");
	workingdir(exe_path); 
	size_t kit_pos = exe_path.find("\\kit\\");
#else
	string repo_root("/home/jdenys/iknow/");
	workingdir(exe_path);
	size_t kit_pos = exe_path.find("/kit/");
#endif
	if (kit_pos != string::npos) {
		repo_root = string(exe_path.begin(), exe_path.begin() + kit_pos + 1);
	}
	else {
		std::cerr << "Run the Language Compiler from the /kit/ directory" << endl;
		exit(-1);
	}
	std::string csv_path = repo_root + "language_models/";
	std::string aho_path = repo_root + "modules/aho/";
	std::string ldata_path = repo_root + "modules/engine/language_data";

	string language_to_build;

	if (argc == 2) {
		language_to_build = argv[1];
		if (!SetOfLanguages.count(language_to_build)) { // unsupported language
			cerr << "***error*** language \"" << language_to_build << "\" not supported !" << endl;
			cerr << "Supported languages are : " << endl;
			for_each(SetOfLanguages.begin(), SetOfLanguages.end(), [](const string& language) { cout << "\"" << language << "\"" << char(9); });
			return -1;
		}
	}
	try {
		std::cout << "iKnowLanguage compiler" << std::endl << std::endl;
		std::ofstream os("compiler_report.log"); // logging file for indexing results
		os << "\xEF\xBB\xBF"; // Force utf8 header, maybe utf8 is not the system codepage, for std::cout, use "chcp 65001" to switch

		if (language_to_build.empty()) { // build them all
			for (auto it = SetOfLanguages.begin(); it != SetOfLanguages.end(); ++it) {
				CSV_DataGenerator my_csv_generator(csv_path, aho_path, ldata_path);
				my_csv_generator.loadCSVdata(*it, false);
				my_csv_generator.generateRAW();
				my_csv_generator.generateAHO();
			}
		}
		else { // build one
			CSV_DataGenerator my_csv_generator(csv_path, aho_path, ldata_path);
			my_csv_generator.loadCSVdata(language_to_build, false);
			my_csv_generator.generateRAW();
			my_csv_generator.generateAHO();
		}
		os.close();
	}
	catch (std::exception& e)
	{
		cerr << e.what() << endl;
	}
	catch (...) {
		cerr << "Language Compiler failed..." << std::endl;
	}
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
