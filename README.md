# Xiaozhi AI VN (`Xiaozhi_AI_VN`)

Kho lưu trữ mã nguồn, phần cứng và hướng dẫn cấu hình để tự build **Xiaozhi AI** phiên bản tiếng Việt sử dụng vi điều khiển ESP32-S3.

---

## 📂 Cấu trúc Repository

```text
Xiaozhi_AI_VN/
├── esp32s3_kit/         # Thiết kế phần cứng (KiCAD 10 project, có sẵn file xuất PCB PDF)
├── firmware/            # Chứa các file binary (.bin) biên dịch sẵn cho từng loại board
└── README.md            # Tài liệu giới thiệu dự án

---

1. Thư mục esp32s3_kit/
Phần cứng thiết kế: Dự án mạch phần cứng dùng KiCAD (phiên bản 10).

Tài liệu sản xuất: Đã hỗ trợ sẵn các file xuất PCB dưới dạng PDF giúp dễ dàng xem xét, in ấn hoặc đặt mạch (Gerber/PCB layout).

2. Thư mục firmware/
File Binary: Tổng hợp các file .bin sẵn sàng để flash trực tiếp lên các dòng board ESP32 hỗ trợ dự án.

Giúp người dùng có thể trải nghiệm nhanh chóng mà chưa cần tự build mã nguồn từ đầu.

🚀 Hướng dẫn Build Firmware (Đang cập nhật)
Phần hướng dẫn chi tiết từng bước cách tự build firmware Xiaozhi AI tiếng Việt sẽ sớm được cập nhật tại đây:

[ ] Cài đặt môi trường phát triển (ESP-IDF / PlatformIO).

[ ] Cấu hình kết nối Audio (I2S, Micro, Loa) và Wi-Fi/Server AI.

[ ] Tiến hành biên dịch và nạp firmware (flash) vào ESP32-S3.

🤝 Đóng góp & Phát triển
Mọi đóng góp, báo lỗi (issue) hoặc thảo luận cải tiến phần cứng/phần mềm đều được hoan nghênh để hoàn thiện cộng đồng Xiaozhi AI tại Việt Nam!