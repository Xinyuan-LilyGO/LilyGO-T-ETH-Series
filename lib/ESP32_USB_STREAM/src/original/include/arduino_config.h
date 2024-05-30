/*
 * Automatically generated file. DO NOT EDIT.
 * Espressif IoT Development Framework (ESP-IDF)  #define CONFIGuration Header
 */
#pragma once
#define CONFIG_SAMPLE_PROC_TASK_STACK_SIZE 3072
#define CONFIG_SAMPLE_PROC_TASK_PRIORITY 2
#define CONFIG_SAMPLE_PROC_TASK_CORE 0
#define CONFIG_USB_PROC_TASK_STACK_SIZE 3072
#define CONFIG_USB_PROC_TASK_PRIORITY 5
#define CONFIG_USB_PROC_TASK_CORE 1
#define CONFIG_NUM_ISOC_STREAM_URBS 3
#define CONFIG_NUM_PACKETS_PER_URB_URB 4
#define CONFIG_UVC_GET_CONFIG_DESC 1
#define USB_STREAM_VER_MAJOR (1)  //version of USB Stream check .yml 
#define USB_STREAM_VER_MINOR (0)
#define USB_STREAM_VER_PATCH (4)
#define CONFIG_CTRL_TRANSFER_DATA_MAX_BYTES  1024       //Max data length assumed in control transfer
#define CONFIG_NUM_BULK_STREAM_URBS        2         //Number of bulk stream URBS created for continuous enqueue
#define CONFIG_NUM_BULK_BYTES_PER_URB     2048          //Required transfer bytes of each URB, check
#define CONFIG_NUM_ISOC_UVC_URBS           3         //Number of isochronous stream URBS created for continuous enqueue
#define CONFIG_NUM_PACKETS_PER_URB      4            //Number of packets in each isochronous stream URB
#define CONFIG_USB_WAITING_AFTER_CONN_MS    50        //Waiting n ms for usb device ready after connection
#define CONFIG_NUM_ISOC_SPK_URBS  6                  //Number of isochronous stream URBS created for continuous enqueue
#define CONFIG_NUM_ISOC_MIC_URBS  3                  //Number of isochronous stream URBS created for continuous enqueue
#define CONFIG_UAC_MIC_CB_MIN_MS_DEFAULT  16         //Default min ms for mic callback
#define CONFIG_UAC_SPK_ST_MAX_MS_DEFAULT    16        //Default max ms for speaker stream
#define CONFIG_UAC_SPK_VOLUME_LEVEL_DEFAULT  80        //Default volume level for speaker
#define CONFIG_UAC_MIC_VOLUME_LEVEL_DEFAULT  80        //Default volume level for mic
#define CONFIG_UAC_MIC_PACKET_COMPENSATION    1       //padding data if mic packet loss
#define CONFIG_UAC_SPK_PACKET_COMPENSATION       1    //padding zero if speaker buffer empty
#define CONFIG_UAC_SPK_PACKET_COMPENSATION_SIZE_MS 10  //padding n MS zero if speaker buffer empty
#define CONFIG_UAC_SPK_PACKET_COMPENSATION_TIMEOUT_MS 1000 //padding n MS zero if speaker buffer empty
#define CONFIG_USB_PRE_ALLOC_CTRL_TRANSFER_URB       1  //Pre-allocate URB for control transfer
#define CONFIG_UVC_GET_DEVICE_DESC  1
#define CONFIG_UVC_PRINT_DESC  1
#define CONFIG_USB_ENUM_FAILED_RETRY  1
#define CONFIG_USB_ENUM_FAILED_RETRY_COUNT 10
#define CONFIG_USB_ENUM_FAILED_RETRY_DELAY_MS 200
#define CONFIG_UVC_DROP_OVERFLOW_FRAME  1
#define CONFIG_UVC_PRINT_PROBE_RESULT  1
#define CONFIG_UVC_CHECK_BULK_JPEG_HEADER  1


#define CONFIG_CTRL_TRANSFER_DATA_MAX_BYTES 1024
#define CONFIG_UVC_GET_DEVICE_DESC  1
#define CONFIG_UVC_GET_CONFIG_DESC  1
#define CONFIG_UVC_PRINT_DESC  1
#define CONFIG_USB_PRE_ALLOC_CTRL_TRANSFER_URB  1
#define CONFIG_USB_PROC_TASK_PRIORITY 5
#define CONFIG_USB_PROC_TASK_CORE 1
#define CONFIG_USB_PROC_TASK_STACK_SIZE 3072
#define CONFIG_USB_WAITING_AFTER_CONN_MS 50
#define CONFIG_USB_ENUM_FAILED_RETRY  1
#define CONFIG_USB_ENUM_FAILED_RETRY_COUNT 10
#define CONFIG_USB_ENUM_FAILED_RETRY_DELAY_MS 200

#define CONFIG_SAMPLE_PROC_TASK_PRIORITY 2
#define CONFIG_SAMPLE_PROC_TASK_CORE 0
#define CONFIG_SAMPLE_PROC_TASK_STACK_SIZE 3072
#define CONFIG_UVC_PRINT_PROBE_RESULT  1
#define CONFIG_UVC_CHECK_BULK_JPEG_HEADER  1
#define CONFIG_UVC_DROP_OVERFLOW_FRAME  1
#define CONFIG_NUM_BULK_STREAM_URBS 2
#define CONFIG_NUM_BULK_BYTES_PER_URB 2048
#define CONFIG_NUM_ISOC_UVC_URBS 3
#define CONFIG_NUM_PACKETS_PER_URB 4

#define CONFIG_NUM_ISOC_SPK_URBS 6
#define CONFIG_NUM_ISOC_MIC_URBS 3
#define CONFIG_UAC_MIC_CB_MIN_MS_DEFAULT 16
#define CONFIG_UAC_SPK_ST_MAX_MS_DEFAULT 16
#define CONFIG_UAC_MIC_PACKET_COMPENSATION  1
#define CONFIG_UAC_SPK_PACKET_COMPENSATION  1
#define CONFIG_UAC_SPK_PACKET_COMPENSATION_TIMEOUT_MS 1000
#define CONFIG_UAC_SPK_PACKET_COMPENSATION_SIZE_MS 10
#define CONFIG_UAC_SPK_VOLUME_LEVEL_DEFAULT 80
#define CONFIG_UAC_MIC_VOLUME_LEVEL_DEFAULT 80

#define CONFIG_USB_CTRL_XFER_TIMEOUT_MS 1000