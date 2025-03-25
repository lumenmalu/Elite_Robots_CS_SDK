#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <sstream>
#if defined(__linux) || defined(linux) || defined(__linux__)
    #ifdef ELITE_USE_LIB_SSH
        #include <libssh/libssh.h>
    #else
        #include <sys/types.h>
        #include <sys/wait.h>
        #include <unistd.h>
    #endif
#include <errno.h>
#include <string.h>
#endif

#include "Common/SshUtils.hpp"
#include "Elite/Log.hpp"

namespace ELITE {

namespace SSH_UTILS {

#if defined(__linux) || defined(linux) || defined(__linux__)

std::string executeCommand(const std::string &host, const std::string &user,
                           const std::string &password,
                           const std::string &cmd) {
#ifdef ELITE_USE_LIB_SSH
    ssh_session session = ssh_new();
    if (!session) {
        ELITE_LOG_ERROR("Failed to create SSH session");
        return "";
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());
    
    if (ssh_connect(session) != SSH_OK) {
        ELITE_LOG_ERROR("SSH connection failed: %s", ssh_get_error(session));
        ssh_free(session);
        return "";
    }
    
    if (ssh_userauth_password(session, nullptr, password.c_str()) != SSH_AUTH_SUCCESS) {
        ELITE_LOG_ERROR("Authentication failed: %s", ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        return "";
    }
    
    ssh_channel channel = ssh_channel_new(session);
    if (!channel) {
        ELITE_LOG_ERROR("Failed to create SSH channel");
        ssh_disconnect(session);
        ssh_free(session);
        return "";
    }
    
    if (ssh_channel_open_session(channel) != SSH_OK) {
        ELITE_LOG_ERROR("Failed to open SSH channel: %s", ssh_get_error(session));
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return "";
    }
    
    if (ssh_channel_request_exec(channel, cmd.c_str()) != SSH_OK) {
        ELITE_LOG_ERROR("Failed to execute command: ", ssh_get_error(session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return "";
    }
    
    char buffer[4096];
    int nbytes;
    std::string result;
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[nbytes] = '\0';
        result += buffer;
    }
    
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);
    
    return result;
#else
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        char buf[256] = {0};
        ELITE_LOG_ERROR("Execute cmd \"%s\" fail: %d", cmd.c_str(), strerror_r(errno, buf, sizeof(buf)));
        return "";
    }

    pid_t pid = fork();
    if (pid == -1) {
        char buf[256] = {0};
        ELITE_LOG_ERROR("Execute cmd \"%s\" fail: %d", cmd.c_str(), strerror_r(errno, buf, sizeof(buf)));
        return "";
    }

    if (pid == 0) { // child process
        // Close reader
        close(pipefd[0]); 
        // Redirect stdout to a pipe.
        dup2(pipefd[1], STDOUT_FILENO);
        // Close the writter (which has been duplicated to stdout).
        close(pipefd[1]); 
        execlp("sshpass", "sshpass", "-p", password.c_str(), 
               "ssh", "-o", "StrictHostKeyChecking=no", 
               (user + "@" + host).c_str(), cmd.c_str(), nullptr);
        char err_buf[256] = {0};
        ELITE_LOG_ERROR("Execute cmd \"%s\" fail: %d", cmd.c_str(), strerror_r(errno, err_buf, sizeof(err_buf)));
        exit(1);
    } else {
        // Close the writter
        close(pipefd[1]);
        char buffer[256];
        std::ostringstream result;

        // Read child porcess output
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            result << buffer;
        }
        // Close reader
        close(pipefd[0]);
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            ELITE_LOG_INFO("Execute command \"%s\" exited with status: %d", cmd.c_str(), WEXITSTATUS(status));
        }
        return result.str();
    }
#endif
}



bool downloadFile(const std::string& server, const std::string& user, 
                  const std::string& password, const std::string& remote_path, 
                  const std::string& local_path, 
                  std::function<void (int f_z, int r_z, const char* err)> progress_cb) {
#ifdef ELITE_USE_LIB_SSH
    // Read 64 KB each time.
    constexpr int CHUNK_SIZE = 65536;
    ssh_session session = ssh_new();
    if (!session) {
        ELITE_LOG_ERROR("Failed to create SSH session");
        return false;
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, server.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());

    if (ssh_connect(session) != SSH_OK) {
        ELITE_LOG_ERROR("SSH connection failed: %s",ssh_get_error(session));
        ssh_free(session);
        return false;
    }

    if (ssh_userauth_password(session, nullptr, password.c_str()) != SSH_AUTH_SUCCESS) {
        ELITE_LOG_ERROR("SSH authentication failed: %s", ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    ssh_scp scp = ssh_scp_new(session, SSH_SCP_READ, remote_path.c_str());
    if (!scp || ssh_scp_init(scp) != SSH_OK) {
        ELITE_LOG_ERROR("Failed to initialize SCP: %s", ssh_get_error(session));
        ssh_scp_free(scp);
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    if (ssh_scp_pull_request(scp) != SSH_SCP_REQUEST_NEWFILE) {
        ELITE_LOG_ERROR("Failed to pull SCP request: %s", ssh_get_error(session));
        ssh_scp_free(scp);
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    int file_size = ssh_scp_request_get_size(scp);
    std::string file_name = ssh_scp_request_get_filename(scp);
    ELITE_LOG_INFO("Downloading: %s (%d bytes)", file_name.c_str(), file_size);

    std::ofstream local_file(local_path, std::ios::binary);
    if (!local_file) {
        ELITE_LOG_ERROR("Failed to open local file: %s", local_path.c_str());
        ssh_scp_free(scp);
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    // Read in chunks.
    int total_read = 0;
    std::vector<char> buffer(CHUNK_SIZE);
    while (total_read < file_size) {
        int bytes_to_read = std::min(CHUNK_SIZE, file_size - total_read);
        int bytes_read = ssh_scp_read(scp, buffer.data(), bytes_to_read);

        if (bytes_read == SSH_ERROR) {
            const char* ssh_err = ssh_get_error(session);
            progress_cb(file_size, total_read, ssh_err);
            ELITE_LOG_ERROR("SCP read error: %s", ssh_err);
            break;
        }

        local_file.write(buffer.data(), bytes_read);
        total_read += bytes_read;

        if (progress_cb) {
            progress_cb(file_size, total_read, nullptr);
        }
    }

    ELITE_LOG_INFO("Download complete!");

    local_file.close();
    ssh_scp_free(scp);
    ssh_disconnect(session);
    ssh_free(session);
    return true;
#else
    (void)progress_cb;
    pid_t pid = fork();
    if (pid == -1) {
        char err_buf[256] = {0};
        ELITE_LOG_ERROR("scp file \"%s\" fail: %d", remote_path.c_str(), strerror_r(errno, err_buf, sizeof(err_buf)));
        return false;
    }

    if (pid == 0) {
        execlp("sshpass", "sshpass", "-p", password.c_str(), 
               "scp", "-o", "StrictHostKeyChecking=no", 
               (user + "@" + server + ":" + remote_path).c_str(), 
               local_path.c_str(), 
               nullptr);
        perror("execlp failed");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            ELITE_LOG_INFO("scp file \"%s\" exited with status: %d", remote_path.c_str(), WEXITSTATUS(status));
            if (status) {
                return false;
            } else {
                return true;
            }
        }
    }
#endif
}

#endif

} // namespace SSH_UTILS

} // namespace ELITE
