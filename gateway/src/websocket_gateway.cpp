#include "../include/websocket_gateway.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <openssl/sha.h>
#include <cstddef>

WebSocketGateway::WebSocketGateway(int port, const std::string& backend_host, int backend_port)
    : server_fd(-1), epoll_fd(-1), port(port), 
      backend_host(backend_host), backend_port(backend_port) {
}

WebSocketGateway::~WebSocketGateway() {
    stop();
}

bool WebSocketGateway::setup_server_socket() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        return false;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket to port " << port << std::endl;
        return false;
    }
    
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return false;
    }
    
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    std::cout << "WebSocket Gateway listening on port " << port << std::endl;
    return true;
}

bool WebSocketGateway::setup_epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "Failed to create epoll" << std::endl;
        return false;
    }
    
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        std::cerr << "Failed to add server socket to epoll" << std::endl;
        return false;
    }
    
    return true;
}

bool WebSocketGateway::start() {
    if (!setup_server_socket()) return false;
    if (!setup_epoll()) return false;
    return true;
}

void WebSocketGateway::stop() {
    if (server_fd >= 0) close(server_fd);
    if (epoll_fd >= 0) close(epoll_fd);
    
    for (auto& conn : connections) {
        if (conn.second.sockfd >= 0) close(conn.second.sockfd);
        if (conn.second.backend_sockfd >= 0) close(conn.second.backend_sockfd);
    }
    connections.clear();
}

void WebSocketGateway::handle_new_connection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Failed to accept connection" << std::endl;
        }
        return;
    }
    
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    
    WebSocketConn conn;
    conn.sockfd = client_fd;
    conn.backend_sockfd = -1;
    conn.handshake_done = false;
    
    connections[client_fd] = conn;
    
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
    
    std::cout << "New WebSocket connection from " << inet_ntoa(client_addr.sin_addr) 
              << ":" << ntohs(client_addr.sin_port) << " (fd=" << client_fd << ")" << std::endl;
}

void WebSocketGateway::handle_client_disconnect(int client_fd) {
    auto it = connections.find(client_fd);
    if (it != connections.end()) {
        if (it->second.backend_sockfd >= 0) {
            close(it->second.backend_sockfd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->second.backend_sockfd, nullptr);
        }
        close(client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        connections.erase(it);
        std::cout << "WebSocket client disconnected (fd=" << client_fd << ")" << std::endl;
    }
}

// Simple base64 encoding
std::string WebSocketGateway::base64_encode(const unsigned char* input, size_t length) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string ret;
    int val = 0;
    int valb = 0;
    
    for (size_t i = 0; i < length; i++) {
        val = (val << 8) + input[i];
        valb += 8;
        while (valb >= 6) {
            valb -= 6;
            ret.push_back(base64_chars[(val >> valb) & 0x3F]);
        }
    }
    
    if (valb > 0) ret.push_back(base64_chars[(val << (6 - valb)) & 0x3F]);
    while (ret.size() % 4) ret.push_back('=');
    
    return ret;
}

// Simple SHA1 (using OpenSSL)
std::string WebSocketGateway::sha1(const std::string& input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)input.c_str(), input.length(), hash);
    return base64_encode(hash, SHA_DIGEST_LENGTH);
}

std::string WebSocketGateway::extract_header_value(const std::string& headers, const std::string& header_name) {
    size_t pos = headers.find(header_name);
    if (pos == std::string::npos) return "";
    
    pos = headers.find(":", pos);
    if (pos == std::string::npos) return "";
    
    pos++; // Skip ':'
    while (pos < headers.length() && (headers[pos] == ' ' || headers[pos] == '\t')) pos++;
    
    size_t end = headers.find("\r\n", pos);
    if (end == std::string::npos) end = headers.find("\n", pos);
    if (end == std::string::npos) return "";
    
    return headers.substr(pos, end - pos);
}

bool WebSocketGateway::do_websocket_handshake(int sockfd) {
    // Read HTTP upgrade request
    char buffer[4096];
    ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) return false;
    
    buffer[n] = '\0';
    std::string request(buffer);
    
    // Extract Sec-WebSocket-Key
    std::string ws_key = extract_header_value(request, "Sec-WebSocket-Key");
    if (ws_key.empty()) {
        std::cerr << "Invalid WebSocket upgrade request" << std::endl;
        return false;
    }
    
    // Compute accept key: Sec-WebSocket-Key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
    std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string accept_key = sha1(ws_key + magic);
    
    // Send HTTP upgrade response
    std::string response = 
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + accept_key + "\r\n"
        "\r\n";
    
    if (send(sockfd, response.c_str(), response.length(), MSG_NOSIGNAL) <= 0) {
        std::cerr << "Failed to send WebSocket upgrade response" << std::endl;
        return false;
    }
    
    std::cout << "WebSocket handshake completed (fd=" << sockfd << ")" << std::endl;
    return true;
}

// Simplified WebSocket frame parsing (assuming text frames only)
bool WebSocketGateway::parse_websocket_frame(const std::string& data, std::string& payload, bool& is_complete) {
    if (data.length() < 2) {
        is_complete = false;
        return true;
    }
    
    unsigned char byte1 = data[0];
    unsigned char byte2 = data[1];
    
    bool fin = (byte1 & 0x80) != 0;
    int opcode = byte1 & 0x0F;
    bool masked = (byte2 & 0x80) != 0;
    int len = byte2 & 0x7F;
    
    // Check opcode
    if (opcode == 8) { // Close frame
        is_complete = true;
        return false; // Signal to close connection
    }
    
    if (opcode != 1 && opcode != 2 && opcode != 0) { // Not Text, Binary, or Continuation
        // Ignore PING/PONG for simplicity or handle them
        // For now, just consume them but don't return payload
        is_complete = false; // Treat as if we need more data to skip effective processing here (simplified)
        // ideally we should advance buffer but return "no payload"
        return true; 
    }

    // Parse payload length
    size_t header_len = 2;
    if (len == 126) {
        if (data.length() < 4) {
            is_complete = false;
            return true;
        }
        len = ((unsigned char)data[2] << 8) | (unsigned char)data[3];
        header_len = 4;
    } else if (len == 127) {
        if (data.length() < 10) {
            is_complete = false;
            return true;
        }
        // For simplicity, assume length fits in uint32_t
        len = ((unsigned char)data[6] << 24) | ((unsigned char)data[7] << 16) |
              ((unsigned char)data[8] << 8) | (unsigned char)data[9];
        header_len = 10;
    }
    
    // Parse mask key (if masked)
    if (masked) {
        if (data.length() < header_len + 4) {
            is_complete = false;
            return true;
        }
        header_len += 4;
    }
    
    // Check if we have complete frame
    if ((int)data.length() < header_len + len) {
        is_complete = false;
        return true;
    }
    
    // Extract and unmask payload
    std::string frame_payload(data.begin() + header_len, data.begin() + header_len + len);
    
    if (masked) {
        unsigned char mask[4];
        for (int i = 0; i < 4; i++) {
            mask[i] = data[header_len - 4 + i];
        }
        for (int i = 0; i < len; i++) {
            frame_payload[i] ^= mask[i % 4];
        }
    }
    
    payload = frame_payload;
    is_complete = fin;
    
    // Only return true (valid payload) if it is a Text frame (opcode 1)
    // We already handled Close (8).
    if (opcode != 1 && opcode != 0) { // 0 is continuation (assuming text)
       // If it was a control frame like PING, we technically consumed it but didn't act.
       // Clearing payload to avoid JSON parse error
       payload = "";
    }
    return true;
}

// Create WebSocket frame (assuming text frame, unmasked)
std::string WebSocketGateway::create_websocket_frame(const std::string& payload) {
    std::string frame;
    
    // FIN=1 (0x80), opcode=1 (text) (0x01)
    frame.push_back(0x81);
    
    // Mask=0, length
    size_t len = payload.length();
    if (len < 126) {
        frame.push_back((unsigned char)len);
    } else if (len < 65536) {
        frame.push_back(126);
        frame.push_back((unsigned char)(len >> 8));
        frame.push_back((unsigned char)(len & 0xFF));
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) {
            frame.push_back((unsigned char)((len >> (i * 8)) & 0xFF));
        }
    }
    
    frame.append(payload);
    return frame;
}

int WebSocketGateway::connect_to_backend() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create backend socket" << std::endl;
        return -1;
    }
    
    struct hostent *server = gethostbyname(backend_host.c_str());
    if (server == NULL) {
        std::cerr << "No such host: " << backend_host << std::endl;
        close(sockfd);
        return -1;
    }
    
    struct sockaddr_in addr;
    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);
    addr.sin_port = htons(backend_port);
    
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to connect to backend " << backend_host << ":" << backend_port << std::endl;
        close(sockfd);
        return -1;
    }
    
    // Set non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    // Add to epoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sockfd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event);
    
    std::cout << "Connected to backend server (fd=" << sockfd << ")" << std::endl;
    return sockfd;
}

void WebSocketGateway::forward_ws_to_backend(int client_fd, const std::string& json_str) {
    auto it = connections.find(client_fd);
    if (it == connections.end()) return;
    
    WebSocketConn& conn = it->second;
    
    // Create backend connection if needed
    if (conn.backend_sockfd < 0) {
        conn.backend_sockfd = connect_to_backend();
        if (conn.backend_sockfd < 0) {
            std::cerr << "Failed to create backend connection" << std::endl;
            return;
        }
    }
    
    try {
        // Parse JSON to extract message type
        auto json_obj = json::parse(json_str);
        
        // Extract message type (assuming it's in "type" field)
        if (!json_obj.contains("type")) {
            std::cerr << "Message missing 'type' field" << std::endl;
            return;
        }
        
        uint16_t msg_type = json_obj["type"];
        
        // Prepare payload (the entire JSON)
        std::string payload = json_str;
        uint32_t payload_length = payload.length();
        
        // Prepare header (network byte order)
        char header_buf[6];
        uint16_t msg_type_net = htons(msg_type);
        uint32_t payload_length_net = htonl(payload_length);
        memcpy(header_buf, &msg_type_net, 2);
        memcpy(header_buf + 2, &payload_length_net, 4);
        
        // Combine header and payload into single buffer to avoid fragmentation
        std::vector<char> send_buffer(6 + payload_length);
        memcpy(send_buffer.data(), header_buf, 6);
        if (payload_length > 0) {
            memcpy(send_buffer.data() + 6, payload.c_str(), payload_length);
        }
        
        // Send to backend in one go
        if (send(conn.backend_sockfd, send_buffer.data(), send_buffer.size(), MSG_NOSIGNAL) <= 0) {
            std::cerr << "Failed to send data to backend" << std::endl;
            return;
        }
        
        std::cout << "Forwarded message type " << msg_type << " to backend" << std::endl;
    } catch (const json::exception& e) {
        std::cerr << "Failed to parse JSON from WebSocket: " << e.what() << std::endl;
    }
}

void WebSocketGateway::forward_backend_to_ws(int client_fd, const std::string& data) {
    try {
        // Parse [Type][Length][JSON] from backend
        if (data.length() < 6) return;
        
        uint16_t msg_type_raw;
        uint32_t payload_length_raw;
        memcpy(&msg_type_raw, data.c_str(), 2);
        memcpy(&payload_length_raw, data.c_str() + 2, 4);
        
        uint16_t msg_type = ntohs(msg_type_raw);
        uint32_t payload_length = ntohl(payload_length_raw);
        
        if (data.length() < 6 + payload_length) return;
        
        std::string payload(data.c_str() + 6, payload_length);
        
        // Parse payload as JSON
        auto json_obj = json::parse(payload);
        
        // Add message type field
        json_obj["type"] = msg_type;
        
        // Convert to JSON string
        std::string json_str = json_obj.dump();
        
        // Create WebSocket frame
        std::string ws_frame = create_websocket_frame(json_str);
        
        // Send to WebSocket client
        if (send(client_fd, ws_frame.c_str(), ws_frame.length(), MSG_NOSIGNAL) > 0) {
            std::cout << "Forwarded message type " << msg_type << " to WebSocket client" << std::endl;
        } else {
            std::cerr << "Failed to send WebSocket frame to client" << std::endl;
        }
    } catch (const json::exception& e) {
        std::cerr << "Failed to parse JSON from backend: " << e.what() << std::endl;
    }
}

void WebSocketGateway::handle_websocket_data(int client_fd) {
    auto it = connections.find(client_fd);
    if (it == connections.end()) return;
    
    WebSocketConn& conn = it->second;
    
    // Do handshake if not done yet
    if (!conn.handshake_done) {
        if (!do_websocket_handshake(client_fd)) {
            handle_client_disconnect(client_fd);
            return;
        }
        conn.handshake_done = true;
        return;
    }
    
    // Read WebSocket frame data
    char buffer[WS_BUFFER_SIZE];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
    
    if (n <= 0) {
        handle_client_disconnect(client_fd);
        return;
    }
    
    conn.ws_buffer.append(buffer, n);
    
    // Try to parse complete frames
    while (!conn.ws_buffer.empty()) {
        std::string payload;
        bool is_complete;
        
        if (!parse_websocket_frame(conn.ws_buffer, payload, is_complete)) {
            // Error
            handle_client_disconnect(client_fd);
            return;
        }
        
        if (!is_complete) {
            // Incomplete frame, wait for more data
            break;
        }
        
        // Remove parsed frame from buffer
        // Find where this frame ends
        unsigned char byte2 = conn.ws_buffer[1];
        bool masked = (byte2 & 0x80) != 0;
        int len = byte2 & 0x7F;
        
        size_t header_len = 2;
        if (len == 126) header_len = 4;
        else if (len == 127) header_len = 10;
        
        if (masked) header_len += 4;
        
        if (len == 126) {
            len = ((unsigned char)conn.ws_buffer[2] << 8) | (unsigned char)conn.ws_buffer[3];
        } else if (len == 127) {
            len = ((unsigned char)conn.ws_buffer[6] << 24) | ((unsigned char)conn.ws_buffer[7] << 16) |
                  ((unsigned char)conn.ws_buffer[8] << 8) | (unsigned char)conn.ws_buffer[9];
        }
        
        conn.ws_buffer.erase(0, header_len + len);
        
        // Forward JSON to backend if not empty
        if (!payload.empty()) {
            forward_ws_to_backend(client_fd, payload);
        }
    }
}

void WebSocketGateway::handle_backend_data(int backend_fd) {
    // Find which client this backend fd belongs to
    int client_fd = -1;
    for (auto& conn : connections) {
        if (conn.second.backend_sockfd == backend_fd) {
            client_fd = conn.first;
            break;
        }
    }
    
    if (client_fd < 0) return;
    
    WebSocketConn& conn = connections[client_fd];
    
    char buffer[4096];
    ssize_t n = recv(backend_fd, buffer, sizeof(buffer), 0);
    
    if (n <= 0) {
        // Backend disconnected, close client too
        handle_client_disconnect(client_fd);
        return;
    }
    
    conn.backend_buffer.append(buffer, n);
    
    // Try to parse and forward complete backend messages
    while (conn.backend_buffer.length() >= 6) {
        uint32_t payload_length_raw;
        memcpy(&payload_length_raw, conn.backend_buffer.c_str() + 2, 4);
        uint32_t payload_length = ntohl(payload_length_raw);
        
        if ((int)conn.backend_buffer.length() < 6 + payload_length) {
            break; // Incomplete message
        }
        
        // Forward complete message to WebSocket client
        std::string msg(conn.backend_buffer.c_str(), 6 + payload_length);
        forward_backend_to_ws(client_fd, msg);
        
        // Remove from buffer
        conn.backend_buffer.erase(0, 6 + payload_length);
    }
}

void WebSocketGateway::run() {
    struct epoll_event events[MAX_EVENTS];
    
    std::cout << "WebSocket Gateway running (waiting for connections)..." << std::endl;
    
    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            std::cerr << "epoll_wait error" << std::endl;
            break;
        }
        
        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;
            
            if (fd == server_fd) {
                // New connection
                handle_new_connection();
            } else {
                // Check if it's a WebSocket client or backend connection
                auto it = connections.find(fd);
                if (it != connections.end()) {
                    // WebSocket client
                    handle_websocket_data(fd);
                } else {
                    // Must be a backend connection
                    handle_backend_data(fd);
                }
            }
        }
    }
}
