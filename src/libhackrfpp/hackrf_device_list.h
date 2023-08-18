#pragma once

#include "hackrf.h"

#include <stdexcept>
#include <string>

class HackRfDeviceList
{
public:
	HackRfDeviceList();

	~HackRfDeviceList();

	int size() const;

    char** begin() const;

    char** end() const;

	std::string operator[](int index) const;

private:
	hackrf_device_list_t* list;
};