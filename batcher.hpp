/*! \author		Haohan Liu
	\name		batcher.hpp
	\date		2019-09-22
	*/
#pragma once

#define UNICODE

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <istream>
#include <fstream>
#include <iterator>
#include <thread>
#include <string>
#include <vector>
#include <map>
#include <process.h>
#include <algorithm>

using namespace std;

class batcher {
private:
	std::map<string, vector<string>> launchTimeInfo;
	std::map<string, vector<string>> errDetail;

	string remove_char_from_string(string ostr, char c, char s = '0');
	wstring remove_char_from_wstring(wstring ostr, char c, char s = '0');
	void display_vector(const vector<string>& v);
	string retrieve_params(const vector<string> params, string str);
	void launch_using_create_process(std::string groupNum, std::string infoStr);
public:
	void run(char** argv);
};