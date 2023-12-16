/*
*--------------------------------------
 * Program Name: TINET lib
 * Author: TKB Studios
 * License: Mozilla Public License Version 2.0
 * Description: Allows the user to communicate with the TINET servers
 *--------------------------------------
*/

// TODO: implement login system
// TODO: Fix the serial receive not receiving all the data

#include "tinet-lib/tinet.h"
#include <string.h>
#include <fileioc.h>
#include <srldrvce.h>
#include <time.h>
#include <sys/timers.h>
#include <ti/info.h>

char *username;
char *authkey;
uint8_t NetKeyAppVar;

srl_device_t srl_device;
uint8_t srl_buf[512];
bool has_srl_device = false;
bool bridge_connected = false;
char tinet_net_buffer[2048];

const system_info_t *systemInfo;

static usb_error_t handle_usb_event(usb_event_t event, void *event_data, usb_callback_data_t *callback_data __attribute__((unused))) {
    usb_error_t err;

    if ((err = srl_UsbEventCallback(event, event_data, callback_data)) != USB_SUCCESS)
        return err;

    if(event == USB_DEVICE_CONNECTED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)) {
        const usb_device_t device = event_data;
        usb_ResetDevice(device);
    }

    if(event == USB_HOST_CONFIGURE_EVENT || (event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE))) {
        if(has_srl_device) return USB_SUCCESS;

        usb_device_t device;
        if(event == USB_HOST_CONFIGURE_EVENT) {
            device = usb_FindDevice(NULL, NULL, USB_SKIP_HUBS);
            if(device == NULL) return USB_SUCCESS;
        } else {
            device = event_data;
        }

        const srl_error_t error = srl_Open(&srl_device, device, srl_buf, sizeof srl_buf, SRL_INTERFACE_ANY, 115200);
        if(error) {
            printf("Error %d initting serial\n", error);
            return USB_SUCCESS;
        }

        has_srl_device = true;
    }

    if(event == USB_DEVICE_DISCONNECTED_EVENT) {
        const usb_device_t device = event_data;
        if(device == srl_device.dev) {
            srl_Close(&srl_device);
            has_srl_device = false;
        }
    }

    return USB_SUCCESS;
}

int tinet_init() {
    const system_info_t *systemInfo = os_GetSystemInfo();

    if (systemInfo->hardwareType2 != 9)
    {
        return TINET_UNSUPPORTED_CALC;
    }

    NetKeyAppVar = ti_Open("NETKEY", "r");
    if (NetKeyAppVar == 0) {
        return TINET_NO_KEYFILE;
    }

    uint8_t *data_ptr = ti_GetDataPtr(NetKeyAppVar);
    ti_Close(NetKeyAppVar);

    username = (char *)data_ptr;
    authkey = (char *)(data_ptr + strlen(username) + 1);

    const usb_error_t usb_error = usb_Init(handle_usb_event, NULL, srl_GetCDCStandardDescriptors(), USB_DEFAULT_INIT_FLAGS);

    if(usb_error) {
        usb_Cleanup();
        printf("usb init error %u\n", usb_error);
        return TINET_SRL_INIT_FAIL;
    }

    return TINET_SUCCESS;
}

char* tinet_get_username() {
    return username;
}

int tinet_connect(const int timeout) {
    time_t start_time;
    time_t current_time;
    time(&start_time);

    do {
        time(&current_time);
        if ((int)difftime(current_time, start_time) > timeout) {
            return TINET_TIMEOUT_EXCEEDED;
        }

        const TINET_ReturnCode read_result = tinet_read_srl(tinet_net_buffer);
        if (read_result > 0) {
            printf("read success\n");
            printf("%s\n", tinet_net_buffer);
            if (strcmp(tinet_net_buffer, "BRIDGE_CONNECTED\0") == 0) {
                printf("Bridge connected\n");
                bridge_connected = true;
            }

            msleep(200);

            const TINET_ReturnCode write_response = tinet_write_srl("CONNECT_TCP\0");
            if (write_response == TINET_SUCCESS) {
                printf("requested TCP sock open\n");
                return TINET_SUCCESS;
            }

            printf("TCP init failed\n");
            return TINET_TCP_INIT_FAILED;
        }

    } while (1);
}

srl_device_t tinet_get_srl_device() {
    return srl_device;
}

int tinet_write_srl(const char *message) {
    size_t totalBytesWritten = 0;
    const size_t messageLength = strlen(message);

    while (totalBytesWritten < messageLength) {
        const int bytesWritten = srl_Write(&srl_device, message, messageLength);

        if (bytesWritten < 0) {
            return TINET_SRL_WRITE_FAIL;
        }

        totalBytesWritten += bytesWritten;
    }

    usb_HandleEvents();
    return TINET_SUCCESS;
}

int tinet_read_srl(char *to_buffer) {
    const int bytes_read = srl_Read(&srl_device, to_buffer, 1024);
    to_buffer[bytes_read] = '\0';
    usb_HandleEvents();
    return bytes_read;
}

TINET_ReturnCode tinet_login() {
    if (!bridge_connected || !has_srl_device) {
        return TINET_NO_CONNECTION;
    }

    char calcidStr[sizeof(systemInfo->calcid) * 2 + 1];
    for (unsigned int i = 0; i < sizeof(systemInfo->calcid); i++)
    {
        sprintf(calcidStr + i * 2, "%02X", systemInfo->calcid[i]);
    }

    char login_msg[93];
    snprintf(login_msg, sizeof(login_msg), "LOGIN:%s:%s:%s", calcidStr, username, authkey);

    const TINET_ReturnCode write_result = tinet_write_srl(login_msg);
    if (write_result != TINET_SUCCESS) {
        return TINET_LOGIN_FAILED;
    }

    do {
        const int read_bytes = tinet_read_srl(tinet_net_buffer);
        if (read_bytes > 0) {
            printf("got data back\n");
            break;
        }
    } while (1);

    return TINET_SUCCESS;
}