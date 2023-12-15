#include <tinet.h>
#include <string.h>
#include <fileioc.h>
#include <srldrvce.h>

char *username;
char *authkey;
uint8_t NetKeyAppVar;

srl_device_t srl_device;
uint8_t srl_buf[512];
bool has_srl_device = false;

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

        const srl_error_t error = srl_Open(&srl_device, device, srl_buf, sizeof srl_buf, SRL_INTERFACE_ANY, 9600);
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

int tinet_connect() {
    return TINET_SUCCESS;
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
    return totalBytesWritten;
}

int tinet_read_srl(char *to_buffer) {
    usb_HandleEvents();
    const int bytes_read = srl_Read(&srl_device, to_buffer, strlen(to_buffer) + 1);

    if (bytes_read < 0) {
        return TINET_SRL_READ_FAIL;
    }

    return bytes_read;
}
