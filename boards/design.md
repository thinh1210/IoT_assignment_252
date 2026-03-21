# IoT System Architecture Design

Tài liệu này ghi chú lại kiến trúc hệ thống Mode-Based IoT chạy trên nền tảng FreeRTOS.

## 1. Tổng quan Kiến trúc (Layered Architecture)

Hệ thống được chia thành 3 lớp chính để đảm bảo tính module hóa và dễ dàng mở rộng:

1.  **Input Layer (Lớp đầu vào)**: 
    *   Trách nhiệm: Đọc dữ liệu từ cảm biến (DHT11), giám sát nút nhấn (ButtonHandler).
    *   Cơ chế: Chạy các Task riêng biệt cho mỗi Mode (Mode A, Mode B). Mỗi Task tự quản lý tần suất đọc và gửi sự kiện (`SystemEvent`) vào hàng đợi trung tâm (`mainQueue`).
    
2.  **Processing Layer (Lớp xử lý)**:
    *   Trách nhiệm: Tiếp nhận sự kiện từ hàng đợi, thực thi các logic điều khiển (như chớp LED theo nhiều kiểu khác nhau thông qua `LedController`).
    *   Cơ chế: Tương tự Input Layer, nó có các Task riêng cho từng Mode. Các Task này lắng nghe hàng đợi một cách không chặn (non-blocking) để xử lý sự kiện ngay lập tức.

3.  **Task Manager (Trình quản lý Task)**:
    *   Trách nhiệm: Điều phối việc khởi tạo các Task của các Layer và thực hiện chuyển đổi trạng thái (Switch Mode) bằng cách `Suspend` (tạm dừng) các Task cũ và `Resume` (kích hoạt) các Task mới.

---

## 2. Quy trình thêm linh kiện/phần cứng mới

Khi thêm một phần cứng mới, hãy tuân thủ các bước sau:

### Cho thiết bị Input (Sensor/Button/...)
1.  Tạo Class điều hành trong thư mục `src/Input/` và `include/Input/`.
2.  Đăng ký và khởi tạo trong `InputLayer::init()`.
3.  Cập nhật logic đọc trong các hàm `taskModeXLoop`.
4.  **Quan trọng**: Ghi chú lại thiết lập phần cứng vào file `.md` tương ứng trong thư mục `boards/input/`.

### Cho thiết bị Output (LED/Buzzer/Motor/...)
1.  Tạo Class điều hành trong thư mục `src/Processing/` và `include/Processing/`.
2.  Nếu thiết bị dùng chung tài nguyên phần cứng, hãy sử dụng **Mutex** (như `LedController::ledMutex`) để tránh xung đột.
3.  Tạo các hàm thực thi logic (ví dụ: `blink...`, `patternX...`).
4.  **Quan trọng**: Ghi chú lại thiết lập phần cứng vào file `.md` tương ứng trong thư mục `boards/processing/`.

---

## 3. Quản lý tài nguyên hệ thống

*   **Queue**: Sử dụng `mainQueue` để giao tiếp bất đồng bộ giữa các Layer.
*   **Mutex**: Sử dụng Mutex cho các thiết bị Output để đảm bảo tính toàn vẹn khi nhiều Task cùng muốn truy cập phần cứng.
*   **Stack Size**: Mặc định sử dụng **4096 bytes** cho các Task để tránh lỗi tràn bộ nhớ (Stack Overflow).
