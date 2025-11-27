# Hướng Dẫn Test Backend

## Tổng Quan

Test suite đã được tạo để kiểm tra đầy đủ các chức năng backend Phase 1-4:
- ✅ Authentication (Register, Login, Logout, Session)
- ✅ Practice Mode (Request questions, Submit answers)
- ✅ Test Mode Core (Create room, List rooms, Join room)
- ✅ History & Statistics

## Cấu Trúc Test Files

```
tests/
├── test_client.py      # Test client class với tất cả methods
├── test_individual.py  # Test từng chức năng riêng lẻ
├── quick_test.py      # Test nhanh các chức năng cơ bản
├── README.md          # Documentation
└── TEST_GUIDE.md      # File này
```

## Cách Chạy Tests

### Bước 1: Cài Dependencies (nếu chưa có)

```bash
# Cài SQLite3 và OpenSSL development libraries
sudo apt-get install -y sqlite3 libsqlite3-dev libssl-dev

# Cài Python 3 (thường đã có sẵn)
python3 --version
```

### Bước 2: Build Server

```bash
cd server
make
```

**Lưu ý:** Nếu gặp lỗi compile, đảm bảo đã cài đủ dependencies.

### Bước 3: Chạy Server

```bash
cd server
./bin/server
```

Server sẽ chạy trên port 8888 (mặc định).

### Bước 4: Chạy Tests

Mở terminal mới và chạy:

```bash
cd tests

# Test nhanh (recommended)
python3 quick_test.py

# Test đầy đủ
python3 test_client.py

# Test từng chức năng riêng
python3 test_individual.py
```

## Test Cases Chi Tiết

### 1. Authentication Tests

**Register:**
- ✓ Register user mới
- ✓ Register teacher
- ✓ Register duplicate (expect fail)

**Login:**
- ✓ Login với credentials hợp lệ
- ✓ Login với credentials không hợp lệ (expect fail)
- ✓ Session token được tạo và lưu

**Logout:**
- ✓ Logout thành công
- ✓ Session token bị xóa

### 2. Practice Mode Tests

**Practice Request:**
- ✓ Request questions với số lượng
- ✓ Request với filters (topic, difficulty)
- ✓ Nhận questions (không có đáp án đúng)

**Practice Submit:**
- ✓ Submit answers
- ✓ Nhận kết quả (correct_count/total_questions)
- ✓ Lưu vào PracticeHistory

### 3. Test Room Tests

**Create Room:**
- ✓ TEACHER có thể tạo room
- ✓ USER không thể tạo room (expect fail)
- ✓ Room được tạo với đúng thông tin

**List Rooms:**
- ✓ Lấy danh sách tất cả rooms
- ✓ Hiển thị status, num_questions, duration

**Join Room:**
- ✓ Join room khi status = NOT_STARTED
- ✓ Không thể join khi room đã started (expect fail)
- ✓ Broadcast notification đến participants khác

**View Room Results:**
- ✓ Xem kết quả room đã finished
- ✓ Hiển thị ranking

### 4. History Tests

**Get History:**
- ✓ Lấy lịch sử practice
- ✓ Lấy lịch sử test
- ✓ Format đúng theo protocol

## Expected Output

### Quick Test Output:
```
============================================================
QUICK TEST - Basic Functionality
============================================================
✓ Connected to localhost:8888

1. Testing Register...
✓ Registration successful

2. Testing Login...
✓ Login successful
  Session token: abc123def456...
  Role: USER

3. Testing Practice Request...
✓ Received 3 practice questions
  Q1: Thủ đô của Việt Nam là gì?...
  Q2: 2 + 2 = ?...
  Q3: HTML là viết tắt của gì?...

4. Testing List Rooms...
✓ Found 2 rooms
  Room 1: Math Test [NOT_STARTED]
  Room 2: History Test [ONGOING]

5. Testing Get History...
✓ Found 5 history entries

6. Testing Logout...
✓ Logout successful
```

### Full Test Output:
```
============================================================
Online Testing System - Test Suite
============================================================
✓ Connected to localhost:8888

[TEST] Register: testuser1 (USER)
✓ Registration successful

[TEST] Login: testuser1
✓ Login successful
  Session token: ...
  Role: USER

[TEST] Practice Request: 5 questions, topic=all, difficulty=all
✓ Received 5 practice questions

[TEST] Practice Submit: 5 answers
✓ Practice result: 3/5 correct

[TEST] List Rooms
✓ Found 3 rooms

[TEST] Create Room: Test Room 1
✓ Room created: ID=1

[TEST] Join Room: 1
✓ Joined room successfully
  Room: Test Room 1
  Participants: teacher1, student1

============================================================
Test Summary
============================================================
Total tests: 15
Passed: 14
Failed: 1
Success rate: 93.3%
============================================================
```

## Troubleshooting

### Lỗi: Connection Refused
**Nguyên nhân:** Server chưa chạy hoặc sai port
**Giải pháp:**
```bash
# Kiểm tra server có đang chạy không
ps aux | grep server

# Chạy server
cd server && ./bin/server
```

### Lỗi: Import Error
**Nguyên nhân:** Python version hoặc thiếu module
**Giải pháp:**
```bash
# Kiểm tra Python version (cần >= 3.6)
python3 --version

# Test files chỉ dùng standard library, không cần install thêm
```

### Lỗi: Database Error
**Nguyên nhân:** Database chưa được initialize
**Giải pháp:**
```bash
# Server sẽ tự động tạo database khi chạy lần đầu
# Hoặc chạy script setup thủ công:
cd database
sqlite3 ../testing_app.db < schema.sql
sqlite3 ../testing_app.db < init_data.sql
```

### Lỗi: Test Failures
**Nguyên nhân:** Server chưa implement đầy đủ hoặc có bug
**Giải pháp:**
- Kiểm tra server logs: `server.log`
- Kiểm tra server code có compile không
- Chạy từng test riêng để xác định lỗi

## Test Custom Server

Nếu server chạy trên host/port khác:

```bash
# Quick test
python3 quick_test.py <host> <port>

# Full test
python3 test_client.py <host> <port>
```

Ví dụ:
```bash
python3 quick_test.py 192.168.1.100 9999
```

## Notes

1. **Test Data:** Tests sẽ tạo users mới mỗi lần chạy. Nếu chạy nhiều lần, có thể gặp duplicate errors (đây là expected behavior).

2. **Cleanup:** Database không tự động cleanup test data. Nếu muốn reset:
   ```bash
   rm testing_app.db
   # Server sẽ tạo lại khi restart
   ```

3. **Concurrent Tests:** Có thể chạy nhiều test clients cùng lúc để test concurrency.

4. **Debug Mode:** Để xem chi tiết hơn, sửa log level trong server code hoặc xem `server.log`.

## Next Steps

Sau khi test Phase 1-4 thành công, có thể tiếp tục:
- Phase 5: Test Mode Execution (Start test, Change answer, Submit test, Auto-grade)
- Phase 6: Statistics (đầy đủ)
- Phase 7: Web Client

