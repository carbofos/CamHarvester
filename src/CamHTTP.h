#pragma once
#include <vector>
#include <string>
#include <map>
#include <string_view>
#include <utility>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>


#include "CamFunctions.h"
#include "CamCallable.h"
#include "CamHTTPConstants.h"

class CamHTTP
{
#define STATES_LENGTH 11
public:
	enum states {New, Start, CollectingHeaders, ProcessHeaders, CollectingMultipartHeaders, ProcessMultipartHeaders, DataByLength, DataByBoundary, DataByLengthFinish, DataByBoundaryFinish, Closing};
	//   CamHTTP() noexcept;
	//   ~CamHTTP();

protected:
	std::string incomlete_line;
	std::string current_line;
	std::string boundarystring;
	std::string data;
	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> headers_multipart;
	std::pair<std::string, std::string> parseheader(const std::string_view & source_header_string);
	states state_current = states::New;
	states state_next = states::Closing;
	CamCallable *callback = nullptr;

public:
 
	static states (*stateprocessors[STATES_LENGTH])(const std::string_view, CamHTTP&);
	const static bool chopping[STATES_LENGTH];

	static states stateprocessor_new(const std::string_view, CamHTTP&);
	static states stateprocessor_collectingheaders(const std::string_view, CamHTTP&);
	static states stateprocessor_collectingmultipartheaders(const std::string_view, CamHTTP&);
	static states stateprocessor_processheaders(const std::string_view, CamHTTP&);
	static states stateprocessor_processmultipartheaders(const std::string_view, CamHTTP&);
	static states stateprocessor_databyboundary(const std::string_view, CamHTTP&);
	static states stateprocessor_databylength(const std::string_view, CamHTTP&);
	static states stateprocessor_closing(const std::string_view, CamHTTP&);

	void push_newline(const std::string_view &newline);
	void add_header(const std::string_view rawheader);
	void add_multipartheader(const std::string_view rawheader);
	static std::string get_header(const std::string_view name, const std::map<std::string, std::string> &headers_map);
	static size_t get_content_length(const std::map<std::string, std::string> &headers_map);
	static int http_code(const std::string_view &line);
	static bool isnewline(const std::string_view &l);
	static std::string getBoundaryString(const std::string& mime_type_part);
	states get_state();
	static std::string_view chopped(const std::string_view line);
	void set_on_datareceived(CamCallable * callable);
};

