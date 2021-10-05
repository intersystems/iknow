// iKnowExplicitTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <cstring>

//
// C function declaration
//
typedef int (*p_iknow_json)(const char* request, const char** response);

using namespace std;

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
    }
    else {
        work_dir = "";
    }
}
#endif

#ifdef WIN32
p_iknow_json get_iknow_json()
{
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    GetSystemInfo(&si);
    // cout << si.wProcessorArchitecture << endl;

    HMODULE hMod = LoadLibraryA("iKnowEngine.dll");
    if (NULL == hMod) {
        printf("LoadLibrary \"iKnowEngine.dll\" failed\n");
        exit(1);
    }
    cout << "LoadLibrary \"iKnowEngine.dll\" succeeded" << std::endl;

    p_iknow_json iknow_json = (p_iknow_json) GetProcAddress(hMod, "iknow_json");
    if (NULL == iknow_json) {
        cout << "iknow_json function retrieval failed" << std::endl;
        exit(1);
    }
    cout << "iknow_json function retrieval succeeded" << std::endl;
    return iknow_json;
}
#else
#include <dlfcn.h>
#define P2(x) #x
void* libptr = NULL;
p_iknow_json get_iknow_json()
{
    char	libarray[128];
#ifdef MACOSX
    const char* libnam = "libiknowengine.dylib";
#else
    const char* libnam = "libiknowengine.so";
#endif

    strcpy(libarray, libnam);
    char* error;
    if (!(libptr = dlopen(libarray, RTLD_LAZY))) {
        fprintf(stderr, "dlopen library \"libiknowengine\" failed: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    dlerror();    /* Clear any existing error */
    const char* symbnam = P2(iknow_json);
    p_iknow_json iknow_json = (p_iknow_json) dlsym(libptr, symbnam);
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    cout << "iknow_json function retrieval succeeded" << std::endl;
    return iknow_json;
}
#endif

int main(int argc, char* argv[])
{
    std::cout << "iKnowExplicitTest program\n";

    p_iknow_json iknow_json = get_iknow_json();

    const char* j_response;
    {
        string j_request(R"({"method" : "GetLanguagesSet"})");
        cout << endl << "request:" << j_request << endl;
        iknow_json(j_request.c_str(), &j_response);
        cout << "response:" << j_response << std::endl;
    }
    {
        string j_request(R"({"method" : "NormalizeText", "language":"fr", "text_source" : "Risque d'exploitation"})");
        cout << endl << "request:" << j_request << endl;
        iknow_json(j_request.c_str(), &j_response);
        cout << "response:" << j_response << std::endl;
    }
    {
        const char text_source[] = u8"Микротерминатор может развивать скорость до 30 сантиметров за секунду, пишут калининградские СМИ.";
        string j_request = R"({"method" : "IdentifyLanguage", "text_source" : ")" + string(text_source) + "\"}";
        cout << endl << "request:" << j_request << endl;
        iknow_json(j_request.c_str(), &j_response);
        cout << "response:" << j_response << std::endl;
    }
    {
        string j_request(R"({"method" : "index", "language" : "en", "text_source" : "This is a test of the Python interface to the iKnow engine. Be the change you want to see in life. Now, I have been on many walking holidays, but never on one where I have my bags ferried\nfrom hotel to hotel while I simply get on with the job of walkingand enjoying myself.", "b_trace" : true})");
        cout << endl << "request:" << j_request << endl;
        iknow_json(j_request.c_str(), &j_response);
        cout << "response:" << j_response << std::endl;
    }

#ifndef WIN32
    if (libptr != NULL)
        dlclose(libptr);
#endif
}