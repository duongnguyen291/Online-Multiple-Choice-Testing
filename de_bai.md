Tên dự án & mục tiêu – Giới thiệu ý tưởng, mục đích chính.

Luật chơi / luồng hoạt động – Cách hệ thống hoạt động (ví dụ: cách hoạt động của phiên đấu giá).

Phân tích & thiết kế hệ thống sơ bộ – UML (Use Case, Sequence, v.v.).

Thiết kế giao thức tầng ứng dụng (Application Layer Protocol) – ⚠️ Bắt buột, quan trọng

- Bạn phải tự định nghĩa cách client – server giao tiếp:

- Định dạng gói tin request/response

- Cấu trúc thông điệp

Trình tự giao tiếp (handshake, command flow, kết thúc kết nối, v.v.)

Thiết kế cơ sở dữ liệu – Nếu hệ thống có DB.

Phân công công việc và timeline – Ai làm gì, kế hoạch thực hiện, kết quả dự kiến.

Tiến độ hiện tại – Tóm tắt phần đã làm được.



Cụ thể như sau:

Build an online multiple-choice testing applicationRequired functionalities:

Test Mode:

Users can create a new test room, set the number of questions, and define the test duration. Users can decide when to start and end the test.

View a list of test rooms and the status of each room (not started, ongoing, finished).

Join a test room if the test has not started yet.

View test results for completed rooms.

When the test begins, the server sends the user the test questions.

Allow users to change their answered questions as long as time permits.

Users can submit their answers early, before the test time ends.

After the test time ends, the server notifies users of the number of correct answers.



Practice Mode:

In this mode, users can take practice tests with a set number of questions and a specific time limit. After completing the practice test, the application shows the number of correct answers.



Other Features:

Classify questions by difficulty and allow users to customize the type of questions included in the test.

Store information about tests that users have taken and display it upon request.

Provide statistics in graphical form about the tests that users have completed.

Graphical User Interface (GUI).



This outlines the main functions for a multiple-choice test application with both test and practice modes, along with additional data visualization and question customization options.Stream handling: 1 point

Implementing socket I/O mechanism on the server: 2 points

Account registration and management: 2 points

Login and session management: 2 points

Access control management: 1 point

Participating in practice mode: 1 point

Creating test rooms: 2 points

Viewing the list of test rooms: 1 point

Joining a test room: 2 points

Starting the test: 1 point

Changing previously selected answers: 1 point

Submitting and scoring the test: 2 points

Viewing test results of completed rooms: 1 point

Logging activities: 1 point

Classifying questions by difficulty, topic: 1-3 points

Storing test information + graphical statistics: 2 points

Graphical User Interface (GUI): 3 points

Advanced features: 1-10 points

This represents the point allocation for the various functionalities of the online testing application, highlighting both core features and potential advanced features.



công nghệ

Linux bắt buộc. Không dùng Windows/Winsock.Sockets C/C++ cho mọi giao tiếp client–server.Kiểu kết nối: TCP (đề xuất, vì có state và tin cậy).I/O đa luồng hoặc đa sự kiện:Server: select()/poll()/epoll hoặc thread-pool (khóa dữ liệu cẩn thận).Lưu trữ: SQLite (C API đơn giản) hoặc file JSON/CSV + caching in-memory.GUI (3 điểm): C++ (Qt) hoặc ncurses nếu muốn CLI “giàu” hơn (vẫn C++).Chức năng

Tài khoản: đăng ký, đăng nhập, quản lý phiên, phân quyền (admin/teacher/user).Test Mode: tạo phòng, xem danh sách & trạng thái (not_started/ongoing/finished), join trước giờ start, start test, nhận bộ câu hỏi, đổi đáp án trong thời gian, nộp sớm, tự động chốt khi hết giờ, chấm điểm và trả kết quả.Practice Mode: đặt số câu & thời gian, làm bài, trả số câu đúng.Phân loại câu hỏi: topic, difficulty; cho phép lọc khi tạo test/practice.Lưu lịch sử: các bài đã làm, xem lại khi yêu cầu.Thống kê: tổng hợp, hiển thị dạng đồ thị ở client (điểm theo thời gian, phân bố độ khó…).Logging: hoạt động quan trọng (auth, tạo phòng, join, start, submit, scoring, lỗi).Truy cập/Phân quyền: chỉ chủ phòng được start/end; user không xem đáp án khi test đang chạy, v.v.Mô tả những gì người dùng sẽ thấy

Danh sách phòng thi (id, tên, trạng thái: not_started/ongoing/finished, số câu, thời lượng).Bộ câu hỏi khi vào bài (N câu, 4 đáp án, có thể đảo trật tự).Đổi đáp án trong thời gian còn lại; đồng hồ đếm ngược.Kết quả: điểm dạng x/N, thời gian nộp, (tuỳ chọn) chi tiết câu đúng/sai sau khi phòng finished.Practice result: x/N sau khi kết thúc.Lịch sử: các bài đã làm (ngày, mode, điểm, phòng).Thống kê: đồ thị điểm theo thời gian, phân bố điểm theo topic/difficulty, số bài đã làm.Thông báo thời gian: start, time-up, kết quả.

output nộp:

Source code chạy trên Linux (C/C++)

01 server đa kết nối (select/poll/epoll) + 01 client (CLI hoặc GUI C++).Makefile cho server/ và client/ (make, make run, make clean).File README.md: yêu cầu hệ thống, cách build/run, lệnh demo nhanh.File protocol.md: đặc tả giao thức tầng ứng dụng (chi tiết ở phần III).(Khuyến nghị) Dockerfile hoặc script run.sh để chạy nhanh.Bộ dữ liệu câu hỏiTài liệu thiết kế

UML: Use Case (đăng ký/đăng nhập, tạo phòng, join, làm bài, nộp bài, xem kết quả), Sequence (đăng nhập, tạo phòng, start test, submit), Activity (làm bài & đổi đáp án), (nếu có DB) ERD/tables.protocol.md: lệnh, thông điệp, mã lỗi, quy tắc timeouts, push events.security.md (ngắn): phiên đăng nhập, token/session, kiểm soát truy cập.test-plan.md: test cases (functional + concurrency + lỗi mạng).log-policy.md: quy ước log, mức log, sự kiện được log.demo & kết quả


Yêu cầu: Xây dựng ứng dụng thi trắc nghiệm trực tuyến
Các chức năng yêu cầu:
1. Chế độ Thi (Test Mode):
Người dùng có thể tạo một phòng thi mới, thiết lập số lượng câu hỏi và định nghĩa thời gian thi. Người dùng có thể quyết định khi nào bắt đầu và kết thúc bài thi.
Xem danh sách các phòng thi và trạng thái của mỗi phòng (chưa bắt đầu, đang diễn ra, đã kết thúc).
Tham gia một phòng thi nếu bài thi chưa bắt đầu.
Xem kết quả thi của các phòng đã hoàn thành.
Khi bài thi bắt đầu, máy chủ (server) sẽ gửi câu hỏi thi cho người dùng.
Cho phép người dùng thay đổi các câu hỏi đã trả lời miễn là còn thời gian.
Người dùng có thể nộp bài sớm, trước khi hết giờ thi.
Sau khi hết giờ thi, máy chủ thông báo cho người dùng về số lượng câu trả lời đúng.
2. Chế độ Luyện tập (Practice Mode):
Trong chế độ này, người dùng có thể làm các bài luyện tập với số lượng câu hỏi và giới hạn thời gian được thiết lập sẵn.
Sau khi hoàn thành bài luyện tập, ứng dụng hiển thị số lượng câu trả lời đúng.
3. Các tính năng khác:
Phân loại câu hỏi theo độ khó và cho phép người dùng tùy chỉnh loại câu hỏi được bao gồm trong bài thi.
Lưu trữ thông tin về các bài thi mà người dùng đã làm và hiển thị lại khi có yêu cầu.
Cung cấp thống kê dưới dạng đồ thị về các bài thi mà người dùng đã hoàn thành.
Giao diện Người dùng Đồ họa (GUI).
Đây là các chức năng chính cho một ứng dụng thi trắc nghiệm với cả hai chế độ thi và luyện tập, cùng với các tùy chọn bổ sung về trực quan hóa dữ liệu và tùy chỉnh câu hỏi.

thang điểm (Phân bổ điểm):
Xử lý luồng (Stream handling): 1 điểm
Triển khai cơ chế I/O socket trên máy chủ: 2 điểm
Đăng ký và quản lý tài khoản: 2 điểm
Đăng nhập và quản lý phiên: 2 điểm
Quản lý kiểm soát truy cập: 1 điểm
Tham gia chế độ luyện tập: 1 điểm
Tạo phòng thi: 2 điểm
Xem danh sách phòng thi: 1 điểm
Tham gia phòng thi: 2 điểm
Bắt đầu bài thi: 1 điểm
Thay đổi các câu trả lời đã chọn trước đó: 1 điểm
Nộp bài và chấm điểm bài thi: 2 điểm
Xem kết quả thi của các phòng đã hoàn thành: 1 điểm
Ghi nhật ký (log) các hoạt động: 1 điểm
Phân loại câu hỏi theo độ khó, chủ đề: 1-3 điểm
Lưu trữ thông tin bài thi + thống kê đồ thị: 2 điểm
Giao diện Người dùng Đồ họa (GUI): 3 điểm
Các tính năng nâng cao: 1-10 điểm
Đây là thang điểm phân bổ cho các chức năng khác nhau của ứng dụng thi trực tuyến, làm nổi bật cả các tính năng cốt lõi và các tính năng nâng cao tiềm năng.

