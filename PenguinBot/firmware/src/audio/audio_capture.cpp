#include "audio_capture.h"
#include "../config.h"
#include <esp_timer.h>
#include <esp_log.h>

bool AudioCapture::_i2sInstalled = false;

bool AudioCapture::begin() {
    if (_i2sInstalled) {
        log_w("I2S already installed, skipping");
        return true;
    }

    // === I2S0 全双工配置 ===
    i2s_config_t i2s_cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = DMA_BUF_COUNT,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
        .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        .bits_per_chan = I2S_BITS_PER_CHAN_16BIT,
    };

    i2s_pin_config_t pin_cfg = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_BCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_DIN,
    };

    esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, nullptr);
    if (ret != ESP_OK) {
        log_e("I2S driver install failed: %d", ret);
        return false;
    }
    ret = i2s_set_pin(I2S_NUM_0, &pin_cfg);
    if (ret != ESP_OK) {
        log_e("I2S pin set failed: %d", ret);
        return false;
    }

    // INMP441 上电稳定需要 ~50ms
    delay(100);

    _i2sInstalled = true;
    _running = true;

    // 预留回放缓冲区
    _last_buf = (int16_t*)heap_caps_malloc(LAST_BUF_SAMPLES * sizeof(int16_t),
                                           MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!_last_buf) {
        _last_buf = (int16_t*)malloc(LAST_BUF_SAMPLES * sizeof(int16_t));
    }

    log_i("AudioCapture ready: INMP441 + MAX98357 @ %dHz full-duplex", AUDIO_SAMPLE_RATE);
    return true;
}

void AudioCapture::end() {
    if (_i2sInstalled) {
        i2s_driver_uninstall(I2S_NUM_0);
        _i2sInstalled = false;
    }
    if (_last_buf) { free(_last_buf); _last_buf = nullptr; }
    _running = false;
}

size_t AudioCapture::record(AudioBuffer& buf, uint32_t timeout_ms) {
    if (!_running) return 0;

    size_t max_samples = AUDIO_BUFFER_SAMPLES;
    if (buf.data == nullptr) return 0;

    size_t total = 0;
    size_t silent_samples = 0;
    const size_t silence_threshold = (AUDIO_SAMPLE_RATE * VAD_SILENCE_MS) / 1000;
    bool voice_started = false;

    uint32_t start = millis();
    int16_t local_buf[DMA_BUF_LEN];

    while (total < max_samples && (millis() - start) < timeout_ms) {
        size_t bytes_read = 0;
        esp_err_t ret = i2s_read(I2S_NUM_0, local_buf, sizeof(local_buf), &bytes_read, 10);
        if (ret != ESP_OK) continue;

        size_t samples = bytes_read / sizeof(int16_t);

        // 计算 RMS
        float sum_sq = 0;
        for (size_t i = 0; i < samples; i++) {
            sum_sq += (float)local_buf[i] * local_buf[i];
            if (total + i < max_samples) {
                buf.data[total + i] = local_buf[i];
            }
        }
        _rms = sqrt(sum_sq / samples);

        total += samples;

        // VAD 状态机
        if (_rms > VAD_ENERGY_THRESHOLD) {
            voice_started = true;
            silent_samples = 0;
        } else if (voice_started) {
            silent_samples += samples;
            if (silent_samples >= silence_threshold) {
                buf.voice_detected = true;
                break;  // 说完
            }
        }
    }

    buf.len = total;
    buf.timestamp_ms = millis();

    // 保存副本用于调试回放
    _last_len = min(total, (size_t)LAST_BUF_SAMPLES);
    if (_last_buf) memcpy(_last_buf, buf.data, _last_len * sizeof(int16_t));

    return total;
}

void AudioCapture::playbackLast() {
    if (!_last_buf || _last_len == 0) return;

    size_t written = 0;
    // I2S 输出
    i2s_write(I2S_NUM_0, _last_buf, _last_len * sizeof(int16_t), &written, portMAX_DELAY);
}
