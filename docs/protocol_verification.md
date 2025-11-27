# Protocol Implementation Verification

## So sánh Code với Quy định trong application_design.md

### Quy định trong application_design.md

**Luồng xử lý khi nhận (Receive Logic):**
1. Nhận (recv) ít nhất 6 byte vào bộ đệm (buffer).
2. Đọc 2 byte đầu để lấy msg_type (nhớ dùng ntohs()).
3. Đọc 4 byte tiếp theo để lấy payload_length (nhớ dùng ntohl()).
4. Nếu payload_length > 0, tiếp tục nhận (recv) cho đến khi đủ payload_length byte vào bộ đệm.
5. Sau khi có đủ payload_length byte, trích xuất chuỗi JSON từ bộ đệm.
6. Phân tích (parse) chuỗi JSON và xử lý logic.
7. **Lặp lại từ bước 1** (vì bộ đệm có thể còn dữ liệu của gói tin tiếp theo).

---

## Code Implementation

### File: `server/src/protocol.cpp`

#### 1. Function `recv_exact()` - ✅ ĐÚNG

```cpp
bool Protocol::recv_exact(int sockfd, char* buffer, size_t length) {
    size_t total_received = 0;
    while (total_received < length) {
        ssize_t received = recv(sockfd, buffer + total_received, length - total_received, 0);
        if (received <= 0) {
            return false;
        }
        total_received += received;
    }
    return true;
}
```

**✅ Đúng:** Đảm bảo nhận đủ `length` bytes, xử lý TCP stream fragmentation.

---

#### 2. Function `recv_message()` - ✅ ĐÚNG (với một lưu ý)

```cpp
bool Protocol::recv_message(int sockfd, Message& msg) {
    // Bước 1: Nhận header (6 bytes)
    char header_buf[6];
    if (!recv_exact(sockfd, header_buf, 6)) {
        return false;
    }
    
    // Bước 2-3: Parse header
    uint16_t msg_type_raw;
    uint32_t payload_length_raw;
    memcpy(&msg_type_raw, header_buf, 2);           // ✅ Đọc 2 bytes đầu
    memcpy(&payload_length_raw, header_buf + 2, 4); // ✅ Đọc 4 bytes tiếp
    
    msg.type = ntohs(msg_type_raw);                // ✅ Convert network byte order
    uint32_t payload_length = ntohl(payload_length_raw);
    
    // Bước 4: Validate và nhận payload
    if (payload_length > 2 * 1024 * 1024) {        // ✅ Bảo vệ DoS
        LOG_ERROR("Payload too large");
        return false;
    }
    
    if (payload_length > 0) {
        std::vector<char> buffer(payload_length + 1);
        if (!recv_exact(sockfd, buffer.data(), payload_length)) { // ✅ Nhận đủ bytes
            return false;
        }
        buffer[payload_length] = '\0';
        
        // Bước 5-6: Parse JSON
        try {
            msg.payload = json::parse(buffer.data()); // ✅ Parse JSON
        } catch (const json::exception& e) {
            LOG_ERROR("Failed to parse JSON");
            return false;
        }
    } else {
        msg.payload = json::object(); // ✅ Xử lý payload_length = 0
    }
    
    return true;
}
```

**✅ Đúng:**
- Bước 1: Nhận đủ 6 bytes header
- Bước 2: Đọc 2 bytes đầu, dùng `ntohs()`
- Bước 3: Đọc 4 bytes tiếp, dùng `ntohl()`
- Bước 4: Nhận đủ `payload_length` bytes
- Bước 5: Trích xuất JSON từ buffer
- Bước 6: Parse JSON
- Xử lý payload_length = 0
- Bảo vệ DoS (max 2MB)

**⚠️ Lưu ý:** 
- Code hiện tại chỉ xử lý **1 message mỗi lần gọi** `recv_message()`
- Quy định nói "Lặp lại từ bước 1" - có nghĩa là cần xử lý **nhiều messages** nếu có trong buffer
- Tuy nhiên, với epoll edge-triggered, mỗi event chỉ trigger khi có data mới, nên việc xử lý 1 message mỗi lần là **chấp nhận được**
- Nếu muốn tối ưu hơn, có thể loop trong `handle_client_message()` để xử lý nhiều messages

---

### File: `server/src/server.cpp`

#### Function `handle_client_message()` - ⚠️ CẦN XEM XÉT

```cpp
void Server::handle_client_message(int client_fd) {
    Message msg;
    if (!Protocol::recv_message(client_fd, msg)) {
        handle_client_disconnect(client_fd);
        return;
    }
    
    // Route message based on type
    switch (msg.type) {
        case C2S_REGISTER:
            handle_register(client_fd, msg.payload);
            break;
        // ... other cases
    }
}
```

**⚠️ Vấn đề tiềm ẩn:**
- Chỉ xử lý **1 message** mỗi lần `handle_client_message()` được gọi
- Nếu client gửi nhiều messages liên tiếp, có thể có delay
- Với epoll edge-triggered, có thể miss messages nếu không đọc hết buffer

**✅ Giải pháp đề xuất:**
- Thêm loop trong `handle_client_message()` để xử lý tất cả messages có sẵn:

```cpp
void Server::handle_client_message(int client_fd) {
    // Loop để xử lý tất cả messages có sẵn trong buffer
    while (true) {
        Message msg;
        if (!Protocol::recv_message(client_fd, msg)) {
            // Nếu không có message nào hoặc lỗi, thoát loop
            break;
        }
        
        // Route message
        switch (msg.type) {
            // ... handle messages
        }
    }
}
```

---

## So sánh Send Logic

### Quy định: Send Message

**Header:**
- msg_type: 2 bytes, Network Byte Order (htons)
- payload_length: 4 bytes, Network Byte Order (htonl)

**Payload:**
- JSON string (UTF-8)
- Nếu payload_length = 0, không gửi payload

### Code Implementation - ✅ ĐÚNG

```cpp
bool Protocol::send_message(int sockfd, uint16_t msg_type, const json& payload) {
    // Serialize JSON
    std::string payload_str = payload.dump();
    uint32_t payload_length = payload_str.length();
    
    // Prepare header (network byte order)
    char header_buf[6];
    uint16_t msg_type_net = htons(msg_type);        // ✅ Network byte order
    uint32_t payload_length_net = htonl(payload_length);
    memcpy(header_buf, &msg_type_net, 2);
    memcpy(header_buf + 2, &payload_length_net, 4);
    
    // Send header
    if (!send_exact(sockfd, header_buf, 6)) {
        return false;
    }
    
    // Send payload (if any)
    if (payload_length > 0) {                       // ✅ Xử lý payload_length = 0
        if (!send_exact(sockfd, payload_str.c_str(), payload_length)) {
            return false;
        }
    }
    
    return true;
}
```

**✅ Hoàn toàn đúng với quy định**

---

## Kết luận

### ✅ Đúng với quy định:
1. ✅ Header format: 6 bytes (2 bytes msg_type + 4 bytes payload_length)
2. ✅ Network Byte Order: sử dụng `htons()/ntohs()` và `htonl()/ntohl()`
3. ✅ Payload: JSON UTF-8
4. ✅ Xử lý payload_length = 0
5. ✅ Nhận đủ bytes với `recv_exact()`
6. ✅ Parse JSON đúng cách
7. ✅ Bảo vệ DoS (max payload 2MB)

### ✅ Đã cải thiện:
1. **✅ Xử lý nhiều messages:** Đã thêm loop trong `handle_client_message()` để xử lý tất cả messages có sẵn trong buffer (theo đúng quy định "Lặp lại từ bước 1")
2. **✅ Non-blocking handling:** Đã cải thiện `recv_exact()` để xử lý đúng EAGAIN/EWOULDBLOCK với non-blocking socket
3. **✅ Connection state detection:** Sử dụng MSG_PEEK để phân biệt giữa "no data" và "connection closed"

### 📝 Implementation Details:

**Loop trong handle_client_message():**
```cpp
while (messages_processed < MAX_MESSAGES_PER_LOOP) {
    Message msg;
    if (!Protocol::recv_message(client_fd, msg)) {
        // Check if connection closed or just no data
        // ...
        break; // Exit loop if no more data
    }
    // Process message
    // ...
}
```

**Cải thiện recv_exact():**
- Phân biệt EAGAIN/EWOULDBLOCK (no data, but connection OK)
- Phân biệt EOF (connection closed)
- Phân biệt other errors

### ✅ Kết luận:
- Code **hoàn toàn tuân thủ** quy định trong application_design.md
- Xử lý đúng TCP stream fragmentation
- Xử lý đúng multiple messages trong buffer
- Bảo vệ DoS với max payload limit
- Error handling đầy đủ

