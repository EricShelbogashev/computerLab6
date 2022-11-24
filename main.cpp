#include <iostream>
#include <libusb-1.0/libusb.h>
#include <cstdio>
#include <unordered_map>

using namespace std;

int main() {
    libusb_device **devs;
    libusb_context *ctx = nullptr; // libusb session context
    int r; // for results
    ssize_t cnt; // number of usb devices found
    r = libusb_init(&ctx); // open session
    if (r < 0) {
        cerr << "Error: initialization failed: " << r << endl;
        return 1;
    }
    // set the verbosity level of debug messages
    libusb_set_debug(ctx, 3);
    // get a list of all found USB devices
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        cerr << "Error: USB device list not received." << endl;
        return 1;
    }
    libusb_device_descriptor desc{};
    for (int i = 0; i < cnt; i++) {
        libusb_device_handle *handle;
        auto *data = new uint8_t[33]();
        r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0) {
            cerr << "Error: Device handle not received, code: " << r << endl;
        }
        printf("%02X\t%02X:%02X  \t",desc.bDeviceClass, desc.idVendor, desc.idProduct);
        try {
            libusb_open(devs[i], &handle);
            if (handle != nullptr) {
                if (libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, data, 31) >= 0) {
                    data[32] = '\0';
                    cout << "Serial Number: \"" << data << "\"";
                }
            }
            libusb_close(handle);
        } catch (libusb_error &e) {
            cerr << e << endl;
        }
        cout << endl;
    }
    // release the memory allocated by the get device list function
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx); // close session,
    return 0;
}

// https://www.keil.com/pack/doc/mw/USB/html/_u_s_b__interface__descriptor.html
// https://libusb.sourceforge.io/api-1.0