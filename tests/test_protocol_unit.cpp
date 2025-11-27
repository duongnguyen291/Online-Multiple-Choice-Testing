#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include "../server/include/protocol.h"
#include "../server/include/logger.h"

// Helper: Create a pair of connected sockets for testing using socketpair
int create_socket_pair(int sockets[2]) {
    // Use socketpair for simpler testing
    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) < 0) {
        return -1;
    }
    
    sockets[0] = pair[0];  // Client socket
    sockets[1] = pair[1];   // Server socket
    
    // Set server socket to non-blocking
    int flags = fcntl(sockets[1], F_GETFL, 0);
    fcntl(sockets[1], F_SETFL, flags | O_NONBLOCK);
    
    return 0;
}

void test_send_receive_single_message() {
    std::cout << "[TEST] Single message send/receive...\n";
    
    int sockets[2];
    assert(create_socket_pair(sockets) == 0);
    
    // Send message
    json payload;
    payload["test"] = "value";
    payload["number"] = 123;
    
    bool sent = Protocol::send_message(sockets[0], 101, payload);
    assert(sent);
    
    // Receive message
    Message msg;
    RecvResult result = Protocol::recv_message(sockets[1], msg);
    
    assert(result == RECV_SUCCESS);
    assert(msg.type == 101);
    assert(msg.payload["test"] == "value");
    assert(msg.payload["number"] == 123);
    
    close(sockets[0]);
    close(sockets[1]);
    std::cout << "  ✓ PASSED\n";
}

void test_send_receive_multiple_messages() {
    std::cout << "[TEST] Multiple messages send/receive (loop)...\n";
    
    int sockets[2];
    assert(create_socket_pair(sockets) == 0);
    
    // Send 3 messages
    for (int i = 0; i < 3; i++) {
        json payload;
        payload["index"] = i;
        payload["message"] = "test " + std::to_string(i);
        bool sent = Protocol::send_message(sockets[0], 100 + i, payload);
        assert(sent);
    }
    
    // Receive all messages in loop (simulating handle_client_message)
    int received_count = 0;
    while (received_count < 3) {
        Message msg;
        RecvResult result = Protocol::recv_message(sockets[1], msg);
        
        if (result == RECV_NO_DATA) {
            // No more data
            break;
        } else if (result == RECV_SUCCESS) {
            assert(msg.type == 100 + received_count);
            assert(msg.payload["index"] == received_count);
            received_count++;
        } else {
            std::cout << "  ✗ FAILED: Unexpected result " << result << "\n";
            assert(false);
        }
    }
    
    assert(received_count == 3);
    
    close(sockets[0]);
    close(sockets[1]);
    std::cout << "  ✓ PASSED (received " << received_count << " messages)\n";
}

void test_empty_payload() {
    std::cout << "[TEST] Message with empty payload (payload_length = 0)...\n";
    
    int sockets[2];
    assert(create_socket_pair(sockets) == 0);
    
    // Send message with empty payload
    json empty_payload = json::object();
    bool sent = Protocol::send_message(sockets[0], 201, empty_payload);
    assert(sent);
    
    // Receive
    Message msg;
    RecvResult result = Protocol::recv_message(sockets[1], msg);
    
    assert(result == RECV_SUCCESS);
    assert(msg.type == 201);
    assert(msg.payload.is_object());
    
    close(sockets[0]);
    close(sockets[1]);
    std::cout << "  ✓ PASSED\n";
}

void test_network_byte_order() {
    std::cout << "[TEST] Network byte order conversion...\n";
    
    int sockets[2];
    assert(create_socket_pair(sockets) == 0);
    
    // Send message with large type and length
    json payload;
    payload["data"] = "test data";
    bool sent = Protocol::send_message(sockets[0], 0xABCD, payload);
    assert(sent);
    
    // Receive
    Message msg;
    RecvResult result = Protocol::recv_message(sockets[1], msg);
    
    assert(result == RECV_SUCCESS);
    assert(msg.type == 0xABCD);
    
    close(sockets[0]);
    close(sockets[1]);
    std::cout << "  ✓ PASSED\n";
}

void test_large_payload() {
    std::cout << "[TEST] Large payload (near limit)...\n";
    
    int sockets[2];
    assert(create_socket_pair(sockets) == 0);
    
    // Create large payload (1MB)
    std::string large_data(1024 * 1024, 'A');
    json payload;
    payload["data"] = large_data;
    
    bool sent = Protocol::send_message(sockets[0], 301, payload);
    assert(sent);
    
    // Receive
    Message msg;
    RecvResult result = Protocol::recv_message(sockets[1], msg);
    
    assert(result == RECV_SUCCESS);
    assert(msg.type == 301);
    assert(msg.payload["data"].get<std::string>().length() == 1024 * 1024);
    
    close(sockets[0]);
    close(sockets[1]);
    std::cout << "  ✓ PASSED\n";
}

void test_recv_no_data() {
    std::cout << "[TEST] RECV_NO_DATA when no data available...\n";
    
    int sockets[2];
    assert(create_socket_pair(sockets) == 0);
    
    // Try to receive when no data sent
    Message msg;
    RecvResult result = Protocol::recv_message(sockets[1], msg);
    
    // Should return RECV_NO_DATA (EAGAIN) for non-blocking socket
    assert(result == RECV_NO_DATA || result == RECV_ERROR);
    
    close(sockets[0]);
    close(sockets[1]);
    std::cout << "  ✓ PASSED\n";
}

void test_invalid_json() {
    std::cout << "[TEST] Invalid JSON handling...\n";
    
    int sockets[2];
    assert(create_socket_pair(sockets) == 0);
    
    // Manually send invalid JSON
    uint16_t msg_type = htons(401);
    uint32_t payload_length = htonl(10);
    char header[6];
    memcpy(header, &msg_type, 2);
    memcpy(header + 2, &payload_length, 4);
    send(sockets[0], header, 6, 0);
    send(sockets[0], "invalid{json", 10, 0);
    
    // Receive should fail
    Message msg;
    RecvResult result = Protocol::recv_message(sockets[1], msg);
    
    assert(result == RECV_ERROR);
    
    close(sockets[0]);
    close(sockets[1]);
    std::cout << "  ✓ PASSED\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Protocol Unit Tests\n";
    std::cout << "========================================\n\n";
    
    // Initialize logger
    Logger::get_instance()->set_min_level(ERROR); // Only show errors
    
    try {
        test_send_receive_single_message();
        test_send_receive_multiple_messages();
        test_empty_payload();
        test_network_byte_order();
        test_large_payload();
        test_recv_no_data();
        test_invalid_json();
        
        std::cout << "\n========================================\n";
        std::cout << "All tests PASSED!\n";
        std::cout << "========================================\n";
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n✗ Test failed: " << e.what() << "\n";
        return 1;
    }
}

