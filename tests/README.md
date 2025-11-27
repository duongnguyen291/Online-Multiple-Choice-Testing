# Test Suite Documentation

## Overview

Test suite để kiểm tra tất cả các chức năng đã implement trong Phase 1-4:
- Authentication (Register, Login, Logout)
- Practice Mode (Request, Submit)
- Test Mode Core (Create Room, List Rooms, Join Room)
- History & Statistics

## Requirements

- Python 3.6+
- Server đang chạy trên port 8888 (mặc định)

## Test Files

### 1. `test_client.py`
Test client class với tất cả các methods để test từng chức năng.

**Usage:**
```bash
# Run all tests
python3 test_client.py

# Connect to custom server
python3 test_client.py localhost 8888
```

### 2. `test_individual.py`
Test từng chức năng riêng lẻ với assertions.

**Usage:**
```bash
python3 test_individual.py
```

## Test Cases

### Authentication Tests
- ✓ Register new user
- ✓ Register duplicate (should fail)
- ✓ Login with valid credentials
- ✓ Login with invalid credentials (should fail)
- ✓ Logout

### Practice Mode Tests
- ✓ Request practice questions
- ✓ Request with filters (topic, difficulty)
- ✓ Submit practice answers
- ✓ Receive practice results

### Test Room Tests
- ✓ Create room (TEACHER only)
- ✓ List all rooms
- ✓ Join room
- ✓ User cannot create room
- ✓ View room results

### History Tests
- ✓ Get user history (practice + test)

## Running Tests

### Step 1: Start Server
```bash
cd server
make
./bin/server
```

### Step 2: Run Tests
```bash
cd tests
python3 test_client.py
```

Hoặc test từng suite:
```bash
python3 test_individual.py
```

## Expected Output

```
============================================================
Online Testing System - Test Suite
============================================================
✓ Connected to localhost:8888

[TEST] Register: testuser1 (USER)
✓ Registration successful

[TEST] Login: testuser1
✓ Login successful
  Session token: abc123...
  Role: USER

[TEST] Practice Request: 5 questions, topic=all, difficulty=all
✓ Received 5 practice questions
  Q1: Thủ đô của Việt Nam là gì?...
  ...

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

### Connection Refused
- Đảm bảo server đang chạy: `./bin/server`
- Kiểm tra port: mặc định là 8888

### Import Errors
- Đảm bảo đang ở trong thư mục `tests/`
- Python version >= 3.6

### Test Failures
- Kiểm tra server logs: `server.log`
- Đảm bảo database đã được initialize
- Kiểm tra sample data đã được insert

## Manual Testing

Bạn cũng có thể test thủ công bằng cách sử dụng `test_client.py` như một module:

```python
from test_client import TestClient

client = TestClient()
client.connect()
client.test_register("myuser", "mypass", "USER")
client.test_login("myuser", "mypass")
# ... test other functions
client.disconnect()
```

## Notes

- Tests sẽ tạo users mới mỗi lần chạy (có thể bị duplicate nếu chạy nhiều lần)
- Một số tests cần TEACHER role để pass
- History tests có thể empty nếu chưa có data

