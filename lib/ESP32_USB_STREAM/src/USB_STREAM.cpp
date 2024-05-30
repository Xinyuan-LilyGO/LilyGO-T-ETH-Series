/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "USB_STREAM.h"
#include "original/include/usb_stream.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

// Define the static variable TAG with a string "driver"
static const char *TAG = "arduino-usb";

#define CHECK_ESP_ERROR(result, message) \
    if (result != ESP_OK) { \
        ESP_LOGE(TAG, "%s(%d): %s, Error Code: %d", __FUNCTION__, __LINE__, message, result); \
    }

// A constructor with default values
USB_STREAM::USB_STREAM()
{
    _frame_width = FRAME_RESOLUTION_ANY;
    _frame_height = FRAME_RESOLUTION_ANY;
    _frame_interval = FPS2INTERVAL(15);
    _spk_ch_num = UAC_CH_ANY;
    _spk_bit_resolution = UAC_BITS_ANY;
    _spk_samples_frequency = UAC_FREQUENCY_ANY;
    _spk_buf_size = 6400;
    _mic_ch_num = UAC_CH_ANY;
    _mic_bit_resolution = UAC_BITS_ANY;
    _mic_samples_frequency = UAC_FREQUENCY_ANY;
    _mic_buf_size = 6400;
}

// A destructor with default values
USB_STREAM::~USB_STREAM()
{}

// Method to register a user-defined callback function
void USB_STREAM::uvcCamRegisterCb(uvc_frame_callback_t *newFunction, void *cb_arg)
{
    if (newFunction == NULL) {
        ESP_LOGE(TAG, "registerCallBack function error\n");
        return;
    } else {
        this->_user_frame_cb = newFunction;
        this->_user_frame_cb_arg = cb_arg;
    }
}

// A static function that serves as the callback function for the camera frame
static void _camera_frame_cb(uvc_frame_t *frame, void *ptr)
{
    USB_STREAM *my_instance = (USB_STREAM *)ptr;
    if (my_instance->_user_frame_cb != NULL) {
        my_instance->_user_frame_cb(frame, my_instance->_user_frame_cb_arg);
    }
}

// Method to register a user-defined callback function
void USB_STREAM::uacMicRegisterCb(mic_callback_t *newFunction, void *cb_arg)
{
    if (newFunction == NULL) {
        ESP_LOGE(TAG, "registerCallBack function error\n");
        return;
    } else {
        this->_user_mic_frame_cb = newFunction;
        this->_user_mic_frame_cb_arg = cb_arg;
    }
}

// A static function that serves as the callback function for the camera frame
static void _mic_frame_cb(mic_frame_t *frame, void *ptr)
{
    USB_STREAM *my_instance = (USB_STREAM *)ptr;
    if (my_instance->_user_mic_frame_cb != NULL) {
        my_instance->_user_mic_frame_cb(frame, my_instance->_user_frame_cb_arg);
    }
}

// Method to configure the uvc camera stream
void USB_STREAM::uvcConfiguration(uint16_t width, uint16_t height, uint32_t frameInterval, uint32_t transferBufferSize, uint8_t *transferBufferA, uint8_t *transferBufferB, uint32_t frameBufferSize, uint8_t *frameBuffer)
{
    if (transferBufferA == nullptr || transferBufferB == nullptr || frameBuffer == nullptr) {
        ESP_LOGE(TAG, "arguments cannot be null");
        return;
    }

    _frame_width = width;
    _frame_height = height;
    _frame_interval = frameInterval;

    uvc_config_t uvc_config = {
        .frame_width = _frame_width,
        .frame_height = _frame_height,
        .frame_interval = _frame_interval,
        .xfer_buffer_size = transferBufferSize,
        .xfer_buffer_a = transferBufferA,
        .xfer_buffer_b = transferBufferB,
        .frame_buffer_size = frameBufferSize,
        .frame_buffer = frameBuffer,
        .frame_cb = &_camera_frame_cb,
        .frame_cb_arg = this,
    };
    // Configure the UVC streaming with the provided configuration
    CHECK_ESP_ERROR(uvc_streaming_config(&uvc_config), "UVC streaming config fail");
}

// Method to configure the uac mic stream
void USB_STREAM::uacConfiguration(uint8_t mic_ch_num, uint16_t mic_bit_resolution, uint32_t mic_samples_frequency, uint32_t mic_buf_size, uint8_t spk_ch_num, uint16_t spk_bit_resolution, uint32_t spk_samples_frequency, uint32_t spk_buf_size)
{

    _mic_ch_num = mic_ch_num;
    _mic_bit_resolution = mic_bit_resolution;
    _mic_samples_frequency = mic_samples_frequency;
    _mic_buf_size = mic_buf_size;

    _spk_ch_num = spk_ch_num;
    _spk_bit_resolution = spk_bit_resolution;
    _spk_samples_frequency = spk_samples_frequency;
    _spk_buf_size = spk_buf_size;

    uac_config_t uac_config = {
        .spk_ch_num = _spk_ch_num,
        .mic_ch_num = _mic_ch_num,
        .mic_bit_resolution = _mic_bit_resolution,
        .mic_samples_frequence = _mic_samples_frequency,
        .spk_bit_resolution = _spk_bit_resolution,
        .spk_samples_frequence = _spk_samples_frequency,
        .spk_buf_size = _spk_buf_size,
        .mic_buf_size = _mic_buf_size,
        .mic_cb = &_mic_frame_cb,
        .mic_cb_arg = this,
    };
    CHECK_ESP_ERROR(uac_streaming_config(&uac_config), "UAC streaming config fail");
}

// Method to start the USB streaming
void USB_STREAM::start()
{
    CHECK_ESP_ERROR(usb_streaming_start(), "USB streaming start fail");
}

// Method to stop the USB streaming
void USB_STREAM::stop()
{
    CHECK_ESP_ERROR(usb_streaming_stop(), "USB streaming stop fail");
}

// Method to wait for usb stream to connect
void USB_STREAM::connectWait(int timeoutMs)
{
    CHECK_ESP_ERROR(usb_streaming_connect_wait(timeoutMs), "USB streaming wait fail");
}

// Method to register state for usb stream
void USB_STREAM::registerState(StateChangeCallback newFunction, void *userData)
{
    if (!newFunction) {
        ESP_LOGE(TAG, "Callback function not defined");
        return;
    }
    // Register the provided callback function
    CHECK_ESP_ERROR(usb_streaming_state_register(newFunction, userData), "state register fail");
}

// Method to suspend uvc camera stream
void USB_STREAM::uvcCamSuspend(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UVC, CTRL_SUSPEND, ctrl_value), "uvc camera suspend fail");
}

// Method to resume uvc camera stream
void USB_STREAM::uvcCamResume(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UVC, CTRL_RESUME, ctrl_value), "uvc camera resume fail");
}

// Method to suspend uac mic stream
void USB_STREAM::uacMicSuspend(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_MIC, CTRL_SUSPEND, ctrl_value), "uac mic suspend fail");
}

// Method to resume uac mic stream
void USB_STREAM::uacMicResume(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_MIC, CTRL_RESUME, ctrl_value), "uac mic resume fail");
}

// Method to mute uac mic
void USB_STREAM::uacMicMute(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_MIC, CTRL_UAC_MUTE, ctrl_value), "uac mic mute fail");
}

// Method to adjust uac mic volume
void USB_STREAM::uacMicVolume(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_MIC, CTRL_UAC_VOLUME, ctrl_value), "uac mic volume fail");
}

// Method to suspend uac spk stream
void USB_STREAM::uacSpkSuspend(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_SPK, CTRL_SUSPEND, ctrl_value), "uac spk suspend fail");
}

// Method to resume uac spk stream
void USB_STREAM::uacSpkResume(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_SPK, CTRL_RESUME, ctrl_value), "uac spk resume fail");
}

// Method to mute uac spk
void USB_STREAM::uacSpkMute(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_MIC, CTRL_UAC_MUTE, ctrl_value), "uac spk mute fail");
}

// Method to adjust uac spk volume
void USB_STREAM::uacSpkVolume(void *ctrl_value)
{
    CHECK_ESP_ERROR(usb_streaming_control(STREAM_UAC_MIC, CTRL_UAC_VOLUME, ctrl_value), "uac spk volume fail");
}

// Method to get uvc frame size
uvc_frame_size_t *USB_STREAM::uvcCamGetFrameSize(uvc_frame_size_t *uvc_frame_list)
{
    if (uvc_frame_list == nullptr) {
        return NULL;
    }
    CHECK_ESP_ERROR(uvc_frame_size_list_get(uvc_frame_list, NULL, NULL), "uvc cam get frame size fail");
    return uvc_frame_list;
}

// Method to get uvc frame list size
void USB_STREAM::uvcCamGetFrameListSize(size_t *frame_size, size_t *frame_index)
{
    if (frame_size == nullptr || frame_index == nullptr) {
        ESP_LOGE(TAG, "arguments cannot be null");
        return;
    }
    CHECK_ESP_ERROR(uvc_frame_size_list_get(nullptr, frame_size, frame_index), "get frame list size fail");
}

// Method to reset uvc cam frame
void USB_STREAM::uvcCamFrameReset(uint16_t frame_width, uint16_t frame_height, uint32_t frame_interval)
{

    if (frame_width == NULL || frame_height == NULL || frame_interval == NULL) {
        ESP_LOGE(TAG, "arguments cannot be null");
        return;
    }
    CHECK_ESP_ERROR(uvc_frame_size_reset(frame_width, frame_height, frame_interval), "reset camera frame size fail");
}

// Method to read mic
void USB_STREAM::uacReadMic(uint8_t *buffer, size_t buf_size, size_t *data_bytes, size_t timeout_ms)
{
    if (buffer == nullptr || data_bytes == nullptr || buf_size == 0) {
        ESP_LOGE(TAG, "Invalid parameters for uacReadMic");
        return;
    }
    CHECK_ESP_ERROR(uac_mic_streaming_read(buffer, buf_size, data_bytes, timeout_ms), "read mic data error");
}

// Method to get uac spk frame size
uac_frame_size_t *USB_STREAM::uacSpkGetFrameSize(uac_frame_size_t *uac_frame_list)
{
    if (uac_frame_list == nullptr) {
        return NULL;
    }
    CHECK_ESP_ERROR(uac_frame_size_list_get(STREAM_UAC_SPK, uac_frame_list, NULL, NULL), "uac spk get frame size fail");
    return uac_frame_list;
}

// Method to get uac mic frame size
uac_frame_size_t *USB_STREAM::uacMicGetFrameSize(uac_frame_size_t *uac_frame_list)
{
    if (uac_frame_list == nullptr) {
        return NULL;
    }
    CHECK_ESP_ERROR(uac_frame_size_list_get(STREAM_UAC_MIC, uac_frame_list, NULL, NULL), "uac mic get frame size fail");
    return uac_frame_list;
}

// Method to get uac frame list size
void USB_STREAM::uacMicGetFrameListSize(size_t *frame_size, size_t *frame_index)
{
    if (frame_size == nullptr || frame_index == nullptr) {
        ESP_LOGE(TAG, "arguments cannot be null");
        return;
    }
    CHECK_ESP_ERROR(uac_frame_size_list_get(STREAM_UAC_MIC, nullptr, frame_size, frame_index), "get frame list size fail");
}

// Method to get uac spk frame list size
void USB_STREAM::uacSpkGetFrameListSize(size_t *frame_size, size_t *frame_index)
{
    if (frame_size == nullptr || frame_index == nullptr) {
        ESP_LOGE(TAG, "arguments cannot be null");
        return;
    }
    CHECK_ESP_ERROR(uac_frame_size_list_get(STREAM_UAC_MIC, nullptr, frame_size, frame_index), "get frame list size fail");
}

// Method to reset uac mic frame
void USB_STREAM::uacMicFrameReset(uint8_t ch_num, uint16_t bit_resolution, uint32_t samples_frequency)
{
    if (ch_num == NULL || bit_resolution == NULL || samples_frequency == NULL) {
        ESP_LOGE(TAG, "arguments cannot be null");
        return;
    }
    CHECK_ESP_ERROR(uac_frame_size_reset(STREAM_UAC_MIC, ch_num, bit_resolution, samples_frequency), "reset Mic frame size fail");
}

// Method to reset uac spk frame
void USB_STREAM::uacSpkFrameReset(uint8_t ch_num, uint16_t bit_resolution, uint32_t samples_frequency)
{
    if (ch_num == NULL || bit_resolution == NULL || samples_frequency == NULL) {
        ESP_LOGE(TAG, "arguments cannot be null");
        return;
    }
    CHECK_ESP_ERROR(uac_frame_size_reset(STREAM_UAC_SPK, ch_num, bit_resolution, samples_frequency), "reset Spk frame size fail");
}

// Method to write uac frame
void USB_STREAM::uacWriteSpk(uint16_t *buffer, size_t data_bytes, size_t timeout_ms)
{
    if (buffer == nullptr || data_bytes == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for uacWriteSpk");
        return;
    }
    CHECK_ESP_ERROR(uac_spk_streaming_write(buffer, data_bytes, timeout_ms), "write spk data error");
}
