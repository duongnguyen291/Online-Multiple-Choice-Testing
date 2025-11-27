
1. Bảng Users (Người dùng)
Lưu trữ thông tin tài khoản để đăng nhập và phân quyền.
user_id: (Số nguyên, Khóa chính) - Mã định danh duy nhất cho mỗi người dùng.
username: (Văn bản, Duy nhất) - Tên đăng nhập (ví dụ: "user_a").
hashed_password: (Văn bản) - Mật khẩu đã được mã hóa.
role: (Văn bản) - Vai trò của người dùng (chỉ nhận giá trị 'USER' hoặc 'TEACHER').
created_at: (Thời gian) - Ngày giờ tài khoản được tạo.

2. Bảng Questions (Câu hỏi)
Ngân hàng câu hỏi trung tâm của toàn bộ hệ thống.
question_id: (Số nguyên, Khóa chính) - Mã định danh duy nhất cho mỗi câu hỏi.
content: (Văn bản) - Nội dung của câu hỏi (ví dụ: "Thủ đô của Việt Nam là gì?").
options: (Văn bản/JSON) - Chuỗi JSON chứa các lựa chọn trả lời (ví dụ: {"a": "Hà Nội", "b": "TP. HCM"}).
correct_option: (Văn bản) - Khóa (key) của đáp án đúng (ví dụ: "a").
difficulty: (Văn bản) - Độ khó của câu hỏi (chỉ nhận 'easy', 'medium', 'hard').
topic: (Văn bản) - Chủ đề của câu hỏi (ví dụ: "Địa lý", "Toán học").
created_by: (Khóa ngoại, tham chiếu đến Users.user_id) - Người đã tạo câu hỏi này (có thể là TEACHER).
created_at: (Thời gian) - Ngày giờ câu hỏi được tạo.

3. Bảng Sessions (Phiên đăng nhập)
Quản lý trạng thái đăng nhập của người dùng. Mỗi "session_token" đại diện cho một lần đăng nhập hợp lệ.
session_token: (Văn bản, Khóa chính) - Chuỗi token ngẫu nhiên, duy nhất được gửi kèm trong mỗi gói tin.
user_id: (Khóa ngoại, tham chiếu đến Users.user_id) - Người dùng sở hữu phiên đăng nhập này.
expiry_timestamp: (Thời gian) - Thời điểm phiên này hết hạn.
created_at: (Thời gian) - Ngày giờ phiên được tạo.

4. Bảng PracticeHistory (Lịch sử Luyện tập)
Lưu lại kết quả của mỗi lần người dùng hoàn thành một bài "Practice Mode".
practice_id: (Số nguyên, Khóa chính) - Mã định danh duy nhất cho lần làm bài luyện tập.
user_id: (Khóa ngoại, tham chiếu đến Users.user_id) - Người dùng đã làm bài này.
correct_count: (Số nguyên) - Số câu trả lời đúng.
total_questions: (Số nguyên) - Tổng số câu hỏi trong bài luyện tập đó.
filters_used: (Văn bản/JSON) - Chuỗi JSON lưu lại các tùy chọn (độ khó, chủ đề) mà người dùng đã chọn cho bài này.
score_percentage: phần trăm điểm
completed_at: (Thời gian) - Ngày giờ hoàn thành bài.

5. Bảng TestRooms (Phòng thi)
Lưu thông tin và trạng thái của các phòng thi "Test Mode".
room_id: (Số nguyên, Khóa chính) - Mã định danh duy nhất cho mỗi phòng thi.
name: (Văn bản) - Tên của phòng thi (ví dụ: "Thi cuối kỳ C++").
creator_id: (Khóa ngoại, tham chiếu đến Users.user_id) - Người đã tạo ra phòng thi này.
status: (Văn bản) - Trạng thái hiện tại của phòng (chỉ nhận 'NOT_STARTED', 'ONGOING', 'FINISHED').
num_questions: (Số nguyên) - Số lượng câu hỏi được cấu hình cho phòng.
duration_minutes: (Số nguyên) - Thời gian làm bài (tính bằng phút) được cấu hình.
filters_used: (Văn bản/JSON) - Chuỗi JSON lưu các bộ lọc (chủ đề, độ khó) dùng để chọn câu hỏi cho phòng này.
start_timestamp: (Thời gian) - Mốc thời gian chính xác khi phòng bắt đầu (được cập nhật khi chủ phòng bấm "Start").
end_timestamp: (Thời gian) - Mốc thời gian chính xác khi phòng kết thúc.
question_bank_id (Số nguyên) - ID của ngân hàng đề
created_at: (Thời gian) - Ngày giờ phòng được tạo.

6. Bảng RoomParticipants (Người tham gia phòng)
Bảng liên kết, cho biết người dùng nào đã tham gia vào phòng thi nào.
room_id: (Khóa ngoại, tham chiếu đến TestRooms.room_id) - Phòng thi mà người dùng tham gia.
user_id: (Khóa ngoại, tham chiếu đến Users.user_id) - Người dùng đã tham gia.
status: (Văn bản) - Trạng thái của người tham gia (chỉ nhận 'JOINED' - đang làm bài, 'SUBMITTED' - đã nộp bài sớm).
score: (Số nguyên) - Điểm số cuối cùng của người dùng (ban đầu là NULL, được cập nhật sau khi chấm bài).
time_spent (Số nguyên) - Thời gian người tham gia đã dành trong phòng thi để theo dõi thời gian đã dùng
joined_at: (Thời gian) - Ngày giờ người dùng tham gia phòng.
(Khóa chính bao gồm cả room_id và user_id)

7. Bảng TestRoomQuestions (Đề thi của phòng)
Bảng liên kết, lưu trữ bộ đề thi cố định đã được chọn cho một phòng thi cụ thể khi nó bắt đầu.
room_id: (Khóa ngoại, tham chiếu đến TestRooms.room_id) - Phòng thi sở hữu bộ đề này.
question_id: (Khóa ngoại, tham chiếu đến Questions.question_id) - Câu hỏi được chọn từ ngân hàng đề.
question_order: (Số nguyên) - (Tùy chọn) Thứ tự của câu hỏi trong đề thi (ví dụ: câu 1, câu 2...).
(Khóa chính bao gồm cả room_id và question_id)

8. Bảng UserTestAnswers (Bài làm của Người dùng)
Lưu trữ từng câu trả lời của người dùng trong một phòng thi. Đây là bảng được cập nhật liên tục khi người dùng thay đổi đáp án.
user_id: (Khóa ngoại) - Người dùng đã trả lời.
room_id: (Khóa ngoại) - Phòng thi nơi câu trả lời được thực hiện.
question_id: (Khóa ngoại) - Câu hỏi cụ thể đang được trả lời.
selected_option: (Văn bản) - Lựa chọn mà người dùng đã chọn (ví dụ: "a", "b", "c"...).
last_updated: (Thời gian) - Thời điểm cuối cùng người dùng thay đổi câu trả lời này.
is_correct (boolean) - Lưu luôn trạng thái đúng sai
(Khóa chính bao gồm cả user_id, room_id, và question_id để đảm bảo mỗi người chỉ có một câu trả lời cho một câu hỏi)

-- Bảng Users (Người dùng)
CREATE TABLE Users (
    user_id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(255) UNIQUE NOT NULL,
    hashed_password VARCHAR(255) NOT NULL,
    role VARCHAR(10) CHECK(role IN ('USER', 'TEACHER')) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Bảng Questions (Câu hỏi)
CREATE TABLE Questions (
    question_id INT PRIMARY KEY AUTO_INCREMENT,
    content TEXT NOT NULL,
    options JSON NOT NULL,
    correct_option VARCHAR(1) NOT NULL, -- Key of the correct option ('a', 'b', 'c', 'd')
    difficulty VARCHAR(10) CHECK(difficulty IN ('easy', 'medium', 'hard')) NOT NULL,
    topic VARCHAR(255) NOT NULL,
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (created_by) REFERENCES Users(user_id) -- References teacher/creator
);

-- Bảng Sessions (Phiên đăng nhập)
CREATE TABLE Sessions (
    session_token VARCHAR(255) PRIMARY KEY,
    user_id INT,
    expiry_timestamp TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES Users(user_id)
);

-- Bảng PracticeHistory (Lịch sử Luyện tập)
CREATE TABLE PracticeHistory (
    practice_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT,
    correct_count INT,
    total_questions INT,
    filters_used JSON,
    completed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    score_percentage FLOAT,
    FOREIGN KEY (user_id) REFERENCES Users(user_id)
);

-- Bảng TestRooms (Phòng thi)
CREATE TABLE TestRooms (
    room_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(255) NOT NULL,
    creator_id INT,
    status VARCHAR(20) CHECK(status IN ('NOT_STARTED', 'ONGOING', 'FINISHED')) NOT NULL,
    num_questions INT NOT NULL,
    duration_minutes INT NOT NULL,
    filters_used JSON,
    start_timestamp TIMESTAMP,
    end_timestamp TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    question_bank_id INT,
    FOREIGN KEY (creator_id) REFERENCES Users(user_id)
);

-- Bảng RoomParticipants (Người tham gia phòng thi)
CREATE TABLE RoomParticipants (
    room_id INT,
    user_id INT,
    status VARCHAR(20) CHECK(status IN ('JOINED', 'SUBMITTED')) NOT NULL,
    score INT,
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    time_spent INT,  -- Thời gian tham gia phòng thi (tính bằng giây)
    PRIMARY KEY (room_id, user_id),
    FOREIGN KEY (room_id) REFERENCES TestRooms(room_id),
    FOREIGN KEY (user_id) REFERENCES Users(user_id)
);

-- Bảng TestRoomQuestions (Đề thi của phòng)
CREATE TABLE TestRoomQuestions (
    room_id INT,
    question_id INT,
    question_order INT,
    PRIMARY KEY (room_id, question_id),
    FOREIGN KEY (room_id) REFERENCES TestRooms(room_id),
    FOREIGN KEY (question_id) REFERENCES Questions(question_id)
);

-- Bảng UserTestAnswers (Bài làm của Người dùng)
CREATE TABLE UserTestAnswers (
    user_id INT,
    room_id INT,
    question_id INT,
    selected_option VARCHAR(1),
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_correct BOOLEAN,  -- Đánh dấu câu trả lời đúng/sai
    PRIMARY KEY (user_id, room_id, question_id),
    FOREIGN KEY (user_id) REFERENCES Users(user_id),
    FOREIGN KEY (room_id) REFERENCES TestRooms(room_id),
    FOREIGN KEY (question_id) REFERENCES Questions(question_id)
);





