/*
* ISC_IRIS_callout.cpp
* Boilerplate code to convert the iKnowEngine into a loadable IRIS module, and communicate using JSON
* For more information on callout modules, see :
* https://docs.intersystems.com/iris20212/csp/docbook/DocBook.UI.Page.cls?KEY=BGCL
* 
* Add ISC_IRIS_callout.cpp to the project, 
* add the path to your IRIS installation to the C++ preprocessor include directory list (eg: "C:\InterSystems\IRIS\dev\iris-callin\include"),
* and build the module.
* 
* See the "TestiKnowJSON" outcommented section,
* this is COS code, equivalent to the same C++ functionality you find in iKnowExplicitTest.cpp,
* use it as the starting point of working with the standalone iKnow engine directly from COS.
*/

#include <cstring>
#include <iostream>

#ifdef WIN32
#define IKNOW_API __declspec(dllexport)
#else
#define IKNOW_API
#endif

extern int iknow_json_(const char* request, const char** response);

/*
** before loading iKnowEngine
*/
extern "C" IKNOW_API void ZFInit(void) 
{
}

/*
** before unloading iKnowEngine
*/ 
extern "C" IKNOW_API void ZFUnload() 
{
}

#define ZF_DLL  /* Required for all Callout code. */
#include <iris-cdzf.h>  /* Required for all Callout code. */
#include <cstdio>
#include <stdio.h>
#include <wchar.h>   // Required for 16-bit and 32-bit strings
#include <iris-callin.h>

int iknow_json_callout(const char* request, IRIS_EXSTRP retval) {  // 8-bit characters
	std::cout << "request:" << request << std::endl << std::endl;
	const char* j_response = R"({ "output" : "NULL" })";
	iknow_json_(request, &j_response);
	std::cout << "response:" << j_response << std::endl << std::endl;

	size_t len = std::strlen(j_response);
	IRISEXSTRKILL(retval);
	if (!IRISEXSTRNEW(retval, (int) len)) { return ZF_FAILURE; }
	memcpy(retval->str.ch, j_response, len);   // copy to retval->str.ch
	return ZF_SUCCESS;
}

ZFBEGIN
ZFENTRY("iKnowJSON", "1c1J", iknow_json_callout)
ZFEND

/*
TestiKnowJSON
	Set dllId = $ZF(-4,1,"C:\Users\jdenys\source\repos\iknow\kit\x64\Release\bin\iKnowEngine.dll")
	Set funcId = $ZF(-4,3,dllId,"iKnowJSON")

	Set request = ##class(%DynamicObject).%New()
	Set request.method = "GetLanguagesSet"
	Set responseText = $ZF(-5,dllId,funcId,request.%ToJSON())
	Set response = ##class(%DynamicAbstractObject).%FromJSON(responseText)
	Zwrite response Kill request

	Set request = ##class(%DynamicObject).%New()
	Set request.method = "NormalizeText"
	Set request.language = "fr"
	Set request."text_source" = "Risque d'exploitation"
	Set responseText = $ZF(-5,dllId,funcId,request.%ToJSON())
	Set response = ##class(%DynamicAbstractObject).%FromJSON(responseText)
	Zwrite response Kill request

	Set request = ##class(%DynamicObject).%New()
	Set request.method = "IdentifyLanguage"
	Set textUtf8 = $ZCONVERT("Микротерминатор может развивать скорость до 30 сантиметров за секунду, пишут калининградские СМИ.", "O", "UTF8")
	Set request."text_source" = textUtf8
	Set responseText = $ZF(-5,dllId,funcId,request.%ToJSON())
	Set response = ##class(%DynamicAbstractObject).%FromJSON(responseText)
	Zwrite response Kill request

	Set request = ##class(%DynamicObject).%New()
	Set request.method = "index"
	Set request.language = "en"
	Set request."text_source" = "This is a test of the Python interface to the iKnow engine. Be the change you want to see in life. Now, I have been on many walking holidays, but never on one where I have my bags ferried\nfrom hotel to hotel while I simply get on with the job of walkingand enjoying myself."
	Do request.%Set("b_trace",1,"boolean")
	Set responseText = $ZF(-5,dllId,funcId,request.%ToJSON())
	Set response = ##class(%DynamicAbstractObject).%FromJSON(responseText)
	Zwrite response Kill request

	Quit
*/