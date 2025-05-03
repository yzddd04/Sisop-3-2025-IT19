#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define DATABASE_DIR "server/database/"
#define LOG_FILE "server/server.log"

void log_action(const char *source, const char *action, const char *info) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        fprintf(log_file, "[%s][%04d-%02d-%02d %02d:%02d:%02d]: [%s] [%s]\n",
                source,
                tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
                action, info);
        fclose(log_file);
    }
}

char* reverse_string(char *str) {
    if (!str) return NULL;
    
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    return str;
}

char* hex_decode(const char *hex_str, size_t *out_len) {
    if (!hex_str || strlen(hex_str) % 2 != 0) return NULL;
    
    size_t len = strlen(hex_str) / 2;
    char *bytes = malloc(len + 1);
    if (!bytes) return NULL;
    
    for (size_t i = 0; i < len; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &bytes[i]);
    }
    bytes[len] = '\0';
    
    if (out_len) *out_len = len;
    return bytes;
}

int decrypt_and_save(const char *text_data, char *filename) {
    // Reverse the text
    char *reversed = strdup(text_data);
    if (!reversed) return 0;
    reverse_string(reversed);
    
    // Hex decode
    size_t decoded_len;
    char *decoded = hex_decode(reversed, &decoded_len);
    free(reversed);
    if (!decoded) return 0;
    
    // Generate timestamp filename
    time_t now = time(NULL);
    snprintf(filename, 64, "%ld.jpeg", now);
    
    // Save to database
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", DATABASE_DIR, filename);
    
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        free(decoded);
        return 0;
    }
    
    fwrite(decoded, 1, decoded_len, fp);
    fclose(fp);
    free(decoded);
    
    return 1;
}

int send_file(int client_socket, const char *filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", DATABASE_DIR, filename);
    
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return 0;
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Send file size first
    write(client_socket, &file_size, sizeof(file_size));
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        if (write(client_socket, buffer, bytes_read) != bytes_read) {
            fclose(fp);
            return 0;
        }
    }
    
    fclose(fp);
    return 1;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int option;
    
    while (1) {
        // Read client option
        if (read(client_socket, &option, sizeof(option)) <= 0) break;
        
        if (option == 1) { // Decrypt and save
            // Read filename length
            size_t filename_len;
            read(client_socket, &filename_len, sizeof(filename_len));
            
            // Read filename
            char filename[filename_len + 1];
            read(client_socket, filename, filename_len);
            filename[filename_len] = '\0';
            
            // Read file content length
            size_t file_content_len;
            read(client_socket, &file_content_len, sizeof(file_content_len));
            
            // Read file content
            char file_content[file_content_len + 1];
            read(client_socket, file_content, file_content_len);
            file_content[file_content_len] = '\0';
            
            log_action("Client", "DECRYPT", filename);
            
            // Process and save
            char output_filename[64];
            int success = decrypt_and_save(file_content, output_filename);
            
            if (success) {
                write(client_socket, "SUCCESS", 8);
                write(client_socket, output_filename, strlen(output_filename) + 1);
                log_action("Server", "SAVE", output_filename);
            } else {
                write(client_socket, "FAILURE", 8);
            }
        }
        else if (option == 2) { // Download file
            // Read filename length
            size_t filename_len;
            read(client_socket, &filename_len, sizeof(filename_len));
            
            // Read filename
            char filename[filename_len + 1];
            read(client_socket, filename, filename_len);
            filename[filename_len] = '\0';
            
            log_action("Client", "DOWNLOAD", filename);
            
            // Try to send file
            int success = send_file(client_socket, filename);
            
            if (success) {
                log_action("Server", "UPLOAD", filename);
            } else {
                long file_size = -1;
                write(client_socket, &file_size, sizeof(file_size));
            }
        }
        else if (option == 3) { // Exit
            log_action("Client", "EXIT", "Client requested to exit");
            break;
        }
    }
    
    close(client_socket);
}

int main() {
    // Create database directory if not exists
    mkdir("server", 0755);
    mkdir(DATABASE_DIR, 0755);
    
    // Daemonize
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    umask(0);
    setsid();
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_fd, 3) < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Server loop
    while (1) {
        int client_socket;
        int addrlen = sizeof(address);
        
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) continue;
        
        // Handle client in a new process
        pid_t child_pid = fork();
        if (child_pid < 0) {
            close(client_socket);
            continue;
        }
        
        if (child_pid == 0) { // Child process
            close(server_fd);
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else { // Parent process
            close(client_socket);
        }
    }
    
    return 0;
}