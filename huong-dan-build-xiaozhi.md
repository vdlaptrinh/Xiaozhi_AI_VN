# Hướng dẫn build firmware Xiaozhi ESP32 (tiếng Việt AI)

## 1. Cài đặt môi trường
- Cài **ESP-IDF v6.1** (The project requires ESP-IDF v6.0.1 or later, khuyến nghị v6.1)
- Cài **Visual Studio Code** (kèm extension C/C++ + ESP-IDF để chỉnh sửa code dễ dàng)

## 2. Lấy mã nguồn
```bash
git clone https://github.com/78/xiaozhi-esp32
cd xiaozhi-esp32
git submodule update --init --recursive
```

> **Lưu ý quan trọng:** phải chạy `git submodule update --init --recursive`, nếu bỏ qua bước này build sẽ lỗi thiếu các thành phần (motor, display, driver...).

## 3. Tùy chỉnh code (tùy chọn)
- Mở thư mục bằng VS Code để chỉnh sửa code, thêm các chức năng:
  - Thông báo tin tức
  - Giá vàng
  - Tích hợp MCP / Home Assistant...
- Các phần này nằm trong `main/`, cấu hình ở `main/Kconfig`.

## 4. Build với ESP-IDF
Mở terminal **IDF CMD / IDF PowerShell v6.1**, rồi:
```bash
cd xiaozhi-esp32
idf.py set-target esp32s3
idf.py menuconfig
```

### Trong `menuconfig`:
- **Xiaozhi Assistant** → **Board Type** → chọn board của bạn, theo dõi các mục:
  - Video: chọn loại board (VD: M5StickC Plus, board tự làm...)
  - Chọn **ngôn ngữ**
  - Chọn **loại màn hình**
  - Cấu hình loa / mic
  - Cấu hình server ASR nhận diện giọng nói tiếng Việt (VD FunASR / Whisper) tại **Audio** hoặc **Network/ASR**

## 5. Biên dịch
```bash
idf.py build
```

## 6. Nạp firmware và xem log
```bash
idf.py flash monitor
```
(Thoát monitor bằng `Ctrl + ]`)

## 7. Gộp firmware (tùy chọn)
```bash
idf.py merge-bin
```
File gộp nằm trong `build/` (VD `merged.bin`), dùng để flash qua web ESP Home Flasher hoặc tool ngoài.

## Mẹo
- Kiểm tra đúng board trong `menuconfig` trước khi build — sai board sẽ lỗi hoặc màn hình không hoạt động.
- Với tiếng Việt cần cấu hình server ASR từ xa trong `menuconfig` → `Audio`.