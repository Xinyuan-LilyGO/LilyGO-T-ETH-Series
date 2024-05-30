/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdio.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "original/include/usb_stream.h"

class USB_STREAM {

public:
    //Public member variables for storing user-defined callback function and arguments
    void *_user_mic_frame_cb_arg = NULL;
    void *_user_frame_cb_arg = NULL;
    uvc_frame_callback_t *_user_frame_cb = NULL;
    mic_callback_t *_user_mic_frame_cb = NULL;
    typedef void (*StateChangeCallback)(usb_stream_state_t event, void *arg);

    /**
     * @brief Construct a new USB_STREAM object
     *
     */
    USB_STREAM();

    /**
     * @brief Destroy the USB_STREAM object
     *
     */
    ~USB_STREAM();

    /**
     * @brief Start usb streaming with pre-configs, usb driver will create internal tasks to handle usb data from stream pipe, and run user's callback after new frame ready.
     */
    void start();

    /**
     * @brief Stop current usb streaming, internal tasks will be delete, related resourse will be free
     */
    void stop();

    /*
     * @brief Wait for USB device connection
    */
    void connectWait(int timeoutMs);

    /**
     * @brief This function registers a callback for USB streaming, please note that only one callback
     *  can be registered, the later registered callback will overwrite the previous one.
     *
     * @param newFunction A pointer to a function that will be called when the USB streaming state changes.
     * @param usr_data usr_data is a void pointer.
     */
    void registerState(StateChangeCallback newFunction, void *usr_data);

    /**
     * @brief Register a callback fucntion to an object
     *
     * @param newFunction Callback function
     * @param cb_arg callback args
     */
    void uvcCamRegisterCb(uvc_frame_callback_t *newFunction, void *cb_arg);

    /**
     * @brief Configuration for an object
     *
     * @param width width of a frame
     * @param height height of a frame
     * @param frameInterval frame interval
     * @param transferBufferSize  Transfer buffer size, using double buffer here, must larger than one frame size
     * @param transferBufferA    buffer a for usb payload
     * @param transferBufferB Buffer b for usb payload
     * @param frameBufferSize Frame buffer size, must larger than one frame size
     * @param frameBuffer  Buffer for one frame
     */
    void uvcConfiguration(uint16_t width, uint16_t height, uint32_t frameInterval, uint32_t transferBufferSize, uint8_t *transferBufferA, uint8_t *transferBufferB, uint32_t frameBufferSize, uint8_t *frameBuffer);

    /**
     * @brief Suspends USB Camera streaming
     *
     * @param ctrl_value control value
     */
    void uvcCamSuspend(void *ctrl_value);

    /**
     * @brief Resumes USB Camera streaming
     *
     * @param ctrl_value control value
     */
    void uvcCamResume(void *ctrl_value);

    /**
     * @brief Get the frame size list of current connected camera
     *
     * @param frame_size the frame size list
     */
    uvc_frame_size_t *uvcCamGetFrameSize(uvc_frame_size_t *frame_size);

    /**
     * @brief Get the frame list size of current connected Cam
     *
     * @param frame_size the frame list size
     * @param frame_index current frame index
     */
    void uvcCamGetFrameListSize(size_t *frame_size, size_t *frame_index);

    /**
     * @brief Reset the expected frame size and frame interval, please reset when uvc streaming
     * in suspend state.The new configs will be effective after streaming resume.
     *
     * Note: frame_width and frame_height can be set to 0 at the same time, which means
     * no change on frame size.
     *
     * @param frame_width frame width, FRAME_RESOLUTION_ANY means any width
     * @param frame_height frame height, FRAME_RESOLUTION_ANY means any height
     * @param frame_interval frame interval, 0 means no change
     */
    void uvcCamFrameReset(uint16_t frame_width, uint16_t frame_height, uint32_t frame_interval);

    /**
     * @brief Suspends USB Mic streaming
     *
     * @param ctrl_value control value
     */
    void uacMicSuspend(void *ctrl_value);

    /**
     * @brief Resumes USB Mic streaming
     *
     * @param ctrl_value control value
     */
    void uacMicResume(void *ctrl_value);

    /**
     * @brief Mute USB Mic streaming
     *
     * @param ctrl_value control value
     */
    void uacMicMute(void *ctrl_value);

    /**
     * @brief  Control Mic volume
     *
     * @param ctrl_value control value
     */
    void uacMicVolume(void *ctrl_value);

    /**
     * @brief Get the frame size list of current connected Mic
     *
     * @param frame_size the frame size list
     */
    // uac_frame_size_t *uacMicGetFrameSize(size_t *frame_size);
    uac_frame_size_t *uacMicGetFrameSize(uac_frame_size_t *frame_size);

    /**
     * @brief Get the frame list size of current connected Mic
     *
     * @param frame_size the frame list size
     * @param frame_index current frame index
     */
    void uacMicGetFrameListSize(size_t *frame_size, size_t *frame_index);

    /**
      * @brief Configuration for UAC object
      *
      * @param mic_ch_num microphone channel numbers
      * @param mic_bit_resolution  microphone resolution(bits)
      * @param mic_samples_frequency microphone frequency(Hz)
      * @param mic_buf_size mic receive buffer size, 0 if not use
      */
    void uacConfiguration(uint8_t mic_ch_num, uint16_t mic_bit_resolution, uint32_t mic_samples_frequency, uint32_t mic_buf_size, uint8_t spk_ch_num, uint16_t spk_bit_resolution, uint32_t spk_samples_frequency, uint32_t spk_buf_size);

    /**
     * @brief Register a callback fucntion to an UAC Mic object
     *
     * @param newFunction Callback function
     * @param cb_arg callback args
     */
    void uacMicRegisterCb(mic_callback_t *newFunction, void *cb_arg);

    /**
     * @brief Read data from internal mic buffer, the actual size will be returned
     *
     * @param buffer pointer to the buffer to store the received data
     * @param buf_size The size of the data buffer.
     * @param data_bytes The actual size read from buffer
     * @param timeout_ms The timeout value for the read operation.
     */
    void uacReadMic(uint8_t *buffer, size_t buf_size, size_t *data_bytes, size_t timeout_ms);

    /**
     * @brief Reset Mic audio channel number, bit resolution and samples frequency, please reset when the streaming
     * in suspend state. The new configs will be effective after streaming resume.
     *
     * @param ch_num audio channel numbers
     * @param bit_resolution audio bit resolution
     * @param samples_frequency audio samples frequency
     */
    void uacMicFrameReset(uint8_t ch_num, uint16_t bit_resolution, uint32_t samples_frequency);

    /**
     * @brief  Control Mic volume
     *
     * @param ctrl_value control value
     */
    void uacSpkSuspend(void *ctrl_value);

    /**
     * @brief  Resumes USB Spk streaming
     *
     * @param ctrl_value control value
     */
    void uacSpkResume(void *ctrl_value);

    /**
     * @brief  Mute USB spk streaming
     * @param ctrl_value control value
     */
    void uacSpkMute(void *ctrl_value);

    /**
     * @brief  Control Spk volume
     *
     * @param ctrl_value control value
     */
    void uacSpkVolume(void *ctrl_value);

    /**
    * @brief Get the frame size list of current connected Spk
    *
    * @param frame_size the frame size list
    */
    uac_frame_size_t *uacSpkGetFrameSize(uac_frame_size_t *frame_size);

    /**
     * @brief Get the frame list size of current connected Spk
     *
     * @param frame_size the frame list size
     * @param frame_index current frame index
     */
    void uacSpkGetFrameListSize(size_t *frame_size, size_t *frame_index);

    /**
     * @brief Write data to the speaker buffer, will be send out when USB device is ready
     *
     * @param buffer The data to be written.
     * @param data_bytes The size of the data to be written.
     * @param timeout_ms The timeout value for writing data to the buffer.
     */
    void uacWriteSpk(uint16_t *buffer, size_t data_bytes, size_t timeout_ms);

    /**
     * @brief Reset Spk audio channel number, bit resolution and samples frequency, please reset when the streaming
     * in suspend state. The new configs will be effective after streaming resume.
     *
     * @param ch_num audio channel numbers
     * @param bit_resolution audio bit resolution
     * @param samples_frequency audio samples frequency
     */
    void uacSpkFrameReset(uint8_t ch_num, uint16_t bit_resolution, uint32_t samples_frequency);

private:
    uint16_t _frame_width;
    uint16_t _frame_height;
    uint32_t _frame_interval;
    uint8_t _spk_ch_num;
    uint16_t _spk_bit_resolution;
    uint32_t _spk_samples_frequency;
    uint32_t _spk_buf_size;
    uint8_t _mic_ch_num;
    uint16_t _mic_bit_resolution;
    uint32_t _mic_samples_frequency;
    uint32_t _mic_buf_size;
};
