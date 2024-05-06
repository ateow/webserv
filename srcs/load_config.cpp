#include "load_config.hpp"

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
                else if (key == "list_directory") route.list_directory = (value == "on");
                else if (key == "accepted_methods") {
                    std::istringstream iss(value);
                    std::string method;
                    while (getline(iss, method, ' ')) {
                        trim(method);
                        route.accepted_methods.push_back(method);
                    }
                }
                else if (key == "redirect") route.redirect = value;
                else if (key == "cgi_enable") route.cgi_enable = (value == "true");
                else if (key == "cgi_path") route.cgi_path = value;
                else if (key == "cgi_extensions") route.cgi_extensions = value;
                else if (key == "upload_enable") route.upload_enable = (value == "true");
                else if (key == "upload_path") route.upload_path = value;
            } else if (section == "cgi_global") {
                if (key == "cgi_bin_path") cgi_config.cgi_bin_path = value;
                else if (key == "php_cgi") cgi_config.php_cgi = value;
                else if (key == "python_cgi") cgi_config.python_cgi = value;
                else if (key == "cgi_executable_extensions") cgi_config.cgi_executable_extensions = value;
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

int checkConfig(const WebServerConfig& config) {
    // Check CGIConfig
    if (is_empty_or_whitespace(config.cgi_config.cgi_bin_path) ||
        is_empty_or_whitespace(config.cgi_config.php_cgi) ||
        is_empty_or_whitespace(config.cgi_config.python_cgi) ||
        is_empty_or_whitespace(config.cgi_config.cgi_executable_extensions)) {
        return 0;
    }

    // Check WebServerConfig
    // if (is_empty_or_whitespace(config.chunk_handling)) {
    //     return 0;
    // }

    for (std::vector<ServerConfig>::const_iterator server = config.servers.begin(); server != config.servers.end(); ++server) {
        // Check ServerConfig
        if (is_empty_or_whitespace(server->host) ||
            server->port == 0 || // Port should be greater than zero
            is_empty_or_whitespace(server->s_name) ||
            is_empty_or_whitespace(server->limit_client_body_size)) {
            return 0;
        }

        for (std::map<int, std::string>::const_iterator error_page = server->default_error_pages.begin();
             error_page != server->default_error_pages.end(); ++error_page) {
            if (is_empty_or_whitespace(error_page->second)) {
                return 0;
            }
        }

        // Check RouteConfig
        if (is_empty_or_whitespace(server->route.root_directory) ||
            is_empty_or_whitespace(server->route.default_file) ||
            is_empty_or_whitespace(server->route.redirect) ||
            is_empty_or_whitespace(server->route.cgi_path) ||
            is_empty_or_whitespace(server->route.cgi_extensions) ||
            is_empty_or_whitespace(server->route.upload_path)) {
            return 0;
        }

        if (server->route.accepted_methods.empty()) {
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
//             std::cout << "    Redirect: " << route.redirect << "\n";
//             std::cout << "    CGI Enabled: " << (route.cgi_enable ? "Yes" : "No") << "\n";
//             std::cout << "    CGI Path: " << route.cgi_path << "\n";
//             std::cout << "    CGI Extensions: " << route.cgi_extensions << "\n";
//             std::cout << "    Upload Enabled: " << (route.upload_enable ? "Yes" : "No") << "\n";
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
