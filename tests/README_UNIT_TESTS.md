# Unit Tests Documentation

## Overview

Unit tests để verify implementation đúng với quy định trong `application_design.md`, đặc biệt là phần "Lặp lại từ bước 1" (xử lý nhiều messages).

## Test Files

### 1. `test_protocol_multiple_messages.py` (Python Integration Test)

**Mục đích:** Test server xử lý nhiều messages trong buffer theo quy định.

**Test Cases:**
- ✅ Multiple messages handling: Gửi 3 messages liên tiếp, verify server xử lý tất cả
- ✅ Burst messages: Gửi 10 messages cùng lúc, verify tất cả được xử lý
- ✅ Mixed message types: Gửi nhiều loại messages khác nhau, verify routing đúng

**Usage:**
```bash
cd tests
python3 test_protocol_multiple_messages.py
```

### 2. `test_protocol_unit.cpp` (C++ Unit Test)

**Mục đích:** Unit test cho protocol parsing và multiple messages handling.

**Test Cases:**
- ✅ Single message send/receive
- ✅ Multiple messages send/receive (loop)
- ✅ Empty payload (payload_length = 0)
- ✅ Network byte order conversion
- ✅ Large payload (near 2MB limit)
- ✅ RECV_NO_DATA when no data available
- ✅ Invalid JSON handling

**Build & Run:**
```bash
cd tests
make test
# Hoặc
make
./bin/test_protocol_unit
```

## Implementation Details

### Loop trong handle_client_message() - Đúng với Design

**Quy định (application_design.md):**
> Lặp lại từ bước 1 (vì bộ đệm có thể còn dữ liệu của gói tin tiếp theo)

**Implementation:**

```cpp
void Server::handle_client_message(int client_fd) {
    // Bước 7: Lặp lại từ bước 1
    while (messages_processed < MAX_MESSAGES_PER_LOOP) {
        Message msg;
        RecvResult result = Protocol::recv_message(client_fd, msg);
        
        if (result == RECV_NO_DATA) {
            // Không còn data trong buffer (EAGAIN) - dừng loop
            break;
        } else if (result == RECV_ERROR) {
            // Lỗi hoặc connection closed
            handle_client_disconnect(client_fd);
            return;
        } else if (result == RECV_SUCCESS) {
            // Xử lý message
            switch (msg.type) {
                // ... route message
            }
            // Tiếp tục loop để xử lý message tiếp theo (nếu có)
        }
    }
}
```

**Điểm chính:**
1. ✅ Loop để xử lý tất cả messages có sẵn
2. ✅ Dừng khi `RECV_NO_DATA` (EAGAIN) - không còn data
3. ✅ Disconnect khi `RECV_ERROR` - lỗi hoặc connection closed
4. ✅ Xử lý từng message và tiếp tục loop

### RecvResult Enum

```cpp
enum RecvResult {
    RECV_SUCCESS,      // Message received successfully
    RECV_NO_DATA,      // No data available (EAGAIN), connection OK
    RECV_ERROR,        // Error or connection closed
    RECV_INCOMPLETE    // Partial data (should not happen)
};
```

**Lợi ích:**
- Phân biệt rõ các trường hợp
- Xử lý đúng với non-blocking socket
- Tuân thủ quy định "Lặp lại từ bước 1"

## Test Results Expected

### Python Integration Tests

```
============================================================
MULTIPLE MESSAGES HANDLING TEST SUITE
============================================================

[TEST] Multiple Messages Handling
  ✓ Received 3 responses
  ✓ Multiple messages handling test completed

[TEST] Burst Messages
  ✓ Received 10/10 responses
  ✓ All burst messages handled correctly

[TEST] Mixed Message Types
  ✓ Received 3/3 responses
  ✓ Mixed message types handled correctly

============================================================
TEST SUMMARY
============================================================
Multiple Messages Handling    : ✓ PASSED
Burst Messages                : ✓ PASSED
Mixed Message Types           : ✓ PASSED

Total: 3/3 tests passed
============================================================
```

### C++ Unit Tests

```
========================================
Protocol Unit Tests
========================================

[TEST] Single message send/receive...
  ✓ PASSED

[TEST] Multiple messages send/receive (loop)...
  ✓ PASSED (received 3 messages)

[TEST] Message with empty payload...
  ✓ PASSED

[TEST] Network byte order conversion...
  ✓ PASSED

[TEST] Large payload (near limit)...
  ✓ PASSED

[TEST] RECV_NO_DATA when no data available...
  ✓ PASSED

[TEST] Invalid JSON handling...
  ✓ PASSED

========================================
All tests PASSED!
========================================
```

## Running All Tests

```bash
# Terminal 1: Start server
cd server
./bin/server

# Terminal 2: Run Python integration tests
cd tests
python3 test_protocol_multiple_messages.py

# Terminal 3: Run C++ unit tests
cd tests
make test
```

## Notes

1. **Non-blocking socket:** Server sử dụng non-blocking socket với epoll edge-triggered
2. **EAGAIN handling:** Khi `recv()` trả về EAGAIN, có nghĩa là không còn data trong buffer, đây là trường hợp bình thường
3. **Loop termination:** Loop dừng khi `RECV_NO_DATA` (EAGAIN), không phải khi có lỗi
4. **Performance:** Xử lý nhiều messages trong một loop giúp tăng performance, giảm số lần gọi epoll

## Verification

Code implementation **HOÀN TOÀN TUÂN THỦ** quy định trong `application_design.md`:
- ✅ Đúng 7 bước receive logic
- ✅ Loop để xử lý nhiều messages (bước 7)
- ✅ Xử lý đúng TCP stream fragmentation
- ✅ Xử lý đúng non-blocking socket với EAGAIN

