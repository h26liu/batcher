/*! \author		Haohan Liu
	\name		batcher.cpp
	\date		2019-09-22
	*/
#include "batcher.hpp"

string batcher::remove_char_from_string(string ostr, char c, char s)
{
	std::string nstr(ostr);
	nstr.erase(std::remove(nstr.begin(), nstr.end(), c), nstr.end());

	// remove whitespaces (optional)
	if (s != '0') {
		nstr.erase(std::remove(nstr.begin(), nstr.end(), ' '), nstr.end());
	}

	return nstr;
}

wstring batcher::remove_char_from_wstring(wstring ostr, char c, char s)
{
	std::wstring nstr(ostr);
	nstr.erase(std::remove(nstr.begin(), nstr.end(), c), nstr.end());

	// remove whitespaces (optional)
	if (s != '0') {
		nstr.erase(std::remove(nstr.begin(), nstr.end(), ' '), nstr.end());
	}

	return nstr;
}

void batcher::display_vector(const vector<string>& v)
{
	std::copy(v.begin(), v.end(),
		std::ostream_iterator<string>(std::cout, " "));
}

string batcher::retrieve_params(const vector<string> params, string str)
{
	for (auto p : params) {
		str += p;
		str += " ";
	}

	return str;
}

void batcher::launch_using_create_process(std::string groupNum, std::string infoStr)
{
	std::size_t firstChar = infoStr.find_first_not_of(" "); // first char in the str (not whitespace) position
	std::size_t firstComma = infoStr.find_first_of(","); // first comma postion
	std::size_t secondComma = infoStr.find(",", firstComma + 1); // second comma position

	// <LAUNCH GROUP NUMBER>
	std::string launchGroup = groupNum;
	// <PROGRAM NAME>
	std::string programName("");
	for (size_t i = firstComma; i < secondComma; )
	{
		programName += infoStr[i];
		i++;
		if (infoStr[i] == ',') {
			programName = remove_char_from_string(programName, ',');
			break;
		}
	}
	programName = programName.substr(programName.find_first_not_of(" ")); // remove extra whitespaces
	// <COMMAND LINE PARAMS>
	std::string cmdParams = remove_char_from_string(infoStr.substr(secondComma), ',');
	// separate each param into a string vector
	std::stringstream ps(cmdParams);
	std::istream_iterator<std::string> p_begin(ps);
	std::istream_iterator<std::string> p_end;
	std::vector<string> vparams(p_begin, p_end); // it has to be string vector, cause some params may be string

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Create Process
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	std::wstring w_programName(programName.begin(), programName.end());
	std::wstring w_params(cmdParams.begin(), cmdParams.end());
	std::wstring command = w_programName + L" " + w_params;

	STARTUPINFO sinfo = { 0 };
	sinfo.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi = { 0 };

	unsigned long const CP_MAX_COMMANDLINE = 32768;

	try {
		wchar_t* commandline = new wchar_t[CP_MAX_COMMANDLINE];

		// wcsncpy_s(pointer to the wide char array is copying to, the size of the destination buffer, pointer to the wide string copying from, max number of wide chars to copy)
		wcsncpy_s(commandline, CP_MAX_COMMANDLINE, command.c_str(), command.size() + 1);
		auto res = CreateProcess(
			NULL, // application name is null since provided in commandline (doesn't use PATH)
			commandline, // contains application name + params
			NULL,		// same security as parent
			NULL,
			false,
			CREATE_NEW_CONSOLE, // launch the process in a new console window
			NULL,
			NULL, // same current directory as parent
			&sinfo,		// startup options
			&pi		// process information
		);

		if (res == 0) {
			cerr << "Error: " << GetLastError() << endl;
			//return;
		}

		delete[] commandline;
	}
	catch (std::bad_alloc&) {
		wcerr << L"Insufficient memory to launch application" << endl;
		//return;
	}

	DWORD exit_code;

	if (pi.hProcess != NULL && WAIT_FAILED == WaitForSingleObject(pi.hProcess, INFINITE))
		cerr << "Failure waiting for process to terminate" << endl;

	if (pi.hProcess != NULL) 
		GetExitCodeProcess(pi.hProcess, &exit_code);

	// Times
	FILETIME creationTime, exitTime, kernelTime, userTime;

	SYSTEMTIME kTime;
	SYSTEMTIME uTime;

	if (pi.hProcess != NULL) {
		GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
	
		//SYSTEMTIME kTime;
		::FileTimeToSystemTime(&kernelTime, &kTime);

		//SYSTEMTIME uTime;
		::FileTimeToSystemTime(&userTime, &uTime);
	}

	if (pi.hProcess != NULL && pi.hThread != NULL) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


	// Kernel Time
	std::string str_kTime =
		"K:" + std::string(2 - std::to_string(kTime.wHour).length(), '0') + std::to_string(kTime.wHour) +
		":" + std::string(2 - std::to_string(kTime.wMinute).length(), '0') + std::to_string(kTime.wMinute) +
		":" + std::string(2 - std::to_string(kTime.wSecond).length(), '0') + std::to_string(kTime.wSecond) +
		"." + std::string(3 - std::to_string(kTime.wMilliseconds).length(), '0') + std::to_string(kTime.wMilliseconds);

	// User Time
	std::string str_uTime =
		"U:" + std::string(2 - std::to_string(uTime.wHour).length(), '0') + std::to_string(uTime.wHour) +
		":" + std::string(2 - std::to_string(uTime.wMinute).length(), '0') + std::to_string(uTime.wMinute) +
		":" + std::string(2 - std::to_string(uTime.wSecond).length(), '0') + std::to_string(uTime.wSecond) +
		"." + std::string(3 - std::to_string(uTime.wMilliseconds).length(), '0') + std::to_string(uTime.wMilliseconds);

	// Exit Code
	std::string str_eCode = "E:" + std::to_string(exit_code);

	// Launch Group
	std::string str_lGroup = "G:" + launchGroup;

	// launch time info string
	launchTimeInfo[launchGroup]
		.push_back(retrieve_params
		(vparams, (str_kTime + " " + str_uTime + " " + str_eCode + " " + str_lGroup + " " + programName + " ")));

	// err detail string
	if (exit_code != 0) {
		std::string errStr = retrieve_params(vparams, ("G#: " + launchGroup + " Command: " + programName + " Params: "));
		errStr += "\n ---> Error = ";
		errStr += std::to_string(exit_code);
		errDetail[launchGroup]
			.push_back(errStr);
	}
}

void batcher::run(char** argv)
{
	// App Description
	cout << "batcher, " << "(c)" << " 2019 All Rights Reserved, Haohan Liu\n" << endl;

	// Read command line args
	std::ifstream file(argv[1]);

	// create a map container
	// process each str into the map
	// <GROUP#, str>
	std::string str;
	std::map<string, vector<string>> lgi; // launch group information

	while (std::getline(file, str)) {
		if (str.find_first_not_of(' ') != std::string::npos) {

			std::size_t firstChar = str.find_first_not_of(" "); // first char in the str (not whitespace) position
			std::size_t firstComma = str.find_first_of(","); // first comma postion

			// <LAUNCH GROUP NUMBER>
			std::string launchGroup = remove_char_from_string(str.substr(firstChar, firstComma), ',', ' ');

			lgi[launchGroup].push_back(str);
		}
	}

	// processing here $-$
	std::map<string, vector<string>>::const_iterator mit = lgi.begin();
	for (; mit != lgi.end(); mit++)
	{
		std::vector<std::thread> threads;

		for (size_t i = 0; i < mit->second.size(); i++)
		{
			threads.push_back(std::thread(&batcher::launch_using_create_process, this, mit->first, mit->second[i]));
		}

		for (size_t i = 0; i < threads.size(); i++)
		{
			threads[i].join();
		}
	}

	// PRINT REPORT
	cout << "Launch Times" << endl;

	// @launch time
	mit = launchTimeInfo.begin();
	for (; mit != launchTimeInfo.end(); ++mit) {
		cout << "\nGroup:" << mit->first << endl;

		for (size_t i = 0; i < mit->second.size(); i++) {
			cout << mit->second[i] << endl;
		}
	}
	// @err detail
	mit = errDetail.begin();
	cout << "\n\nErrors" << endl;
	for (; mit != errDetail.end(); ++mit) {

		for (size_t i = 0; i < mit->second.size(); i++) {
			cout << mit->second[i] << endl;
		}
	}

	cout << endl;
}
