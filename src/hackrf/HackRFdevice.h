#pragma once

#include <hackrf.h>
#include <functional>

#define FD_BUFFER_SIZE (8*1024)

#define FREQ_ONE_MHZ (1000000ll)

#define DEFAULT_FREQ_HZ (900000000ll) /* 900MHz */
#define FREQ_MIN_HZ	(0ull) /* 0 Hz */
#define FREQ_MAX_HZ	(7250000000ll) /* 7250MHz */
#define IF_MIN_HZ (2150000000ll)
#define IF_MAX_HZ (2750000000ll)
#define LO_MIN_HZ (84375000ll)
#define LO_MAX_HZ (5400000000ll)
#define DEFAULT_LO_HZ (1000000000ll)

#define DEFAULT_SAMPLE_RATE_HZ (10000000) /* 10MHz default sample rate */

#define DEFAULT_BASEBAND_FILTER_BANDWIDTH (5000000) /* 5MHz default */

#define SAMPLES_TO_XFER_MAX (0x8000000000000000ull) /* Max value */

#define BASEBAND_FILTER_BW_MIN (1750000)  /* 1.75 MHz min value */
#define BASEBAND_FILTER_BW_MAX (28000000) /* 28 MHz max value */


class HackRfdeviceList
{
public:
	HackRfdeviceList() 
	{
		list = hackrf_device_list();
	}

	~HackRfdeviceList()
	{
		hackrf_device_list_free(list);
	}

	int count() {
		return list->devicecount;
	}

	char* getSN(int index)
	{
		if (index >= count())
			return nullptr;
		return list->serial_numbers[index];
	}



private:
	hackrf_device_list_t* list;
};

class HackRFdevice
{
public:
	HackRFdevice();
	~HackRFdevice();
	void OpenAny();
	void Close();

	void set_sample_rate(const double freq_hz);
	void set_freq(const uint64_t freq_hz);
	void amp_enable(bool en);
	void set_txvga_gain(uint32_t value);
	void set_lna_gain(uint32_t value);
	void set_vga_gain(uint32_t value);
	
	void startTransmit(std::function<int(hackrf_transfer*)>* callback);
	void startReceive(std::function<int(hackrf_transfer*)>* callback);
	void transferAbort();
private:
	hackrf_device* device = nullptr;
};

