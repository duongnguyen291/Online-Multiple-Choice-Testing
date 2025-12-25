#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <map>
#include <set>
#include <sys/epoll.h>
#include "database.h"
#include "protocol.h"

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096

// Client connection info
struct ClientInfo {
    int sockfd;
    std::string session_token;
    int user_id;
    std::string username;
    std::string role;
};

class Server {
private:
    int server_fd;
    int epoll_fd;
    int port;
    Database* db;
    
    // Map socket fd -> ClientInfo
    std::map<int, ClientInfo> clients;
    
    // Map room_id -> set of socket fds (for broadcasting)
    std::map<int, std::set<int>> room_clients;
    
    // Setup server socket
    bool setup_server_socket();
    
    // Setup epoll
    bool setup_epoll();
    
    // Accept new connection
    void handle_new_connection();
    
    // Handle client disconnect
    void handle_client_disconnect(int client_fd);
    
    // Handle client message
    void handle_client_message(int client_fd);
    
    // Message handlers
    void handle_register(int client_fd, const json& payload);
    void handle_login(int client_fd, const json& payload);
    void handle_logout(int client_fd, const json& payload);
    void handle_practice_request(int client_fd, const json& payload);
    void handle_practice_submit(int client_fd, const json& payload);
    void handle_list_rooms(int client_fd, const json& payload);
    void handle_create_room(int client_fd, const json& payload);
    void handle_join_room(int client_fd, const json& payload);
    void handle_start_test(int client_fd, const json& payload);
    void handle_change_answer(int client_fd, const json& payload);
    void handle_submit_test(int client_fd, const json& payload);
    void handle_get_history(int client_fd, const json& payload);
    void handle_get_stats(int client_fd, const json& payload);
    void handle_view_room_results(int client_fd, const json& payload);
    
    // Question Management Handlers
    void handle_list_questions(int client_fd, const json& payload);
    void handle_create_question(int client_fd, const json& payload);
    void handle_update_question(int client_fd, const json& payload);
    void handle_delete_question(int client_fd, const json& payload);
    
    // Helper: validate session and get user info
    bool validate_session(int client_fd, const std::string& session_token, int& user_id, std::string& role);
    
    // Helper: broadcast message to all clients in a room
    void broadcast_to_room(int room_id, uint16_t msg_type, const json& payload);
    
    // Helper: broadcast message to all connected clients
    void broadcast_to_all(uint16_t msg_type, const json& payload);
    
public:
    Server(int port, Database* database);
    ~Server();
    
    // Start server (blocking)
    bool start();
    
    // Stop server
    void stop();
};

#endif // SERVER_H

