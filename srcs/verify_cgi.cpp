#include <iostream>
#include <string>
#include <sys/stat.h>

bool checkCGIExists(const std::string & cgiPath) {
    struct stat fileInfo;

    // Use stat to get the file attributes
    if (stat(cgiPath.c_str(), &fileInfo) != 0) {
        std::cerr << "Error: CGI script '" << cgiPath << "' does not exist." << std::endl;
        return false;
    }

    // Check if the file is executable
    if (!(fileInfo.st_mode & S_IXUSR)) {
        std::cerr << "Error: CGI script '" << cgiPath << "' is not executable." << std::endl;
        return false;
    }

    return true;
}

// int main() {
//     std::string cgiPath = "../cgi-bin/process_data.cgi";

//     if (!checkCGIExists(cgiPath)) {
//         return -1;  // Handle the error as appropriate for your application
//     }

//     std::cout << "CGI script is valid and executable." << std::endl;

//     // Further processing can go here...
//     return 0;
// }