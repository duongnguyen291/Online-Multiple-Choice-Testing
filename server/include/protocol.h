#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Message type constants (opcodes)
// Client to Server (C2S)
#define C2S_REGISTER           101
#define C2S_LOGIN             102
#define C2S_LOGOUT            103
#define C2S_PRACTICE_REQUEST  201
#define C2S_PRACTICE_SUBMIT   202
#define C2S_LIST_ROOMS        301
#define C2S_CREATE_ROOM       302
#define C2S_JOIN_ROOM         303
#define C2S_START_TEST        401
#define C2S_CHANGE_ANSWER     402
#define C2S_SUBMIT_TEST       403
#define C2S_GET_HISTORY       501
#define C2S_GET_STATS         502
#define C2S_VIEW_ROOM_RESULTS 503

// Server to Client (S2C)
#define S2C_RESPONSE_OK           801
#define S2C_RESPONSE_ERROR        802
#define S2C_LOGIN_OK              803
#define S2C_PRACTICE_QUESTIONS    901
#define S2C_PRACTICE_RESULT       902
#define S2C_ROOM_LIST            1001
#define S2C_ROOM_CREATED         1002
#define S2C_JOIN_OK              1003
#define S2C_USER_JOINED_ROOM     1004
#define S2C_ROOM_STATUS_CHANGED  1005
#define S2C_TEST_STARTED         1101
#define S2C_TEST_ENDED           1102
#define S2C_YOUR_RESULT          1103
#define S2C_HISTORY_DATA         1201
#define S2C_STATS_DATA           1202
#define S2C_ROOM_RESULTS_DATA    1203

// Error codes
#define ERR_LOGIN_FAILED       1001
#define ERR_USERNAME_EXISTS    1002
#define ERR_ROOM_NOT_FOUND     2001
#define ERR_ROOM_STARTED       2002
#define ERR_NOT_ROOM_OWNER     2003
#define ERR_INVALID_SESSION    3001
#define ERR_SYSTEM_ERROR       9999

// Protocol header structure (6 bytes)
// Note: Using packed struct to avoid alignment issues
struct __attribute__((packed)) ProtocolHeader {
    uint16_t msg_type;      // 2 bytes
    uint32_t payload_length; // 4 bytes
};

// Protocol message structure
struct Message {
    uint16_t type;
    json payload;
    
    Message() : type(0) {}
    Message(uint16_t t, const json& p) : type(t), payload(p) {}
};

// Receive result enum
enum RecvResult {
    RECV_SUCCESS,      // Message received successfully
    RECV_NO_DATA,      // No data available (EAGAIN), connection OK
    RECV_ERROR,        // Error or connection closed
    RECV_INCOMPLETE    // Partial data received (should not happen with proper implementation)
};

class Protocol {
public:
    // Send message: [Type][Length][JSON Payload]
    static bool send_message(int sockfd, uint16_t msg_type, const json& payload);
    
    // Receive message: [Type][Length][JSON Payload]
    // Returns: RECV_SUCCESS if message received, RECV_NO_DATA if no data (EAGAIN),
    //          RECV_ERROR if error or connection closed
    static RecvResult recv_message(int sockfd, Message& msg);
    
    // Helper: receive exact N bytes
    // Returns: true if all bytes received, false if error or EAGAIN
    static bool recv_exact(int sockfd, char* buffer, size_t length);
    
    // Helper: check if socket has data available (non-blocking peek)
    static bool has_data_available(int sockfd);
    
    // Helper: send exact N bytes
    static bool send_exact(int sockfd, const char* buffer, size_t length);
    
    // Create error response
    static json create_error_response(int error_code, const std::string& message);
    
    // Create success response
    static json create_success_response(const std::string& message);
};

#endif // PROTOCOL_H

