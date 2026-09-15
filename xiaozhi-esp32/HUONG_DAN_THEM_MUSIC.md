# HƯỚNG DẪN THÊM TÍNH NĂNG MCP MUSIC VÀO XIAOZHI-ESP32

Hướng dẫn này tổng hợp **đầy đủ các bước đã thực hiện** để port tính năng "MCP music"
từ dự án cũ `xiaozhi-esp32-music` sang dự án mới `xiaozhi-esp32`.

- Dự án đích: `D:\DAI\esp32_projects\xiaozhi-esp32`
- Dự án nguồn: `D:\DAI\esp32_projects\xiaozhi-esp32-music`

## 1. TỔNG QUAN

Tính năng gồm:

1. **Trình phát nhạc MP3 stream** (`Esp32Music`): gọi API `http://110.42.59.54:2233/stream_pcm?song=...`
   để lấy `audio_url`, tải MP3 theo stream, decode bằng libhelix, phát PCM ra loa,
   hiển thị tên bài hát + lời bài hát (lyrics LRC).
2. **2 tool MCP**: `self.music.play_song` và `self.music.set_display_mode`.
3. **Tự động dừng nhạc** khi thiết bị rời khỏi trạng thái idle (bấm nút / wake word).

Có **3 file mới** và **8 file sửa** (xem chi tiết bên dưới).

---

## 2. CÁC FILE MỚI (THÊM MỚI)

Đều nằm trong thư mục: `main\boards\common\`

### 2.1. `music.h` — interface cơ sở (tạo mới)

Nội dung đầy đủ:

```cpp
#ifndef MUSIC_H
#define MUSIC_H

#include <cstdint>
#include <string>

class Music {
public:
    // 显示模式
    enum class DisplayMode {
        kSpectrum = 0,  // 频谱显示
        kLyrics = 1     // 歌词显示
    };

    virtual ~Music() = default;

    virtual bool Download(const std::string& song_name, const std::string& artist_name = "") = 0;
    virtual std::string GetDownloadResult() = 0;

    virtual bool StartStreaming(const std::string& music_url) = 0;
    virtual bool StopStreaming() = 0;
    virtual size_t GetBufferSize() const = 0;
    virtual bool IsDownloading() const = 0;
    virtual int16_t* GetAudioData() = 0;

    virtual void SetDisplayMode(DisplayMode mode) = 0;
    virtual DisplayMode GetDisplayMode() const = 0;
};

#endif  // MUSIC_H
```

> Khác với bản nguồn: `DisplayMode` được đưa vào *interface* `Music` (thay vì chỉ ở trong `Esp32Music`)
> để `mcp_server.cc` dùng được mà không cần include `esp32_music.h`.

### 2.2. `esp32_music.h` — header lớp `Esp32Music` (tạo mới)

Gồm: các thành viên `std::thread` (play/download/lyric), bộ đệm `std::queue<AudioChunk>`,
biến MP3 `HMP3Decoder`, enum `DisplayMode` (alias của `Music::DisplayMode`),
các hàm private `DownloadAudioStream/PlayAudioStream/ClearAudioBuffer/...`,
public API `Download/GetDownloadResult/StartStreaming/StopStreaming/GetBufferSize/IsDownloading/GetAudioData/SetDisplayMode/GetDisplayMode`.

Điểm quan trọng so với bản nguồn:
- Đã **bỏ** khai báo hàm `ResetSampleRate();` (API cũ không có trong codec mới).
- Có thêm `#include <cstdint>`.
- `DisplayMode` = `Music::DisplayMode`.

### 2.3. `esp32_music.cc` — cài đặt trình phát (tạo mới)

~1560 dòng, port từ `esp32_music.cc` của dự án nguồn với các **thay đổi bắt buộc** sau:

| Phần | Bản nguồn (cũ) | Bản port (mới) |
|------|----------------|----------------|
| Constructor | `display_mode_(DISPLAY_MODE_LYRICS)`, log "default spectrum display mode" | `display_mode_(DisplayMode::kLyrics)`, log "default lyrics display mode", vẫn `InitializeMp3Decoder()` |
| `Download()` | có log "qq交流群826072986" | bỏ log đó, các bước gọi API/auth header/JSON giống y hệt; dùng `DisplayMode::kLyrics` |
| `StartStreaming()` | `esp_pthread_set_cfg` 8KB stack, spawn download + play thread | giữ nguyên (IDF 6.1 vẫn có `esp_pthread_set_cfg`) |
| `StopStreaming()` | gọi `ResetSampleRate()`, `display->SetMusicInfo("")`, `display->stopFft()` | **bỏ cả 3**; vẫn join thread + timeout 1s + detach nếu quá lâu |
| `PlayAudioStream()` | chặn đầu: `codec->output_enabled()` else abort | **bỏ abort**; chỉ check `codec` null; sau khi đủ buffer thì `codec->EnableOutput(true)` nếu output đang tắt |
| Trạng thái (trong vòng lặp play) | `kDeviceStateListening/Speaking` → `app.ToggleChatState()` để ép về idle | **bỏ ToggleChatState** → mọi trạng thái khác idle chỉ `delay(50ms); continue` (tạm dừng, khi idle tự chạy tiếp) |
| Hiển thị tên bài | `display->SetMusicInfo("《...》播放中...")`, `display->start()` (spectrum) | `display->SetChatMessage("system", "《...》播放中...")`; spectrum chỉ log "$not supported"; lyrics vẫn "Lyrics display mode active" |
| Xuất PCM | `app.AddAudioData(AudioStreamPacket)` + chuyển codec sang sample rate của MP3 | **bỏ `AddAudioData`** → tự resample tuyến tính về `codec->output_sample_rate()` rồi `codec->OutputData(output_pcm)`; trước mỗi frame nếu `!codec->output_enabled()` thì `codec->EnableOutput(true)`; sau mỗi frame gọi `Application::GetInstance().GetAudioService().MarkOutputActive()` (chống power-save tắt loa giữa bài) |
| Chuyển stereo→mono | nhân đôi code giữa 2 nhánh | gộp dùng `mono_buffer.assign(...)` cho mono/other |
| `final_pcm_data_fft` | ghi MONO gốc (chưa resample), malloc theo `final_sample_count` | ghi bản đã resample, malloc theo độ dài thực tế |
| `ResetSampleRate()` | có (dùng `codec->SetOutputSampleRate(-1)`, `codec->original_output_sample_rate()`) | **xóa hoàn toàn** (API không tồn tại ở đích) |
| `DownloadLyrics()` | có log "qq交流群" | bỏ log đó; retry/redirect/read logic giữ nguyên |
| `UpdateLyricDisplay()` | `display->SetChatMessage("lyric", ...)` | **giữ nguyên** (đích cũng có API này) |
| `SetDisplayMode()` | `DISPLAY_MODE_SPECTRUM/LYRICS` | dùng `DisplayMode::kSpectrum/kLyrics` |

Các helper giữ nguyên: `get_device_mac`, `get_device_chip_id`, `generate_dynamic_key`
(SHA256, secret `"your-esp32-secret-key-2024"`), `add_auth_headers`, `url_encode`,
`buildUrlWithParams` (không dùng), `ResamplePcm` (mới thêm).

> ⚠️ **QUAN TRỌNG với IDF 6.1 (mbedtls 4.x / TF-PSA-Crypto):** header `mbedtls/sha256.h` đã
> **không còn public** (bị chuyển vào thư mục private). Build sẽ báo
> `fatal error: mbedtls/sha256.h: No such file or directory`.
> **Bắt buộc sửa** trong `esp32_music.cc`:
> - `#include <mbedtls/sha256.h>` → `#include <mbedtls/md.h>`
> - `mbedtls_sha256((unsigned char*)data.c_str(), data.length(), hash, 0);` → 
>   ```cpp
>   const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
>   mbedtls_md(md_info, (unsigned char*)data.c_str(), data.length(), hash);
>   ```
> `mbedtls/md.h` nằm cùng thư mục public với `mbedtls/base64.h` (include path đã có qua
> `mbedtls` trong PRIV_REQUIRES của `main`), và `mbedtls_md()` được build sẵn trong target
> `extras` của component mbedtls → **biên dịch + link bình thường** (được ~17 component
> khác trong IDF dùng chung).

### ⚠️ Các điểm phải sửa thêm khi port sang IDF 6.1 (đã làm trong `esp32_music.cc`)

1. **HTTP API trả `std::expected<int, NetworkError>`** (managed component `78__esp-ml307`),
   KHÔNG còn trả `int` trực tiếp. Phải sửa tại 5 chỗ (`Download`, `DownloadAudioStream`, `DownloadLyrics`):
   - `GetStatusCode()`: dùng `auto status_code = http->GetStatusCode();` rồi kiểm tra
     `if (!status_code || *status_code != 200)`; in log dùng `*status_code`.
   - `Read()`: dùng `auto read_result = http->Read(...); if (!read_result) { ... read_result.error().ToString() ... } int n = *read_result;`.
2. **ESP-IDF build với `-fno-exceptions`** → không được dùng `try/catch`. Đã thay khối
   `std::stoi`/`std::stof` trong `ParseLyrics` bằng hàm helper `parse_lrc_timestamp()`
   (tự parse "mm:ss.xx" không ném exception; lỗi → log `Failed to parse time` rồi `continue`).
3. `buildUrlWithParams()` không được dùng → cảnh báo `-Wunused-function`, nhưng không chặn build
   (flag có `-Wno-error=unused-function`), nên **giữ nguyên** cho khớp bản nguồn.

---

## 3. CÁC FILE SỬA (THÊM/XÓA DÒNG CỤ THỂ)

> Cách đọc: **THÊM** = thêm dòng mới; **SỬA/THAY** = thay cả đoạn cũ bằng đoạn mới;
> **XÓA** = xóa dòng trong bản nguồn. Tất cả đường dẫn đều nằm trong `D:\DAI\esp32_projects\xiaozhi-esp32\main\`.

### 3.1. `boards\common\board.h`

1. **THÊM** 1 dòng vào nhóm include (sau dòng `#include "assets.h"`, trở thành dòng 16):
   ```cpp
   #include "music.h"
   ```
2. **THAY** dòng destructor:
   ```cpp
   // XÓA dòng cũ:
   virtual ~Board() = default;
   // THÊM dòng mới (định nghĩa trong board.cc):
   virtual ~Board();
   ```
3. **THÊM** 2 dòng vào khối `protected` (sau dòng `std::string uuid_;`):
   ```cpp
   // 音乐播放器（MCP音乐功能）
   Music* music_ = nullptr;
   ```
4. **THÊM** 1 dòng (sau dòng `virtual Camera* GetCamera();`):
   ```cpp
   virtual Music* GetMusic();
   ```
   → thứ tự các hàm thay đổi: `GetDisplay(); GetCamera(); GetMusic(); GetNetwork(); ...`

### 3.2. `boards\common\board.cc`

1. **THÊM** include tại đầu file (sau `#include "display/display.h"`, dòng 4):
   ```cpp
   #include "esp32_music.h"
   ```
2. **SỬA** constructor — sau dòng log UUID, **THÊM** dòng khởi tạo music:
   ```cpp
   Board::Board() {
       Settings settings("board", true);
       uuid_ = settings.GetString("uuid");
       if (uuid_.empty()) {
           uuid_ = GenerateUuid();
           settings.SetString("uuid", uuid_);
       }
       ESP_LOGI(TAG, "UUID=%s SKU=%s", uuid_.c_str(), BOARD_NAME);

       music_ = new Esp32Music();        // <-- THÊM dòng này
   }
   ```
3. **THÊM** destructor mới ngay sau constructor (trước `std::string Board::GenerateUuid()`):
   ```cpp
   Board::~Board() {
       if (music_ != nullptr) {
           delete music_;
           music_ = nullptr;
       }
   }
   ```
4. **THÊM** hàm `GetMusic()` ngay sau `Camera* Board::GetCamera() { return nullptr; }` (dòng ~66):
   ```cpp
   Music* Board::GetMusic() { return music_; }
   ```

### 3.3. `application.cc`

**SỬA/THAY** khối "Add state change listeners" (ở trong `Application::Initialize()`, dòng ~97).

Đoạn cũ (2 dòng bên trong lambda):
```cpp
    // Add state change listeners
    state_machine_.AddStateChangeListener([this](DeviceState old_state, DeviceState new_state) {
        xEventGroupSetBits(event_group_, MAIN_EVENT_STATE_CHANGED);
    });
```

Đoạn mới (thêm khối `if` dừng nhạc):
```cpp
    // Add state change listeners
    state_machine_.AddStateChangeListener([this](DeviceState old_state, DeviceState new_state) {
        // Stop music streaming when leaving the idle state (e.g. wake word / button pressed)
        if (old_state == kDeviceStateIdle && new_state != kDeviceStateIdle) {
            auto music = Board::GetInstance().GetMusic();
            if (music != nullptr) {
                ESP_LOGI(TAG, "Device leaving idle state, stopping music streaming");
                music->StopStreaming();
            }
        }
        xEventGroupSetBits(event_group_, MAIN_EVENT_STATE_CHANGED);
    });
```
→ Thực chất: **giữ nguyên** dòng `xEventGroupSetBits(...)`, **THÊM** 8 dòng (khối `if`) phía trên nó.

> `kDeviceStateIdle` được định nghĩa trong `main/device_state.h`, đã có sẵn (không cần sửa).

### 3.4. `mcp_server.cc`

1. **THÊM** include `<cctype>` vào nhóm STL include (dòng ~11, sau `#include <algorithm>`):
   ```cpp
   #include <algorithm>
   #include <cctype>      // <-- THÊM
   #include <cstring>
   #include <iterator>
   ```
2. **THÊM** khối 2 tool music vào trong `McpServer::AddCommonTools()`, ngay sau khối
   `#ifdef HAVE_LVGL ... #endif` (camera) và **trước** comment `// Restore the original tools list...`
   (kết quả đánh số dòng: 119–167):

   ```cpp
       auto music = board.GetMusic();
       if (music) {
           AddTool("self.music.play_song",
                   "Search and play a song through the built-in music player.\n"
                   "Args:\n"
                   "  `song_name`: The name of the song to play.\n"
                   "  `artist_name`: The name of the singer (optional).\n"
                   "Return:\n"
                   "  A JSON object indicating that the song has started playing.\n"
                   "The music only plays while the device is in the idle state.",
                   PropertyList({Property("song_name", kPropertyTypeString),
                                 Property("artist_name", kPropertyTypeString, std::string(""))}),
                   [music](const PropertyList& properties) -> ToolResult {
                       auto song_name = properties["song_name"].value<std::string>();
                       auto artist_name = properties["artist_name"].value<std::string>();
                       if (!music->Download(song_name, artist_name)) {
                           return std::unexpected("Failed to get the music resource, please try "
                                                  "another song");
                       }
                       cJSON* result = cJSON_CreateObject();
                       if (result == nullptr) {
                           return std::unexpected("Failed to allocate music result");
                       }
                       cJSON_AddStringToObject(result, "message", "The song has started playing");
                       cJSON_AddStringToObject(result, "song_name", song_name.c_str());
                       return result;
                   });

           AddTool("self.music.set_display_mode",
                   "Set the display mode of the music player. The mode can be `spectrum` or "
                   "`lyrics`. Note: `spectrum` is not supported in the current port and only "
                   "`lyrics` mode shows content on the screen.",
                   PropertyList({Property("mode", kPropertyTypeString)}),
                   [music](const PropertyList& properties) -> ToolResult {
                       auto mode = properties["mode"].value<std::string>();
                       std::transform(mode.begin(), mode.end(), mode.begin(),
                                      [](unsigned char c) { return std::tolower(c); });
                       if (mode == "spectrum" || mode == "频谱") {
                           music->SetDisplayMode(Music::DisplayMode::kSpectrum);
                           return std::string("Display mode set to spectrum");
                       }
                       if (mode == "lyrics" || mode == "歌词") {
                           music->SetDisplayMode(Music::DisplayMode::kLyrics);
                           return std::string("Display mode set to lyrics");
                       }
                       return std::unexpected("Invalid display mode, please use 'spectrum' or "
                                              "'lyrics'");
                   });
       }
   ```

> `board` đã tồn tại trong scope (`auto& board = Board::GetInstance();` ở đầu `AddCommonTools`).
> Không cần include thêm trong file này (`music.h` kéo theo qua `board.h`).

### 3.5. `CMakeLists.txt` (thư mục `main\`)

**THÊM** 1 dòng vào khối "Add board common files" `list(APPEND SOURCES ...)` (dòng ~66, sau
`"boards/common/system_reset.cc"`):
```cmake
    "boards/common/system_reset.cc"
    "boards/common/esp32_music.cc"     # <-- THÊM
)
```

### 3.6. `idf_component.yml` (thư mục `main\`)

**THÊM** 2 dòng dependency libhelix (dòng ~113, sau khối `espressif/esp_wifi_remote`):
```yaml
  espressif/esp_wifi_remote:
    version: ^1.6.4
    rules:
    - if: target in [esp32p4]
  chmorgan/esp-libhelix-mp3:            # <-- THÊM
    version: '*'                        # <-- THÊM
  espfriends/servo_dog_ctrl:
```
→ Component này là **bộ giải mã MP3** (`mp3dec.h`) mà `esp32_music.h/.cc` cần.

### 3.7. `audio\audio_service.h`

**THÊM** khai báo phương thức public (sau `void ResetDecoder();`, dòng ~156):
```cpp
    /**
     * Mark the audio output as recently active so that the audio power-saving
     * timer does not disable the speaker. Used by the music player which writes
     * PCM directly to the codec.
     */
    void MarkOutputActive();
```

### 3.8. `audio\audio_service.cc`

**THÊM** định nghĩa (ngay trước `void AudioService::SetModelsList(...)`, dòng ~855):
```cpp
void AudioService::MarkOutputActive() { last_output_time_ = std::chrono::steady_clock::now(); }
```

> Giải thích: AudioService mới có "power save" tắt loa sau 15s không phát. Nhạc viết PCM
> trực tiếp xuống codec (không qua audio service) nên `last_output_time_` không được làm mới
> → nếu không gọi hàm này, loa sẽ bị tắt giữa bài. `MarkOutputActive()` giữ loa bật suốt khi phát nhạc.

---

## 4. FILE NÀO KHÔNG CẦN SỬA (đã kiểm tra)

- `main\application.h` — đã có `GetDeviceState()`, `GetAudioService()`, include `device_state.h` (đủ).
- `main\display\display.h`, `main\display\lcd_display.cc` — đã có `SetChatMessage(role, content)`, dùng với role `"system"` / `"lyric"`.
- `main\audio\audio_codec.h/.cc` — đã có `OutputData(std::vector<int16_t>&)`, `output_sample_rate()`, `output_enabled()`, `EnableOutput(bool)` (API mới dùng để phát PCM).
- `main\device_state.h` — đã có `kDeviceStateIdle`.
- `main\system_info.cc` — đã có `GetMacAddress()` (dùng cho auth header).

---

## 5. KIỂM TRA & BUILD

> ✅ **ĐÃ VERIFY**: build thành công trên board `freenove-esp32s3-display-2.8-lcd`
> (ESP32-S3, IDF 6.1) → tạo `build\xiaozhi.bin` OK (partition app còn 31%).
> Các lỗi gặp phải khi build và cách khắc phục đã nêu ở mục 2.3 (mbedtls/md.h,
> `std::expected`, `-fno-exceptions`).

Build lần đầu:

```powershell
# 1) Nạp môi trường
. C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1

# 2) Vào thư mục dự án, chọn board (ví dụ esp32s3 có PSRAM)
cd D:\DAI\esp32_projects\xiaozhi-esp32
idf.py set-target esp32s3        # chọn đúng board của bạn
idf.py build
```

> Lưu ý 1: các cấp phát nhớ dùng `MALLOC_CAP_SPIRAM` → board **không có PSRAM** sẽ không phát
> được nhạc (giữ đúng hành vi bản nguồn).
> Lưu ý 2: tool `self.music.play_song` chạy trên main loop → `Download()` (HTTP lấy JSON)
> có thể chẹn main loop vài trăm ms–1s; tải stream MP3 vẫn chạy trên thread riêng.

---

## 6. DANH SÁCH ĐẦY ĐỦ (BẢNG TỔNG KẾT)

| # | Thao tác | File | Loại |
|---|----------|------|------|
| 1 | Tạo interface, enum `DisplayMode` | `main\boards\common\music.h` | THÊM MỚI |
| 2 | Header lớp phát nhạc | `main\boards\common\esp32_music.h` | THÊM MỚI |
| 3 | Cài đặt trình phát nhạc (port, ~1560 dòng) | `main\boards\common\esp32_music.cc` | THÊM MỚI |
| 4 | `#include "music.h"` + member `music_` + `GetMusic()` + destructor | `main\boards\common\board.h` | SỬA |
| 5 | `new Esp32Music()` + destructor + `GetMusic()` | `main\boards\common\board.cc` | SỬA |
| 6 | Dừng nhạc khi rời idle | `main\application.cc` | SỬA |
| 7 | Tool `self.music.play_song`, `self.music.set_display_mode` + `#include <cctype>` | `main\mcp_server.cc` | SỬA |
| 8 | Thêm `boards/common/esp32_music.cc` vào SOURCES | `main\CMakeLists.txt` | SỬA |
| 9 | Thêm `chmorgan/esp-libhelix-mp3` | `main\idf_component.yml` | SỬA |
| 10 | Khai báo `MarkOutputActive()` | `main\audio\audio_service.h` | SỬA |
| 11 | Định nghĩa `MarkOutputActive()` | `main\audio\audio_service.cc` | SỬA |