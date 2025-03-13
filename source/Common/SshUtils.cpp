#include <libssh/libssh.h>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "Common/SshUtils.hpp"
#include "Elite/Log.hpp"

namespace ELITE {

namespace SSH_UTILS {

#if defined(__linux) || defined(linux) || defined(__linux__)

std::string executeCommand(const std::string &host, const std::string &user,
                           const std::string &password,
                           const std::string &cmd) {
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
}



bool downloadFile(const std::string& server, const std::string& user, 
                  const std::string& password, const std::string& remote_path, 
                  const std::string& local_path, 
                  std::function<void (int f_z, int r_z, const char* err)> progress_cb) {
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
}

#endif

} // namespace SSH_UTILS

} // namespace ELITE
