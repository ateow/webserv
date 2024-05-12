#include "../includes/load_config.hpp"

void trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not1(std::ptr_fun<int, int>(isspace))));
    s.erase(std::find_if(s.rbegin(), s.rend(), not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
}

void WebServerConfig::parseConfig(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::string section;
    std::map<std::string, size_t> serverMap;
    std::map<std::string, size_t> routeMap;

    while (getline(file, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue; // Skip comments and empty lines

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            // New section
            section = line.substr(1, line.size() - 2);
            if (section.find("server") != std::string::npos) {
                servers.push_back(ServerConfig());
                serverMap[section] = servers.size() - 1;
            } else if (section.find("route") != std::string::npos) {
                routeMap[section] = atoi(section.substr(5).c_str()) - 1; // Assuming "route1" maps to 0th index
            }
            continue;
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            trim(key);
            trim(value);

            if (section.find("server") != std::string::npos && serverMap.count(section)) {
                ServerConfig& server = servers[serverMap[section]];
                if (key == "host") server.host = value;
                else if (key == "port") server.port = atoi(value.c_str());
                else if (key == "server_names") {
                    server.s_name = value;
                    std::istringstream iss(value);
                    std::string token;
                    while (getline(iss, token, ' ')) {
                        trim(token);
                        server.server_names.push_back(token);
                    }
                }
                else if (key == "default_error_pages") {
                    std::istringstream errors(value);
                    std::string error;
                    while (std::getline(errors, error, ' ')) {  // Assume spaces between entries
                        trim(error);
                        size_t colon_pos = error.find(':');
                        if (colon_pos != std::string::npos) {
                            int code = atoi(error.substr(0, colon_pos).c_str());
                            std::string path = error.substr(colon_pos + 1);
                            server.default_error_pages[code] = path;
                        }
                    }
                }
                else if (key == "limit_client_body_size") server.limit_client_body_size = value;
            } else if (section.find("route") != std::string::npos && routeMap.count(section) && routeMap[section] < servers.size()) {
                RouteConfig& route = servers[routeMap[section]].route;
                if (key == "root_directory") route.root_directory = value;
                else if (key == "default_file") route.default_file = value;
                else if (key == "list_directory") route.list_directory = value;
                else if (key == "accepted_methods") {
                    std::istringstream iss(value);
                    std::string method;
                    while (getline(iss, method, ' ')) {
                        trim(method);
                        route.accepted_methods.push_back(method);
                    }
                }
                else if (key == "old_path") {
                    std::istringstream iss(value);
                    std::string old_path;
                    while (getline(iss, old_path, ' ')) {
                        trim(old_path);
                        route.old_paths.push_back(old_path);
                    }
                }
                else if (key == "redirect") route.redirect = value;
                else if (key == "cgi_enable") route.cgi_enable = value;
                else if (key == "cgi_path") route.cgi_path = value;
                else if (key == "upload_enable") route.upload_enable = value;
                else if (key == "upload_path") route.upload_path = value;
            }
            //  else if (section == "chunk_handling") {
            //     chunk_handling = value;
            // }
        }
    }
    file.close();
}


// Function to check if a string is empty or consists only of whitespace
bool is_empty_or_whitespace(const std::string& s) {
    return s.find_first_not_of(" \t\n\r\f\v") == std::string::npos;
}

int checkConfig(WebServerConfig& config) {

    // Check WebServerConfig
    // if (is_empty_or_whitespace(config.chunk_handling)) {
    //     return 0;
    // }

    for (std::vector<ServerConfig>::iterator server = config.servers.begin(); server != config.servers.end(); ++server) {
        // Check ServerConfig
        if (is_empty_or_whitespace(server->host) ||
            is_empty_or_whitespace(server->s_name) ||
            is_empty_or_whitespace(server->limit_client_body_size)) {
            return 0;
        }
        if (server->route.accepted_methods.empty()) {
            return 0;
        }

        if (server->port <= 0 || server->port > 65535){ // Port should be less than 65536)
            std::cerr << "Error: Invalid port number for server " << server->host << ":" << server->port << std::endl;
            return 0;
        }
        std::string limit = server->limit_client_body_size;
        if (limit[limit.size() - 1] != 'B' || limit[limit.size() - 2] != 'M') {
            std::cerr << "Error: Invalid client body size limit format for server : limit_client_body_size should end with MB for " << server->s_name << std::endl;
            return 0;
        }
        for (size_t i = 0; i < limit.size() - 2; ++i) {
            char c = limit[i];
            if (!isdigit(c)) {
            std::cerr << "Error: Invalid client body size limit format for server : limit_client_body_size for " << server->s_name << " contains non numbers" << std::endl;
            return 0;
            }
        }
        int limit_size = atoi(limit.substr(0, limit.size() - 2).c_str());
        if (limit_size <= 0) {
            std::cerr << "Error: Invalid client body size limit for server " << server->s_name << ": " << limit_size << std::endl;
            return 0;
        }
        server->limit_client_body_size_bytes = limit_size * 1024 * 1024;
        std::map<int, std::string>::const_iterator it = server->default_error_pages.find(404);
        if (it == server->default_error_pages.end()) {
            server->default_error_pages.insert(std::pair<int, std::string>(404, ERROR404));
            std::cout << "Warning: 404 error page not found for server " << server->s_name << ". Using default 404 error page\n";
        }
        it = server->default_error_pages.find(400);
        if (it == server->default_error_pages.end()) {
            server->default_error_pages.insert(std::pair<int, std::string>(400, ERROR400));
            std::cout << "Warning: 400 error page not found for server " << server->s_name << ". Using default 400 error page\n";
        }
        it = server->default_error_pages.find(413);
        if (it == server->default_error_pages.end()) {
            server->default_error_pages.insert(std::pair<int, std::string>(413, ERROR413));
            std::cout << "Warning: 413 error page not found for server " << server->s_name << ". Using default 413 error page\n";
        }
        it = server->default_error_pages.find(414);
        if (it == server->default_error_pages.end()) {
            server->default_error_pages.insert(std::pair<int, std::string>(414, ERROR414));
            std::cout << "Warning: 414 error page not found for server " << server->s_name << ". Using default 414 error page\n";
        }
        it = server->default_error_pages.find(500);
        if (it == server->default_error_pages.end()) {
            server->default_error_pages.insert(std::pair<int, std::string>(500, ERROR500));
            std::cout << "Warning: 500 error page not found for server " << server->s_name << ". Using default 500 error page\n";
        }

        for (std::map<int, std::string>::const_iterator error_page = server->default_error_pages.begin();
            error_page != server->default_error_pages.end(); ++error_page) {
            if (is_empty_or_whitespace(error_page->second)) {
                return 0;
            }
            std::ifstream file(error_page->second.c_str());
            if (!file) {
                std::cerr << "Error: File " << error_page->second << " does not exist for status code " << error_page->first << std::endl;
                return 0;
            }
            file.close();
        }


        // Check RouteConfig
        if (is_empty_or_whitespace(server->route.root_directory) ||
            is_empty_or_whitespace(server->route.default_file) ||
            is_empty_or_whitespace(server->route.list_directory) ||
            is_empty_or_whitespace(server->route.redirect) ||
            is_empty_or_whitespace(server->route.cgi_path) ||
            is_empty_or_whitespace(server->route.upload_path)) {
            return 0;
        }
        if (server->route.root_directory.substr(0, 2) != "./" &&
            server->route.root_directory.substr(0, 3) != "../" &&
            server->route.root_directory.substr(0, 1) != "/") {
                std::cerr << "Error: Invalid root directory for server " << server->s_name << ": " << server->route.root_directory << std::endl;
            return 0;
        }
        if (access(server->route.root_directory.c_str(), F_OK) == -1) {
            std::cerr << "Error: Invalid root directory for server " << server->s_name << ": " << server->route.root_directory << std::endl;
            return 0;
        }
        if (server->route.list_directory != "on" && server->route.list_directory != "off") {
            std::cerr << "Error: Invalid list_directory for server " << server->s_name << ": " << server->route.list_directory << std::endl;
            return 0;
        }
        std::vector<std::string>::const_iterator method;
        for (method = server->route.accepted_methods.begin(); method != server->route.accepted_methods.end(); ++method) {
            if (*method != "GET" && *method != "HEAD" && *method != "POST" && *method != "PUT" && *method != "DELETE") {
            std::cerr << "Error: Invalid method for server " << server->s_name << ": " << *method << std::endl;
            return 0;
            }
        }
        std::vector<std::string>::const_iterator old_path;
        for (old_path = server->route.old_paths.begin(); old_path != server->route.old_paths.end(); ++old_path) {
            if (old_path->substr(0, 1) != "/") {
            std::cerr << "Error: Invalid old path for server " << server->s_name << ": " << *old_path << std::endl;
            return 0;
            }
        }
        if (server->route.redirect.substr(0, 1) != "/") {
            std::cerr << "Error: Invalid redirect for server " << server->s_name << ": " << server->route.redirect << std::endl;
            return 0;
        }
        if (server->route.cgi_enable != "true" && server->route.cgi_enable != "false") {
            std::cerr << "Error: Invalid cgi_enable for server " << server->s_name << ": " << server->route.cgi_enable << std::endl;
            return 0;
        }
        if (server->route.cgi_path.substr(0, 2) != "./" &&
            server->route.cgi_path.substr(0, 3) != "../" &&
            server->route.cgi_path.substr(0, 1) != "/") {
            std::cerr << "Error: Invalid cgi_path for server " << server->s_name << ": " << server->route.cgi_path << std::endl;
        }
        if (access(server->route.cgi_path.c_str(), F_OK) == -1) {
            std::cerr << "Error: Invalid cgi_path for server " << server->s_name << ": " << server->route.cgi_path << std::endl;
            return 0;
        }
        if (server->route.upload_enable != "true" && server->route.upload_enable != "false") {
            std::cerr << "Error: Invalid upload_enable for server " << server->s_name << ": " << server->route.upload_enable << std::endl;
            return 0;
        }
        if (server->route.upload_path.substr(0, 2) != "./" &&
            server->route.upload_path.substr(0, 3) != "../" &&
            server->route.upload_path.substr(0, 1) != "/") {
            std::cerr << "Error: Invalid upload_path for server " << server->s_name << ": " << server->route.upload_path << std::endl;
            return 0;
        }
        if (access(server->route.upload_path.c_str(), F_OK) == -1) {
            std::cerr << "Error: Invalid upload_path for server " << server->s_name << ": " << server->route.upload_path << std::endl;
            return 0;
        }
    }
    return 1; // All checks passed
}

// int main(int argc, char** argv) {
//     if (argc != 2) {
//         std::cerr << "Usage: " << argv[0] << " <config-file>" << std::endl;
//         return 1;
//     }

//     WebServerConfig config;
//     config.parseConfig(argv[1]);  // Parse the config file specified by the command line

//     if (!checkConfig(config)) {
//         std::cerr << "Configuration is incomplete\n";
//         return 1;
//     }

//     // Print parsed data for verification
//     for (size_t i = 0; i < config.servers.size(); ++i) {
//         const ServerConfig& server = config.servers[i];
//         std::cout << "Server " << i + 1 << ":\n";
//         std::cout << "  Host: " << server.host << "\n";
//         std::cout << "  Port: " << server.port << "\n";
//         std::cout << "  Server Names: ";
//         for (size_t j = 0; j < server.server_names.size(); ++j) {
//             std::cout << server.server_names[j] << (j + 1 < server.server_names.size() ? ", " : "\n");
//         }
//         std::cout << "  Client Body Size Limit: " << server.limit_client_body_size << "\n";
//         std::cout << "  Default Error Pages:\n";
//         for (std::map<int, std::string>::const_iterator it = server.default_error_pages.begin();
//              it != server.default_error_pages.end(); ++it) {
//             std::cout << "    " << it->first << ": " << it->second << "\n";
//         }

//         // Print Route Configurations
//             const RouteConfig& route = server.route;
//             std::cout << "  Route " << i + 1 << ":\n";
//             std::cout << "    Root Directory: " << route.root_directory << "\n";
//             std::cout << "    Default File: " << route.default_file << "\n";
//             std::cout << "    Directory Listing: " << (route.list_directory ? "On" : "Off") << "\n";
//             std::cout << "    Accepted Methods: ";
//             for (size_t k = 0; k < route.accepted_methods.size(); ++k) {
//                 std::cout << route.accepted_methods[k] << (k + 1 < route.accepted_methods.size() ? ", " : "\n");
//             }
//             std::cout << "    Old paths: ";
//             for (size_t l = 0; l < route.old_paths.size(); ++ l) {
//                 std::cout << route.old_paths[l] << (l + 1 < route.old_paths.size() ? ", " : "\n");
//             }
//             std::cout << "    Redirect: " << route.redirect << "\n";
//             std::cout << "    CGI Enabled: " << (route.cgi_enable ? "Yes" : "No") << "\n";
//             std::cout << "    CGI Path: " << route.cgi_path << "\n";
//             std::cout << "    Upload Path: " << route.upload_path << "\n";
//     }

//     // Print CGI Global Configuration
//     std::cout << "Global CGI Configuration:\n";
//     std::cout << "  CGI Bin Path: " << config.cgi_config.cgi_bin_path << "\n";
//     std::cout << "  PHP CGI: " << config.cgi_config.php_cgi << "\n";
//     std::cout << "  Python CGI: " << config.cgi_config.python_cgi << "\n";
//     std::cout << "  Executable Extensions: " << config.cgi_config.cgi_executable_extensions << "\n";
//     // std::cout << "  Chunk Handling: " << config.chunk_handling << "\n";

//     return 0;
// }
