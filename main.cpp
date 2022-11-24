#include <iostream>
#include <libusb-1.0/libusb.h>
#include <sstream>
#include <unordered_map>

namespace serial {
    const int serialNumberLength = 32;
    std::unordered_map<int, std::string> map = {
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
}

void print_dev(libusb_device *dev);

int main() {

    libusb_device **devs;
    libusb_context *ctx = nullptr; // libusb session context
    int r; // for results
    ssize_t cnt; // number of usb devices found
    r = libusb_init(&ctx); // open session
    if (r < 0) {
        std::cerr << "Error: initialization failed: " << r << std::endl;
        return 1;
    }
    // get a list of all found USB devices
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        std::cerr << "Error: USB device list not received." << std::endl;
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
        std::cerr << "Error: Device handle not received, code: " << r << std::endl;
        return;
    }
    libusb_get_config_descriptor(dev, 0, &config);
    std::cout << "Device class: " << serial::map[static_cast<int>(desc.bDeviceClass)] << std::endl;
    std::cout << "Vendor id: " << std::hex << desc.idVendor << std::endl;
    std::cout << "Product id: " << desc.idProduct << std::endl;

    libusb_device_handle *handle;
    auto *data = new uint8_t[33]();
    try {
        libusb_open(dev, &handle);
        if (handle != nullptr) {
            if (libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, data, serial::serialNumberLength - 1) >= 0) {
                data[serial::serialNumberLength] = '\0';
                std::cout << "Serial number: " << data << std::endl;
            }
        }
    } catch (libusb_error &e) {
        std::cerr << e << std::endl;
    }
    delete[] data;
    if (handle) {
        try {
            libusb_close(handle);
        } catch (libusb_error &e) {
            std::cerr << e << std::endl;
        }
    }
    std::cout << std::endl;
    libusb_free_config_descriptor(config);
}