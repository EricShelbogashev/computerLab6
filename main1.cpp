#include <iostream>
#include <libusb-1.0/libusb.h>
#include <cstdio>
#include <sstream>
#include <unordered_map>

using namespace std;

void printdev(libusb_device *dev);

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
    printf("Devices found: %d\n\n", cnt);
    printf("Device class\n");
    printf("│\t \tVendor id\n");
    printf("│\t \t│\tAlternate setting number \n");
    printf("│\t \t│\t│\tProduct id, endpoint descriptor\n");
    printf("│\t \t│\t│\t│\tEndpoint address\n");
    for (int i = 0; i < cnt; i++) {
        printdev(devs[i]);
    }
    // release the memory allocated by the get device list function
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx); // close session,
    return 0;
}

void printTreeComp(int steps, bool isLast = false) {
    std::stringstream ss;
    ss << "\n";
    std::string step = "\t ";
    for (int i = 0; i < steps; ++i) {
        ss << step;
    }
    if (isLast) {
        ss << "└─\t";
    } else {
        ss << "├─\t";
    }
    cout << ss.str();
}

// https://www.keil.com/pack/doc/mw/USB/html/_u_s_b__interface__descriptor.html
// https://libusb.sourceforge.io/api-1.0
void printdev(libusb_device *dev) {
    libusb_device_descriptor desc{};
    libusb_config_descriptor *config;
    const libusb_interface *inter;
    const libusb_interface_descriptor *interdesc;
    const libusb_endpoint_descriptor *epdesc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
        cerr << "Error: Device handle not received, code: " << r << endl;
        return;
    }
    libusb_get_config_descriptor(dev, 0, &config);
    printf("[%02Xh]\t[%02X]\t[%02X]\t", (int) desc.bDeviceClass, desc.idVendor, desc.idProduct);

    libusb_device_handle *handle;
    auto *data = new uint8_t[33]();
    try {
        libusb_open(dev, &handle);
        if (handle != nullptr) {
            if (libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, data, 31) >= 0) {
                data[32] = '\0';
                cout << "Serial Number: \"" << data << "\"";
            }
        }
    } catch (libusb_error &e) {
        cerr << e << endl;
    }
    for (int i = 0; i < (int) config->bNumInterfaces; i++) {
        printTreeComp(2, i == config->bNumInterfaces - 1);
        inter = &config->interface[i];
        printf("%.2d\t", inter->num_altsetting);

        for (int j = 0; j < inter->num_altsetting; j++) {
            interdesc = &inter->altsetting[j];

            printTreeComp(3, j == inter->num_altsetting - 1);
            printf("%.2d\t\t", (int) interdesc->bInterfaceNumber);

            for (int k = 0; k < (int) interdesc->bNumEndpoints; k++) {
                epdesc = &interdesc->endpoint[k];
                printTreeComp(4, k == interdesc->bNumEndpoints - 1);
                printf("%.2d\t", (int) epdesc->bEndpointAddress);
            }
        }
    }
    if (handle) {
        try {
            libusb_close(handle);
        } catch (libusb_error &e) {
            cerr << e << endl;
        }
    }
    cout << endl << endl;
    libusb_free_config_descriptor(config);
}