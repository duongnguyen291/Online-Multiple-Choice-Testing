-- Sample data for testing
-- Insert default users (passwords are hashed with simple SHA256 for demo)

-- Default teacher: username=teacher, password=teacher123
INSERT INTO Users (username, hashed_password, role) VALUES 
('teacher', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855', 'TEACHER');

-- Default users
INSERT INTO Users (username, hashed_password, role) VALUES 
('user1', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855', 'USER'),
('user2', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855', 'USER'),
('user3', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855', 'USER');

-- Sample questions - Geography
INSERT INTO Questions (content, options, correct_option, difficulty, topic, created_by) VALUES
('Thủ đô của Việt Nam là gì?', '{"a":"Hà Nội","b":"TP. Hồ Chí Minh","c":"Đà Nẵng","d":"Hải Phòng"}', 'a', 'easy', 'Địa lý', 1),
('Sông dài nhất Việt Nam là sông nào?', '{"a":"Sông Hồng","b":"Sông Cửu Long","c":"Sông Đồng Nai","d":"Sông Mekong"}', 'd', 'medium', 'Địa lý', 1),
('Núi cao nhất Việt Nam là núi gì?', '{"a":"Núi Bà Đen","b":"Núi Ngọc Linh","c":"Phan Xi Păng","d":"Núi Bạch Mã"}', 'c', 'easy', 'Địa lý', 1),
('Việt Nam có bao nhiêu tỉnh thành?', '{"a":"60","b":"63","c":"65","d":"58"}', 'b', 'medium', 'Địa lý', 1),
('Biển Đông nằm ở phía nào của Việt Nam?', '{"a":"Bắc","b":"Nam","c":"Đông","d":"Tây"}', 'c', 'easy', 'Địa lý', 1);

-- Sample questions - Math
INSERT INTO Questions (content, options, correct_option, difficulty, topic, created_by) VALUES
('2 + 2 = ?', '{"a":"3","b":"4","c":"5","d":"6"}', 'b', 'easy', 'Toán học', 1),
('Căn bậc hai của 144 là?', '{"a":"10","b":"11","c":"12","d":"13"}', 'c', 'easy', 'Toán học', 1),
('Nếu x + 5 = 10, thì x = ?', '{"a":"3","b":"4","c":"5","d":"6"}', 'c', 'easy', 'Toán học', 1),
('3 × 7 = ?', '{"a":"20","b":"21","c":"22","d":"23"}', 'b', 'easy', 'Toán học', 1),
('100 ÷ 4 = ?', '{"a":"20","b":"25","c":"30","d":"35"}', 'b', 'easy', 'Toán học', 1),
('Giải phương trình x² - 4 = 0, x = ?', '{"a":"±1","b":"±2","c":"±3","d":"±4"}', 'b', 'medium', 'Toán học', 1),
('Tính đạo hàm của f(x) = x²', '{"a":"x","b":"2x","c":"x²","d":"2x²"}', 'b', 'hard', 'Toán học', 1);

-- Sample questions - History
INSERT INTO Questions (content, options, correct_option, difficulty, topic, created_by) VALUES
('Việt Nam giành độc lập vào năm nào?', '{"a":"1945","b":"1946","c":"1954","d":"1975"}', 'a', 'easy', 'Lịch sử', 1),
('Chiến thắng Điện Biên Phủ diễn ra vào năm nào?', '{"a":"1953","b":"1954","c":"1955","d":"1956"}', 'b', 'medium', 'Lịch sử', 1),
('Ngày Quốc khánh Việt Nam là ngày nào?', '{"a":"1/1","b":"30/4","c":"2/9","d":"19/5"}', 'c', 'easy', 'Lịch sử', 1),
('Ai là người đọc Tuyên ngôn Độc lập năm 1945?', '{"a":"Hồ Chí Minh","b":"Võ Nguyên Giáp","c":"Phạm Văn Đồng","d":"Lê Duẩn"}', 'a', 'easy', 'Lịch sử', 1),
('Chiến dịch Hồ Chí Minh kết thúc vào ngày nào?', '{"a":"30/4/1975","b":"1/5/1975","c":"2/9/1945","d":"7/5/1954"}', 'a', 'medium', 'Lịch sử', 1);

-- Sample questions - Computer Science
INSERT INTO Questions (content, options, correct_option, difficulty, topic, created_by) VALUES
('HTML là viết tắt của gì?', '{"a":"Hyper Text Markup Language","b":"High Tech Modern Language","c":"Home Tool Markup Language","d":"Hyperlinks and Text Markup Language"}', 'a', 'easy', 'Tin học', 1),
('TCP/IP hoạt động ở tầng nào của mô hình OSI?', '{"a":"Tầng 1 và 2","b":"Tầng 3 và 4","c":"Tầng 5 và 6","d":"Tầng 6 và 7"}', 'b', 'hard', 'Tin học', 1),
('Thuật toán nào sau đây là thuật toán sắp xếp?', '{"a":"DFS","b":"BFS","c":"Quick Sort","d":"Dijkstra"}', 'c', 'medium', 'Tin học', 1),
('Trong C++, int thường chiếm bao nhiêu byte?', '{"a":"2","b":"4","c":"8","d":"16"}', 'b', 'medium', 'Tin học', 1),
('Big O của thuật toán tìm kiếm nhị phân là?', '{"a":"O(n)","b":"O(log n)","c":"O(n²)","d":"O(1)"}', 'b', 'medium', 'Tin học', 1);

-- Sample questions - English
INSERT INTO Questions (content, options, correct_option, difficulty, topic, created_by) VALUES
('What is the past tense of "go"?', '{"a":"goed","b":"went","c":"gone","d":"going"}', 'b', 'easy', 'Tiếng Anh', 1),
('Which word is a noun?', '{"a":"quickly","b":"run","c":"beautiful","d":"book"}', 'd', 'easy', 'Tiếng Anh', 1),
('"She ___ to the store yesterday." Fill in the blank.', '{"a":"go","b":"goes","c":"went","d":"going"}', 'c', 'easy', 'Tiếng Anh', 1),
('What does "enormous" mean?', '{"a":"very small","b":"very large","c":"very fast","d":"very slow"}', 'b', 'medium', 'Tiếng Anh', 1),
('Which sentence is correct?', '{"a":"He dont like it","b":"He doesnt like it","c":"He doesnt likes it","d":"He doesnt like it"}', 'd', 'medium', 'Tiếng Anh', 1);

