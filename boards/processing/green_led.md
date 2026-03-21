# Green LED Output Setup

Dữ liệu điều khiển đèn LED thông báo trên board.

## Cấu hình chân (GPIO)
*   **LED PIN**: `GPIO_NUM_48` (Được cấu hình trong `config.h` qua `GREEN_LED_GPIO`)
*   **Trạng thái**: Active HIGH.

## Logic điều khiển
1.  **Lớp Processing**: Thông qua `LedController` để điều khiển chớp tắt.
2.  **Cơ chế an toàn**: 
    *   Sử dụng **Mutex** (`ledMutex`) để khóa tài nguyên khi đang chạy các hiệu ứng chớp tắt kéo dài (như hàm `blink200ms_for_3s`).
    *   Task khác muốn chớp LED sẽ phải đợi cho đến khi Task hiện tại hoàn thành và giải phóng Mutex.
3.  **Các kiểu chớp (Patterns)**:
    *   **1000ms x 3**: Chớp chậm (Thường dùng cho Mode A).
    *   **500ms x 3**: Chớp trung bình (Thường dùng cho Mode B).
    *   **200ms trong 3s**: Chớp nhanh báo hiệu sự kiện đặc biệt (ví dụ nhấn nút).
