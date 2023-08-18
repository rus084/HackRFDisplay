#include "hackrf_device_list.h"

HackRfDeviceList::HackRfDeviceList()
    : list(hackrf_device_list())
{
}

HackRfDeviceList::~HackRfDeviceList() {
    hackrf_device_list_free(list);
}

int HackRfDeviceList::size() const {
    return list->devicecount;
}

char** HackRfDeviceList::begin() const {
    return &list->serial_numbers[0];
}

char** HackRfDeviceList::end() const {
    return &list->serial_numbers[size()];
}

std::string HackRfDeviceList::operator[](int index) const {
    if (index >= size() || index < 0) {
        throw std::logic_error("index out of bounds");
    }

    return list->serial_numbers[index];
}
