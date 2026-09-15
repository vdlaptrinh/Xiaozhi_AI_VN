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