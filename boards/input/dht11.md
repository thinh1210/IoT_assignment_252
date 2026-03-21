# DHT11 Sensor Setup

Dữ liệu kĩ thuật của cảm biến nhiệt độ & độ ẩm DHT11.

## Cấu hình chân (GPIO)
*   **DATA PIN**: `GPIO_NUM_1` (Được cấu hình trong `config.h` qua `DHT11_GPIO`)
*   **VCC**: 3.3V hoặc 5V tùy loại module.
*   **GND**: GND chung với Board.

## Logic điều khiển
1.  **Lớp Input**: `DHTSensor` class quản lý việc giao tiếp cấp thấp qua thư viện `DHT.h`.
2.  **Chu kỳ đọc**:
    *   **Mode A**: Đọc định kỳ mỗi **3 giây**.
    *   **Mode B**: Đọc định kỳ mỗi **1 giây** (Chế độ Real-time).
3.  **Hành động**: Sau khi đọc thành công, gửi sự kiện `SENSOR_DATA_READY` vào `mainQueue` để các lớp xử lý cập nhật hiển thị hoặc thực hiện logic điều khiển tiếp theo.
