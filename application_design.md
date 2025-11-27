Đặc tả Giao thức Tầng Ứng dụng: Hệ thống Thi trắc nghiệm Online
Tài liệu này định nghĩa giao thức giao tiếp (Layer 7) giữa Client và Server qua kết nối TCP.
1. Tổng quan
Vận chuyển (Transport): TCP (Đáng tin cậy, có trạng thái).
Định dạng dữ liệu (Payload Format): JSON (UTF-8).
Giải quyết vấn đề luồng (Stream Handling): Sử dụng định dạng gói tin [Type][Length][Value] để phân tách các thông điệp trên luồng TCP.
2. Định dạng Gói tin (Packet Format)
Mỗi thông điệp (message) được truyền đi, bất kể từ Client hay Server, đều phải tuân theo cấu trúc sau:
[ HEADER ] [ PAYLOAD ]
<-- 6 bytes --><-- N bytes -->

Trong đó:
HEADER (6 bytes):
msg_type (2 bytes, uint16_t): Mã lệnh (opcode) xác định loại thông điệp. Phải được gửi theo thứ tự Network Byte Order (dùng htons(), ntohs()).
payload_length (4 bytes, uint32_t): Kích thước (số byte) của phần Payload. Phải được gửi theo thứ tự Network Byte Order (dùng htonl(), ntohl()).
PAYLOAD (N bytes):
payload (char[N]): Một chuỗi JSON (UTF-8) chứa dữ liệu chi tiết của thông điệp. N chính là giá trị của payload_length.
Nếu không có dữ liệu cần gửi, payload_length sẽ là 0 và phần payload sẽ rỗng.
Luồng xử lý khi nhận (Receive Logic):
Nhận (recv) ít nhất 6 byte vào bộ đệm (buffer).
Đọc 2 byte đầu để lấy msg_type (nhớ dùng ntohs()).
Đọc 4 byte tiếp theo để lấy payload_length (nhớ dùng ntohl()).
Nếu payload_length > 0, tiếp tục nhận (recv) cho đến khi đủ payload_length byte vào bộ đệm.
Sau khi có đủ payload_length byte, trích xuất chuỗi JSON từ bộ đệm.
Phân tích (parse) chuỗi JSON và xử lý logic.
Lặp lại từ bước 1 (vì bộ đệm có thể còn dữ liệu của gói tin tiếp theo).
3. Cấu trúc Thông điệp (Message Types & Payload)
Sử dụng JSON cho payload. Các thông điệp luôn có 2 loại:
Request (C2S): Client gửi yêu cầu đến Server.
Response (S2C): Server phản hồi Client. Có 2 dạng:
Direct Response: Phản hồi trực tiếp cho một Request (ví dụ: S2C_RESPONSE_OK).
Push Notification: Tin nhắn chủ động từ Server (ví dụ: S2C_TEST_STARTED).
3.1. Thông điệp Chung & Lỗi
S2C_RESPONSE_OK (Mã: 801)
Hướng: Server -> Client
Mô tả: Phản hồi thành công chung cho các hành động không cần trả về dữ liệu (như Đăng ký, Tạo phòng, Nộp bài...).
Payload:
{
  "message": "Đăng ký thành công"
}

{ "message": "Registration successful" }

{ "code": 1001, "message": "Username already exists" }

1001: Login failed
1002: Username exists
2001: Room not found
2002: Room already started/finished
2003: Not room owner
9999: System error




S2C_RESPONSE_ERROR (Mã: 802)
Hướng: Server -> Client
Mô tả: Phản hồi khi có lỗi xảy ra.
Payload:
{
  "code": <mã_lỗi_nội_bộ>, // (ví dụ: 1001)
  "message": "Tên đăng nhập đã tồn tại"
}

Mã lỗi nội bộ (ví dụ):
1001: Đăng nhập thất bại
1002: Tên đăng nhập tồn tại
2001: Phòng không tồn tại
2002: Phòng đã bắt đầu / kết thúc
2003: Bạn không phải chủ phòng
9999: Lỗi hệ thống
3.2. Luồng Xác thực (Authentication)
C2S_REGISTER (Mã: 101)
Hướng: Client -> Server
Mô tả: Yêu cầu đăng ký tài khoản mới.
Payload:
{
  "username": "user_a",
  "password": "hashed_password_abc", // Client NÊN hash trước khi gửi
  "role": "USER" // (Hoặc "TEACHER" nếu có phân quyền)
}

Phản hồi: S2C_RESPONSE_OK hoặc S2C_RESPONSE_ERROR.
C2S_LOGIN (Mã: 102)
Hướng: Client -> Server
Mô tả: Yêu cầu đăng nhập.
Payload:
{
  "username": "user_a",
  "password": "hashed_password_abc"
}

S2C_LOGIN_OK (Mã: 803) (Thay thế S2C_RESPONSE_OK cho Login)
Hướng: Server -> Client
Mô tả: Đăng nhập thành công, trả về session token.
Payload:
{
  "session_token": "a1b2c3d4e5f6_random_session_token",
  "username": "user_a",
  "role": "USER"
}

Tất cả các Request sau khi đăng nhập (trừ C2S_REGISTER) BẮT BUỘC phải gửi kèm session_token trong payload.
C2S_LOGOUT (Mã: 103)
Hướng: Client -> Server
Mô tả: Yêu cầu đăng xuất.
Payload:
{
  "session_token": "..."
}

Phản hồi: S2C_RESPONSE_OK
3.3. Luồng Luyện tập (Practice Mode)
C2S_PRACTICE_REQUEST (Mã: 201)
Hướng: Client -> Server
Mô tả: Yêu cầu bắt đầu một bài luyện tập.
Payload:
{
  "session_token": "...",
  "num_questions": 20,
  "topic": "all", // hoặc "math", "history"
  "difficulty": "all" // hoặc "easy", "medium", "hard"
}

S2C_PRACTICE_QUESTIONS (Mã: 901)
Hướng: Server -> Client
Mô tả: Gửi bộ câu hỏi luyện tập.
Payload:
{
  "duration_seconds": 1800, // (ví dụ 30 phút)
  "questions": [
    {
      "q_id": 1,
      "content": "Thủ đô của Việt Nam là gì?",
      "option_a": "Hà Nội",
      "option_b": "TP. HCM",
      "option_c": "Đà Nẵng",
      "option_d": "Hải Phòng"
    },
    // ... (các câu hỏi khác, KHÔNG gửi đáp án đúng)
  ]
}

C2S_PRACTICE_SUBMIT (Mã: 202)
Hướng: Client -> Server
Mô tả: Nộp bài luyện tập.
Payload:
{
  "session_token": "...",
  "answers": [
    { "q_id": 1, "selected_option": "option_a" },
    { "q_id": 2, "selected_option": "option_c" }
    // ...
  ]
}

S2C_PRACTICE_RESULT (Mã: 902)
Hướng: Server -> Client
Mô tả: Trả kết quả bài luyện tập.
Payload:
{
  "correct_count": 18,
  "total_questions": 20
}

3.4. Luồng Phòng thi (Test Mode)
C2S_LIST_ROOMS (Mã: 301)
Hướng: Client -> Server
Mô tả: Yêu cầu danh sách các phòng thi.
Payload: { "session_token": "..." }
S2C_ROOM_LIST (Mã: 1001)
Hướng: Server -> Client
Mô tả: Gửi danh sách phòng thi.
Payload:
{
  "rooms": [
    {
      "room_id": 101,
      "name": "Thi giữa kỳ Mạng Máy Tính",
      "status": "NOT_STARTED", // (NOT_STARTED, ONGOING, FINISHED)
      "num_questions": 40,
      "duration_minutes": 60
    },
    // ... (các phòng khác)
  ]
}

C2S_CREATE_ROOM (Mã: 302)
Hướng: Client -> Server
Mô tả: (Teacher/Admin) Yêu cầu tạo phòng thi mới.
Payload:
{
  "session_token": "...",
  "name": "Thi cuối kỳ C++",
  "num_questions": 50,
  "duration_minutes": 90,
  "topic": "all",
  "difficulty": "medium"
}

S2C_ROOM_CREATED (Mã: 1002)
Hướng: Server -> Client
Mô tả: Phản hồi tạo phòng thành công.
Payload:
{
  "room_id": 102,
  "message": "Tạo phòng thành công"
}

C2S_JOIN_ROOM (Mã: 303)
Hướng: Client -> Server
Mô tả: Yêu cầu tham gia một phòng thi.
Payload: { "session_token": "...", "room_id": 102 }
S2C_JOIN_OK (Mã: 1003)
Hướng: Server -> Client
Mô tả: Phản hồi tham gia phòng thành công (vào phòng chờ).
Payload:
{
  "room_id": 102,
  "room_name": "Thi cuối kỳ C++",
  "participants": [ "user_a", "user_b" ] // (những người đã ở trong phòng)
}

Lỗi: S2C_RESPONSE_ERROR (nếu phòng đã bắt đầu hoặc không tồn tại).
S2C_USER_JOINED_ROOM (Mã: 1004) - [PUSH]
Hướng: Server -> All Clients in Room
Mô tả: Thông báo (PUSH) cho mọi người trong phòng khi có user mới tham gia.
Payload: { "username": "user_c" }
C2S_START_TEST (Mã: 401)
Hướng: Client -> Server
Mô tả: (Chủ phòng) Yêu cầu bắt đầu bài thi.
Payload: { "session_token": "...", "room_id": 102 }
Phản hồi: S2C_RESPONSE_ERROR (nếu không phải chủ phòng). (Không cần S2C_RESPONSE_OK, vì S2C_TEST_STARTED chính là OK).
S2C_TEST_STARTED (Mã: 1101) - [PUSH]
Hướng: Server -> All Clients in Room
Mô tả: Thông báo (PUSH) bài thi bắt đầu, gửi kèm bộ đề.
Payload: (Giống hệt S2C_PRACTICE_QUESTIONS)
{
  "room_id": 102,
  "end_timestamp": 1678886400, // (Thời điểm kết thúc tuyệt đối, dạng Unix timestamp)
  "questions": [
    // ... (bộ câu hỏi của phòng thi)
  ]
}

S2C_ROOM_STATUS_CHANGED (Mã: 1005) - [PUSH]
Hướng: Server -> All Clients (in Lobby)
Mô tả: Thông báo (PUSH) cho những người ở sảnh (chưa vào phòng) rằng trạng thái 1 phòng đã đổi (ví dụ: ONGOING).
Payload: { "room_id": 102, "new_status": "ONGOING" }
C2S_CHANGE_ANSWER (Mã: 402)
Hướng: Client -> Server
Mô tả: Cập nhật đáp án (server lưu tạm).
Payload:
{
  "session_token": "...",
  "room_id": 102,
  "q_id": 5,
  "selected_option": "option_d"
}

Phản hồi: Không cần (để giảm tải mạng). Client tự tin tưởng là đã gửi.
C2S_SUBMIT_TEST (Mã: 403)
Hướng: Client -> Server
Mô tả: Yêu cầu nộp bài sớm.
Payload:
{
  "session_token": "...",
  "room_id": 102,
  "answers": [
    // ... (toàn bộ đáp án cuối cùng)
  ]
}

Phản hồi: S2C_RESPONSE_OK (chỉ xác nhận đã nộp, chưa trả kết quả). Client sẽ vào trạng thái "chờ những người khác".
S2C_TEST_ENDED (Mã: 1102) - [PUSH]
Hướng: Server -> All Clients in Room
Mô tả: Thông báo (PUSH) khi hết giờ, hoặc khi mọi người đã nộp xong.
Payload: { "room_id": 102, "message": "Bài thi đã kết thúc. Đang chấm điểm..." }
S2C_YOUR_RESULT (Mã: 1103) - [PUSH]
Hướng: Server -> Client (Gửi riêng lẻ)
Mô tả: Gửi (PUSH) kết quả cá nhân sau khi S2C_TEST_ENDED.
Payload:
{
  "room_id": 102,
  "correct_count": 45,
  "total_questions": 50,
  "rank": 3 // (Tùy chọn)
}

3.5. Luồng Lịch sử & Thống kê
C2S_GET_HISTORY (Mã: 501)
Hướng: Client -> Server
Mô tả: Yêu cầu xem lịch sử các bài đã làm.
Payload: { "session_token": "..." }
S2C_HISTORY_DATA (Mã: 1201)
Hướng: Server -> Client
Mô tả: Gửi dữ liệu lịch sử.
Payload:
{
  "history": [
    {
      "mode": "PRACTICE",
      "date": "2025-10-23",
      "score": "18/20"
    },
    {
      "mode": "TEST",
      "room_name": "Thi cuối kỳ C++",
      "date": "2025-10-24",
      "score": "45/50"
    }
  ]
}

C2S_GET_STATS (Mã: 502)
Hướng: Client -> Server
Mô tả: Yêu cầu dữ liệu thống kê (cho đồ thị).
Payload: { "session_token": "..." }
S2C_STATS_DATA (Mã: 1202)
Hướng: Server -> Client
Mô tả: Gửi dữ liệu thống kê.
Payload:
{
  "score_over_time": [
    { "date": "2025-10-20", "score_percent": 80 },
    { "date": "2025-10-23", "score_percent": 90 },
    { "date": "2025-10-24", "score_percent": 90 }
  ],
  "topic_distribution": [
    { "topic": "math", "correct_percent": 85 },
    { "topic": "history", "correct_percent": 95 }
  ]
}

C2S_VIEW_ROOM_RESULTS (Mã: 503)
Hướng: Client -> Server
Mô tả: Yêu cầu xem kết quả chi tiết của một phòng đã kết thúc.
Payload: { "session_token": "...", "room_id": 102 }
S2C_ROOM_RESULTS_DATA (Mã: 1203)
Hướng: Server -> Client
Mô tả: Gửi bảng điểm của phòng đã kết thúc.
Payload:
{
  "room_id": 102,
  "room_name": "Thi cuối kỳ C++",
  "results": [
    { "username": "user_b", "score": "48/50" },
    { "username": "user_c", "score": "47/50" },
    { "username": "user_a", "score": "45/50" }
  ]
}

Mã Quy Định
Tên Thông Điệp
Hướng
Mô Tả
Payload Ví Dụ
801
S2C_RESPONSE_OK
Server -> Client
Phản hồi thành công chung cho các hành động không cần trả về dữ liệu.
{ "message": "Đăng ký thành công" }
802
S2C_RESPONSE_ERROR
Server -> Client
Phản hồi lỗi khi có sự cố xảy ra.
{ "code": 1001, "message": "Tên đăng nhập đã tồn tại" }
101
C2S_REGISTER
Client -> Server
Yêu cầu đăng ký tài khoản mới.
{ "username": "user_a", "password": "hashed_password_abc", "role": "USER" }
102
C2S_LOGIN
Client -> Server
Yêu cầu đăng nhập.
{ "username": "user_a", "password": "hashed_password_abc" }
803
S2C_LOGIN_OK
Server -> Client
Phản hồi đăng nhập thành công, trả về session token.
{ "session_token": "a1b2c3d4e5f6_random_session_token", "username": "user_a", "role": "USER" }
103
C2S_LOGOUT
Client -> Server
Yêu cầu đăng xuất.
{ "session_token": "..." }
201
C2S_PRACTICE_REQUEST
Client -> Server
Yêu cầu bắt đầu một bài luyện tập.
{ "session_token": "...", "num_questions": 20, "topic": "all", "difficulty": "all" }
901
S2C_PRACTICE_QUESTIONS
Server -> Client
Gửi bộ câu hỏi luyện tập.
{ "duration_seconds": 1800, "questions": [ { "q_id": 1, "content": "Thủ đô của Việt Nam là gì?", "option_a": "Hà Nội" } ] }
202
C2S_PRACTICE_SUBMIT
Client -> Server
Nộp bài luyện tập.
{ "session_token": "...", "answers": [ { "q_id": 1, "selected_option": "option_a" } ] }
902
S2C_PRACTICE_RESULT
Server -> Client
Trả kết quả bài luyện tập.
{ "correct_count": 18, "total_questions": 20 }
301
C2S_LIST_ROOMS
Client -> Server
Yêu cầu danh sách các phòng thi.
{ "session_token": "..." }
1001
S2C_ROOM_LIST
Server -> Client
Gửi danh sách phòng thi.
{ "rooms": [ { "room_id": 101, "name": "Thi giữa kỳ Mạng Máy Tính", "status": "NOT_STARTED", "num_questions": 40 } ] }
302
C2S_CREATE_ROOM
Client -> Server
(Teacher/Admin) Yêu cầu tạo phòng thi mới.
{ "session_token": "...", "name": "Thi cuối kỳ C++", "num_questions": 50, "duration_minutes": 90, "topic": "all", "difficulty": "medium" }
1002
S2C_ROOM_CREATED
Server -> Client
Phản hồi tạo phòng thành công.
{ "room_id": 102, "message": "Tạo phòng thành công" }
303
C2S_JOIN_ROOM
Client -> Server
Yêu cầu tham gia một phòng thi.
{ "session_token": "...", "room_id": 102 }
1003
S2C_JOIN_OK
Server -> Client
Phản hồi tham gia phòng thành công (vào phòng chờ).
{ "room_id": 102, "room_name": "Thi cuối kỳ C++", "participants": [ "user_a", "user_b" ] }
1004
S2C_USER_JOINED_ROOM
Server -> Client
Thông báo (PUSH) cho mọi người trong phòng khi có user mới tham gia.
{ "username": "user_c" }
401
C2S_START_TEST
Client -> Server
(Host) Yêu cầu bắt đầu bài thi.
{ "session_token": "...", "room_id": 102 }
1101
S2C_TEST_STARTED
Server -> Client
Thông báo (PUSH) bài thi bắt đầu, gửi kèm bộ đề.
{ "room_id": 102, "end_timestamp": 1678886400, "questions": [ { "q_id": 1, "content": "Thủ đô của Việt Nam là gì?" } ] }
1005
S2C_ROOM_STATUS_CHANGED
Server -> Client
Thông báo (PUSH) trạng thái phòng thi thay đổi (ONGOING).
{ "room_id": 102, "new_status": "ONGOING" }
402
C2S_CHANGE_ANSWER
Client -> Server
Cập nhật đáp án (server lưu tạm).
{ "session_token": "...", "room_id": 102, "q_id": 5, "selected_option": "option_d" }
403
C2S_SUBMIT_TEST
Client -> Server
Yêu cầu nộp bài sớm.
{ "session_token": "...", "room_id": 102, "answers": [ { "q_id": 1, "selected_option": "option_a" } ] }
1102
S2C_TEST_ENDED
Server -> Client
Thông báo (PUSH) khi hết giờ thi, hoặc khi tất cả đã nộp bài.
{ "room_id": 102, "message": "Bài thi đã kết thúc. Đang chấm điểm..." }
1103
S2C_YOUR_RESULT
Server -> Client
Gửi kết quả cá nhân sau khi bài thi kết thúc.
{ "room_id": 102, "correct_count": 45, "total_questions": 50, "rank": 3 }
501
C2S_GET_HISTORY
Client -> Server
Yêu cầu xem lịch sử các bài thi đã làm.
{ "session_token": "..." }
1201
S2C_HISTORY_DATA
Server -> Client
Gửi dữ liệu lịch sử bài thi.
{ "history": [ { "mode": "PRACTICE", "date": "2025-10-23", "score": "18/20" }, { "mode": "TEST", "date": "2025-10-24", "score": "45/50" } ] }
502
C2S_GET_STATS
Client -> Server
Yêu cầu xem thống kê điểm thi.
{ "session_token": "..." }
1202
S2C_STATS_DATA
Server -> Client
Gửi dữ liệu thống kê cho đồ thị.
{ "score_over_time": [ { "date": "2025-10-20", "score_percent": 80 }, { "date": "2025-10-23", "score_percent": 90 } ], "topic_distribution": [ { "topic": "math", "correct_percent": 85 } ] }
503
C2S_VIEW_ROOM_RESULTS
Client -> Server
Yêu cầu xem kết quả chi tiết của một phòng đã kết thúc.
{ "session_token": "...", "room_id": 102 }
1203
S2C_ROOM_RESULTS_DATA
Server -> Client
Gửi bảng điểm của phòng thi đã kết thúc.
{ "room_id": 102, "room_name": "Thi cuối kỳ C++", "results": [ { "username": "user_b", "score": "48/50" } ] }


4. Trình tự Giao tiếp (Communication Flows)
Flow 1: Đăng nhập & Lấy danh sách phòng
Client --(C2S_LOGIN)--> Server
Server (Kiểm tra DB) --(S2C_LOGIN_OK + token)--> Client
Client (Lưu token) --(C2S_LIST_ROOMS + token)--> Server
Server (Truy vấn DB) --(S2C_ROOM_LIST)--> Client
Client (Hiển thị danh sách phòng)
Flow 2: Tạo và Tham gia phòng
Client A --(C2S_CREATE_ROOM + token)--> Server
Server (Tạo phòng) --(S2C_ROOM_CREATED + room_id)--> Client A
Client B --(C2S_LIST_ROOMS + token)--> Server
Server --(S2C_ROOM_LIST + phòng của A)--> Client B
Client B --(C2S_JOIN_ROOM + room_id)--> Server
Server (Thêm B vào phòng) --(S2C_JOIN_OK)--> Client B
Server --(S2C_USER_JOINED_ROOM + "user_b")--> Client A (PUSH)
Flow 3: Bắt đầu, Làm bài và Kết thúc thi
(Client A, B, C đang ở phòng chờ 102)
Client A (Owner) --(C2S_START_TEST)--> Server
Server (Lấy câu hỏi, đặt timer) --(S2C_TEST_STARTED + questions)--> Client A, B, C (PUSH)
Server --(S2C_ROOM_STATUS_CHANGED + "ONGOING")--> All Clients (Lobby) (PUSH)
(Client B, C làm bài...)
Client B --(C2S_CHANGE_ANSWER)--> Server
Client C --(C2S_CHANGE_ANSWER)--> Server
Client B --(C2S_SUBMIT_TEST)--> Server
Server (Đánh dấu B đã nộp) --(S2C_RESPONSE_OK)--> Client B
(Thời gian đếm ngược của Server hết...)
Server (Tự động chấm bài cho A, C) --(S2C_TEST_ENDED)--> Client A, B, C (PUSH)
Server (Tính toán) --(S2C_YOUR_RESULT)--> Client A (PUSH)
Server (Tính toán) --(S2C_YOUR_RESULT)--> Client B (PUSH)
Server (Tính toán) --(S2C_YOUR_RESULT)--> Client C (PUSH)
Server --(S2C_ROOM_STATUS_CHANGED + "FINISHED")--> All Clients (Lobby) (PUSH)

 Ví dụ: C2S_CREATE_ROOM (Mã: 302) — Client → Server
1) Payload (JSON, UTF-8)
{"session_token":"abc123xyZTOKEN","name":"Thi cuối kỳ C++","num_questions":50,"duration_minutes":90,"topic":"all","difficulty":"medium"}

Lưu ý có ký tự ++ (UTF-8 1 byte mỗi ký tự), nội dung tiếng Việt (“Thi cuối kỳ …”) ở UTF-8.


Độ dài payload (UTF-8): 140 bytes.
2) Header (6 bytes, network byte order)
msg_type = 302 (0x012E) → 2 bytes BE


payload_length = 140 (0x0000008C) → 4 bytes BE


Trường
Giá trị thập phân
Giá trị hex (BE)
msg_type
302
01 2E
payload_length
140
00 00 00 8C

3) Gói tin hoàn chỉnh (Header + Payload)
Tổng chiều dài: 6 + 140 = 146 bytes


Hex dump (rút gọn tiền tố):
01 2E 00 00 00 8C 
7B 22 73 65 73 73 69 6F 6E 5F 74 6F 6B 65 6E 22 3A 22 61 62 63 31 32 33 78 79 5A 54 4F 4B 45 4E 22 2C 
22 6E 61 6D 65 22 3A 22 54 68 69 20 63 75 1ED1 69 20 6B 1EF3 20 43 2B 2B 22 2C 
22 6E 75 6D 5F 71 75 65 73 74 69 6F 6E 73 22 3A 35 30 2C 
22 64 75 72 61 74 69 6F 6E 5F 6D 69 6E 75 74 65 73 22 3A 39 30 2C 
22 74 6F 70 69 63 22 3A 22 61 6C 6C 22 2C 
22 64 69 66 66 69 63 75 6C 74 79 22 3A 22 6D 65 64 69 75 6D 22 7D

(Các byte UTF-8 của chữ có dấu sẽ là chuỗi 2–3 byte; ở trên chỉ minh hoạ dòng đầu, thực tế bạn thấy toàn bộ 146 byte khi debug.)

4) Server nhận & xử lý — theo Length Prefixing
Bước 1: recv_exact(6) → đọc đủ 6 byte header.
Parse:


msg_type = ntohs(0x012E) = 302


payload_length = ntohl(0x0000008C) = 140


Bước 2: vì payload_length > 0 → recv_exact(140) để lấy đủ 140 byte payload UTF-8.
Bước 3: parse JSON:
{
  "session_token":"abc123xyZTOKEN",
  "name":"Thi cuối kỳ C++",
  "num_questions":50,
  "duration_minutes":90,
  "topic":"all",
  "difficulty":"medium"
}

Bước 4: kiểm tra nghiệp vụ:
session_token hợp lệ? → tra bảng Sessions (còn hạn, map đúng user_id).


Quyền: user có vai trò TEACHER (hoặc policy cho phép) để tạo phòng?


Kiểm tra rule cấu hình: num_questions > 0, duration_minutes trong giới hạn, topic/difficulty hợp lệ, thư viện có đủ câu (nếu dùng question_bank_id)…


Tạo bản ghi TestRooms + sinh room_id.


(Tuỳ thiết kế) sinh bộ đề cố định vào TestRoomQuestions ngay lúc tạo, hoặc tạo khi START_TEST.


Bước 5: phản hồi:
Nếu OK → gửi S2C_ROOM_CREATED (Mã: 1002):

 { "room_id": 102, "message": "Tạo phòng thành công" }
 Header:


msg_type = 1002 (0x03EA) → 03 EA


payload_length = len(...) (tính theo UTF-8) → 4 byte BE


Payload = JSON ở trên (UTF-8 minify).


Nếu lỗi → S2C_RESPONSE_ERROR (802) với code chi tiết:

 { "code": 42003, "message": "Not room owner / permission denied" }
 (tách biệt hẳn với msg_type để không xung đột)


5) Vì sao Length Prefixing “ăn đứt” phân tách bằng ký tự?
TCP là byte stream: có thể “dính gói” (nhiều message đổ vào 1 lần recv) hoặc “vỡ gói” (1 message phải recv nhiều lần).


Length Prefixing cho phép bạn biết chính xác còn phải đọc bao nhiêu byte cho message hiện tại, nên không phụ thuộc ký tự phân cách, không sợ “cắt nhầm” khi nội dung là JSON/nhị phân.


Checklist thực chiến khi implement recv_exact(n):
Timeout/Retry: nếu recv trả về 0 (peer đóng), hoặc <0 (lỗi), phải thoát/cleanup.


Giới hạn kích thước tối đa cho payload_length (ví dụ 1–2 MB) để tránh DoS.


Luôn hton*/ntoh* cho endianness.


Content-Length = số byte UTF-8, không phải số ký tự (chữ có dấu = 2–3 byte).


6) Nhìn nhanh byte đầu để debug
Header 6 byte:
 01 2E (302) | 00 00 00 8C (140)
 → Mở Wireshark/tcpdump thấy 01 2E 00 00 00 8C là nhận diện ngay “Client đang gửi C2S_CREATE_ROOM, payload 140 byte”.

