#include <iostream>
#include <libusb-1.0/libusb.h>
#include <sstream>
#include <unordered_map>

using namespace std;

static const int serialNumberLength = 32;

void print_dev(libusb_device *dev);

static unordered_map<int, string> map = {
        {0,   "Code Missing"},
        {1,   "Audio device"},
        {2,   "Network adapter"},
        {3,   "User Interface Device"},
        {5,   "Physical Device"},
        {6,   "Images"},
        {7,   "Printer"},
        {8,   "Storage"},
        {9,   "Concentrator"},
        {10,  "CDC-Data"},
        {11,  "Smart Card"},
        {13,  "Content Security"},
        {14,  "Video device"},
        {15,  "Personal Medical Device"},
        {16,  "Audio and video devices"},
        {220, "Diagnostic Device"},
        {224, "Wireless Controller"},
        {239, "Various Devices"},
        {254, "Specific device"},
};

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
    // get a list of all found USB devices
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        cerr << "Error: USB device list not received." << endl;
        return 1;
    }

    for (int i = 0; i < cnt; i++) {
        print_dev(devs[i]);
    }
    // release the memory allocated by the get device list function
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx); // close session,
    return 0;
}

// https://www.keil.com/pack/doc/mw/USB/html/_u_s_b__interface__descriptor.html
// https://libusb.sourceforge.io/api-1.0
void print_dev(libusb_device *dev) {
    libusb_device_descriptor desc{};
    libusb_config_descriptor *config;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
        cerr << "Error: Device handle not received, code: " << r << endl;
        return;
    }
    libusb_get_config_descriptor(dev, 0, &config);
    cout << "Device class: " << map[static_cast<int>(desc.bDeviceClass)] << endl;
    cout << "Vendor id: " << hex << desc.idVendor << endl;
    cout << "Product id: " << desc.idProduct << endl;

    libusb_device_handle *handle;
    auto *data = new uint8_t[33]();
    try {
        libusb_open(dev, &handle);
        if (handle != nullptr) {
            if (libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, data, serialNumberLength - 1) >= 0) {
                data[serialNumberLength] = '\0';
                cout << "Serial number: " << data << endl;
            }
        }
    } catch (libusb_error &e) {
        cerr << e << endl;
    }
    delete[] data;
    if (handle) {
        try {
            libusb_close(handle);
        } catch (libusb_error &e) {
            cerr << e << endl;
        }
    }
    cout << endl;
    libusb_free_config_descriptor(config);
}