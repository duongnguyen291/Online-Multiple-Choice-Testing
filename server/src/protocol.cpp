#include "../include/protocol.h"
#include "../include/logger.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

bool Protocol::send_exact(int sockfd, const char* buffer, size_t length) {
    size_t total_sent = 0;
    while (total_sent < length) {
        ssize_t sent = send(sockfd, buffer + total_sent, length - total_sent, MSG_NOSIGNAL);
        if (sent <= 0) {
            if (sent < 0) {
                LOG_ERROR("send() failed: " + std::string(strerror(errno)));
            }
            return false;
        }
        total_sent += sent;
    }
    return true;
}

#include <poll.h>

bool Protocol::recv_exact(int sockfd, char* buffer, size_t length) {
    size_t total_received = 0;
    while (total_received < length) {
        ssize_t received = recv(sockfd, buffer + total_received, length - total_received, 0);
        if (received < 0) {
            // Non-blocking socket: EAGAIN/EWOULDBLOCK means no data available yet
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // If we have already received some data (or we are in the middle of a message body known by header),
                // we should wait a bit for the rest instead of failing immediately.
                // Using poll to wait up to 500ms for data
                struct pollfd pfd;
                pfd.fd = sockfd;
                pfd.events = POLLIN;
                int ret = poll(&pfd, 1, 500); // Wait 500ms
                
                if (ret > 0 && (pfd.revents & POLLIN)) {
                    continue; // Data available, try recv again
                }
                
                // Still no data after wait, or error
                return false; 
            }
            // Other error
            LOG_ERROR("recv() failed: " + std::string(strerror(errno)));
            return false;
        }
        if (received == 0) {
            // EOF - connection closed
            return false;
        }
        total_received += received;
    }
    return true;
}

bool Protocol::send_message(int sockfd, uint16_t msg_type, const json& payload) {
    try {
        // Serialize payload to JSON string
        std::string payload_str = payload.dump();
        uint32_t payload_length = payload_str.length();
        
        // Prepare header (network byte order) - write as raw bytes to avoid struct padding
        char header_buf[6];
        uint16_t msg_type_net = htons(msg_type);
        uint32_t payload_length_net = htonl(payload_length);
        memcpy(header_buf, &msg_type_net, 2);
        memcpy(header_buf + 2, &payload_length_net, 4);
        
        // Send header (6 bytes)
        if (!send_exact(sockfd, header_buf, 6)) {
            return false;
        }
        
        // Send payload (if any)
        if (payload_length > 0) {
            if (!send_exact(sockfd, payload_str.c_str(), payload_length)) {
                return false;
            }
        }
        
        LOG_DEBUG("Sent message type " + std::to_string(msg_type) + 
                  " with " + std::to_string(payload_length) + " bytes payload");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to send message: " + std::string(e.what()));
        return false;
    }
}

RecvResult Protocol::recv_message(int sockfd, Message& msg) {
    try {
        // Bước 1: Nhận (recv) ít nhất 6 byte vào bộ đệm (buffer)
        char header_buf[6];
        if (!recv_exact(sockfd, header_buf, 6)) {
            // Check if it's EAGAIN (no data) or error
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return RECV_NO_DATA; // No data available, but connection OK
            }
            // Check if connection closed
            char peek_buf[1];
            ssize_t peek_result = recv(sockfd, peek_buf, 1, MSG_PEEK | MSG_DONTWAIT);
            if (peek_result == 0) {
                return RECV_ERROR; // Connection closed
            }
            return RECV_ERROR; // Other error
        }
        
        // Bước 2: Đọc 2 byte đầu để lấy msg_type (nhớ dùng ntohs())
        // Bước 3: Đọc 4 byte tiếp theo để lấy payload_length (nhớ dùng ntohl())
        uint16_t msg_type_raw;
        uint32_t payload_length_raw;
        memcpy(&msg_type_raw, header_buf, 2);
        memcpy(&payload_length_raw, header_buf + 2, 4);
        
        msg.type = ntohs(msg_type_raw);
        uint32_t payload_length = ntohl(payload_length_raw);
        
        // Validate payload length (max 2MB to prevent DoS)
        if (payload_length > 2 * 1024 * 1024) {
            LOG_ERROR("Payload too large: " + std::to_string(payload_length) + " bytes");
            return RECV_ERROR;
        }
        
        // Bước 4: Nếu payload_length > 0, tiếp tục nhận (recv) cho đến khi đủ payload_length byte
        if (payload_length > 0) {
            std::vector<char> buffer(payload_length + 1);
            if (!recv_exact(sockfd, buffer.data(), payload_length)) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Partial message received - this shouldn't happen with proper TCP
                    // But we return error to be safe
                    LOG_WARN("Incomplete payload received");
                    return RECV_INCOMPLETE;
                }
                return RECV_ERROR;
            }
            buffer[payload_length] = '\0';
            
            // Bước 5: Sau khi có đủ payload_length byte, trích xuất chuỗi JSON từ bộ đệm
            // Bước 6: Phân tích (parse) chuỗi JSON và xử lý logic
            try {
                msg.payload = json::parse(buffer.data());
            } catch (const json::exception& e) {
                LOG_ERROR("Failed to parse JSON: " + std::string(e.what()));
                return RECV_ERROR;
            }
        } else {
            msg.payload = json::object();
        }
        
        LOG_DEBUG("Received message type " + std::to_string(msg.type) + 
                  " with " + std::to_string(payload_length) + " bytes payload");
        return RECV_SUCCESS;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to receive message: " + std::string(e.what()));
        return RECV_ERROR;
    }
}

bool Protocol::has_data_available(int sockfd) {
    char peek_buf[1];
    ssize_t result = recv(sockfd, peek_buf, 1, MSG_PEEK | MSG_DONTWAIT);
    if (result > 0) {
        return true; // Data available
    } else if (result == 0) {
        return false; // Connection closed
    } else {
        // EAGAIN/EWOULDBLOCK means no data
        return (errno == EAGAIN || errno == EWOULDBLOCK) ? false : false;
    }
}

json Protocol::create_error_response(int error_code, const std::string& message) {
    json response;
    response["code"] = error_code;
    response["message"] = message;
    return response;
}

json Protocol::create_success_response(const std::string& message) {
    json response;
    response["message"] = message;
    return response;
}

