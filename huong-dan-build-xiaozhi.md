# Hướng dẫn build firmware Xiaozhi AI Tiếng Việt
(ver có hình ảnh [file pdf](https://github.com/vdlaptrinh/Xiaozhi_AI_VN/blob/main/build_xiaozhi_ai.pdf))
## 1. Cài đặt môi trường
- **Git**: https://git-scm.com/install/windows
- **Visual Studio Code**: https://code.visualstudio.com/
- **Python**: phiên bản 3.10 trở lên (ESP-IDF yêu cầu tối thiểu Python 3.10):
  https://www.python.org/downloads/windows/
- **EIM** (chọn bản theo hệ điều hành: Windows hoặc MacBook):
  https://dl.espressif.com/dl/eim/
- **ESP-IDF v6.1** (chọn bản theo hệ điều hành; bản offline):
  https://dl.espressif.com/dl/eim/?tab=offline

### Cách cài EIM (bản offline GUI)
1. Mở ứng dụng cài đặt (EIM).
2. Nếu file `.zst` nằm cùng thư mục với EIM thì sẽ được tự động phát hiện. Nếu không: bấm **New Installation** → **Offline Installation** hoặc **Browse Archive File**.
3. Chọn file lưu trữ `.zst`.
4. Màn hình tiếp theo hiển thị file đã chọn, cho phép cấu hình **Installation Path** (dùng đường dẫn mặc định hoặc tự chọn).
5. Bấm **Start Installation** để bắt đầu. Quá trình cài hiển thị tương tự bản expert.

## 2. Lấy mã nguồn
```
git clone https://github.com/78/xiaozhi-esp32
cd xiaozhi-esp32
git submodule update --init --recursive
```

## 3. Tùy chỉnh code (tùy chọn)
Mở thư mục bằng VS Code để chỉnh sửa code và thêm tính năng:
- Thông báo tin tức
- Giá vàng
- Tích hợp MCP / Home Assistant…

Các phần này nằm trong `main/`, cấu hình ở `main/Kconfig`.

## 4. Build với ESP-IDF
Mở terminal **IDF CMD / IDF PowerShell v6.1**, sau đó:
```
cd xiaozhi-esp32
idf.py set-target esp32s3
idf.py menuconfig
```

Trong `menuconfig`, tại mục **Xiaozhi Assistant** → **Board Type**, chọn board của bạn:
- Chọn loại board (VD: M5StickC Plus, board tự làm…)
- Chọn ngôn ngữ
- Chọn loại màn hình
- Cấu hình loa / mic
- Cấu hình server ASR nhận diện giọng nói tiếng Việt (VD FunASR / Whisper) tại **Audio** hoặc **Network/ASR**

### Quy trình khi build
1. Mở **IDF_v6.1_Powershell**
2. `cd` đến thư mục `xiaozhi_esp32`
3. `code .` để mở VS Code
4. Sửa dòng ~38 để ngôn ngữ mặc định cho chatbot là Tiếng Việt. Xem cấu hình của board tại thư mục tên board tương ứng.
   *(Cách thêm 1 board chưa có sẽ hướng dẫn sau.)*
5. Quay lại **IDF_v6.1_Powershell** chạy:
```
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py build flash monitor
idf.py merge-bin
```

Trong **menuconfig** → **Xiaozhi Assistant**:
- **Target Board**: chọn board đang lập trình
- **OLED Type**: chọn màn hình (một số board không có mục này)

Tùy chọn: tự đánh thức "Hi, Jason"
Bấm `q` rồi `Y` để lưu. Sau đó `idf.py build`.

## 5. Biên dịch
```
idf.py build
```
Sau khi build xong thì flash. Đã kết nối thiết bị với cổng USB của máy tính.

## 6. Nạp firmware và xem log
```
idf.py flash monitor
```
(Thoát monitor bằng `Ctrl + ]`)

## 7. Gộp firmware (tùy chọn)
```
idf.py merge-bin
```
File gộp nằm trong `build/` (VD `merged.bin`), dùng để flash qua web **ESP Home Flasher** hoặc tool ngoài.