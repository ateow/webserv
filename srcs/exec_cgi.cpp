#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <fcntl.h>

void execute_cgi(const std::string& script_path, const std::string& post_data) {
    int fds[2];
    pipe(fds); // Create a pipe for communication between processes

    pid_t pid = fork();

    if (pid == 0) { // Child process
        // Set up environment variables manually for execve
        std::vector<std::string> envp_str = {
            "REQUEST_METHOD=POST",
            "CONTENT_LENGTH=" + std::to_string(post_data.length()),
            "CONTENT_TYPE=application/x-www-form-urlencoded"
        };

        std::vector<char*> envp; // Convert to array of char* for execve
        for (auto& s : envp_str) {
            envp.push_back(&s[0]);
        }
        envp.push_back(nullptr); // NULL terminate the array

        // Prepare the argument list for the script
        std::vector<char*> argv = { const_cast<char*>(script_path.c_str()), nullptr };

        // Redirect stdin to read from pipe
        dup2(fds[0], STDIN_FILENO);
        close(fds[1]); // Close unused write end in child

        // Execute the CGI script using execve
        execve(script_path.c_str(), argv.data(), envp.data());

        // If execve fails:
        std::cerr << "Failed to execute CGI script\n";
        exit(1);
    } else if (pid > 0) { // Parent process
        close(fds[0]); // Close unused read end

        // Write the POST data to the CGI script
        write(fds[1], post_data.c_str(), post_data.length());
        close(fds[1]); // Close write end to send EOF to the script

        // Wait for the script to finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            std::cout << "CGI script executed successfully.\n";
        } else {
            std::cout << "CGI script failed to execute.\n";
        }
    }
}

// int main() {
//     std::string script_path = "../cgi-bin/process_data.cgi";
//     std::string post_data = "name=John+Doe&age=30"; // Example POST data

//     execute_cgi(script_path, post_data);

//     return 0;
// }
