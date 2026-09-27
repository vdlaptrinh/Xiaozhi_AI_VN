XIAOZHI_ENABLE_NEWS_TOOL (mặc định y)
XIAOZHI_ENABLE_ON_THIS_DAY_TOOL (y)
XIAOZHI_ENABLE_SCHEDULE_TOOL (n)
XIAOZHI_ENABLE_SD_MUSIC_TOOL (y)
XIAOZHI_ENABLE_ONLINE_MUSIC_TOOL (n)
XIAOZHI_ENABLE_ENGLISH_TOOL (n)

Dưới đây là tổng hợp các MCP tool của project (thiết bị XiaoZhi) — nhóm tool mà bên server/trợ lý AI gọi được qua ứng dụng (feature mcp: true). Với board đang dùng (freenove-esp32s3-display-2.8-lcd), các tool có hiệu lực là:

Nhóm hệ thống / audio / màn hình (mcp_server.cc, AddCommonTools)

self.get_device_status — trạng thái loa, màn, pin, mạng…
self.audio_speaker.set_volume — chỉnh volume (0–100)
self.screen.set_brightness — độ sáng (nếu có backlight)
self.screen.set_theme — đổi theme light/dark (nếu có LVGL)

Nhóm nhạc từ thẻ nhớ (SD music)

self.music.play_from_sd — phát từ /sdcard/music, tìm theo tên (không phân biệt dấu)
self.music.sd_control — next / prev / pause / resume / stop
(Đã tắt bằng #if 0): self.music.play_song, self.music.set_display_mode — nhạc online Esp32Music

Nhóm tin tức / lịch sử / lịch công tác

self.news.get_latest — 5 tin mới nhất (VnExpress)
self.news.get_detail — chi tiết tin theo index
self.history.on_this_day — sự kiện "ngày này năm xưa"
self.schedule.get_week / self.schedule.get_day — lịch công tác

Nhóm luyện nghe tiếng Anh (english/english_mcp.cc — riêng của feature này)

self.english.get_status — trạng thái hôm nay (streak, goal, bài hiện tại, % tiến độ)
self.english.get_next_lesson — bài nghe tiếp theo
self.english.play_lesson — phát bài theo lesson_id
self.english.stop_lesson — dừng bài (dưới ngưỡng sẽ không tính hoàn thành)
self.english.get_progress — tiến trình dài hạn
self.english.get_lesson_list — danh sách bài (offset/limit)
self.english.set_daily_goal — đặt mục tiêu bài/ngày (1–10)

Tool chỉ user (AddUserOnlyTools, do user gọi trực tiếp, không cho LLM)

self.get_system_info, self.reboot, self.upgrade_firmware
self.screen.get_info, self.screen.snapshot, self.screen.preview_image
self.assets.set_download_url