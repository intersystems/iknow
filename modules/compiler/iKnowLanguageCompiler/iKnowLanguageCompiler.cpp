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

void build_one_language(string& csv, string& aho, string& ldata, string& langdev, string lang)
{
	CSV_DataGenerator my_csv_generator(csv, aho, ldata);
	std::ofstream os(langdev + lang + "_compiler_report.log"); // logging file for indexing results
	if (os.is_open()) {
		os << "\xEF\xBB\xBF"; // Force utf8 header, maybe utf8 is not the system codepage, for std::cout, use "chcp 65001" to switch
		my_csv_generator.loadCSVdata(lang, false, os);
		os.close();
	} else
		my_csv_generator.loadCSVdata(lang, false);

	my_csv_generator.generateRAW();
	my_csv_generator.generateAHO();
}

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
	string csv_path = repo_root + "language_models/";
	string aho_path = repo_root + "modules/aho/";
	string ldata_path = repo_root + "modules/engine/language_data";
	string langdev_path = repo_root + "language_development/";

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
		
		if (language_to_build.empty()) { // build them all
			for (auto it = SetOfLanguages.begin(); it != SetOfLanguages.end(); ++it) {
				build_one_language(csv_path, aho_path, ldata_path, langdev_path, *it);
			}
		}
		else { // build one
			build_one_language(csv_path, aho_path, ldata_path, langdev_path, language_to_build);
		}
	}
	catch (std::exception& e)
	{
		cerr << e.what() << endl;
		return -1; // pass error codes to stop batch processing.
	}
	catch (...) {
		cerr << "Language Compiler failed..." << std::endl;
		return -2; // return error code
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
