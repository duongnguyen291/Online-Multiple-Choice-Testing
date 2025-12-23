#ifndef WEBSOCKET_GATEWAY_H
#define WEBSOCKET_GATEWAY_H

#include <string>
#include <map>
#include <set>
#include <sys/epoll.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096
#define WS_BUFFER_SIZE 65536

// WebSocket connection info
struct WebSocketConn {
    int sockfd;
    std::string session_token;
    int backend_sockfd;  // Connection to backend TCP server
    std::string ws_buffer;  // Buffer for WebSocket frame reassembly
    std::string backend_buffer;  // Buffer for backend response reassembly
    bool handshake_done;
};

class WebSocketGateway {
private:
    int server_fd;
    int epoll_fd;
    int port;
    std::string backend_host;
    int backend_port;
    
    // Map WebSocket socket fd -> WebSocketConn
    std::map<int, WebSocketConn> connections;
    
    bool setup_server_socket();
    bool setup_epoll();
    void handle_new_connection();
    void handle_client_disconnect(int client_fd);
    void handle_websocket_data(int client_fd);
    void handle_backend_data(int backend_fd);
    
    // WebSocket handling
    bool do_websocket_handshake(int sockfd);
    bool parse_websocket_frame(const std::string& data, std::string& payload, bool& is_complete);
    std::string create_websocket_frame(const std::string& payload);
    std::string sha1(const std::string& input);
    std::string base64_encode(const unsigned char* input, size_t length);
    
    // Backend TCP connection
    int connect_to_backend();
    
    // Protocol conversion
    void forward_ws_to_backend(int client_fd, const std::string& json_str);
    void forward_backend_to_ws(int client_fd, const std::string& data);
    
    // Helper
    std::string extract_header_value(const std::string& headers, const std::string& header_name);
    
public:
    WebSocketGateway(int port, const std::string& backend_host, int backend_port);
    ~WebSocketGateway();
    
    bool start();
    void stop();
    
    void run();  // Main event loop
};

#endif // WEBSOCKET_GATEWAY_H
