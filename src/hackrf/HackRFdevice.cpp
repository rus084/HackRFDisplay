#include "HackRFdevice.h"
#include <string>


extern "C"
{
	static int HackRFdeviceCAllback(hackrf_transfer* transfer);
}

static int HackRFdeviceCallback(hackrf_transfer* transfer)
{
	void* ctx = nullptr;
	//if (transfer->rx_ctx)
	//{
	//	ctx = transfer->rx_ctx;
	//}
	//else if (transfer->tx_ctx)
	{
		ctx = transfer->tx_ctx;
	}

	if (ctx == nullptr)
		return -1;

	std::function<int(hackrf_transfer*)>* callback = (std::function<int(hackrf_transfer*)>*)ctx;
	return (*callback)(transfer);
}

HackRFdevice::HackRFdevice()
{
	int result = HACKRF_SUCCESS;
	result = hackrf_init();

	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}

}

HackRFdevice::~HackRFdevice()
{
	Close();
}

void HackRFdevice::OpenAny()
{
	int result = HACKRF_SUCCESS;
	result = hackrf_open(&device);

	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::Close()
{
	if (device != nullptr)
	{
		int result = HACKRF_SUCCESS;
		result = hackrf_close(device);

		device = nullptr;

		if (result != HACKRF_SUCCESS) {
			throw std::string(hackrf_error_name((enum hackrf_error)result));
		}
	}
}

void HackRFdevice::set_sample_rate(const double freq_hz)
{
	int result = HACKRF_SUCCESS;
	result = hackrf_set_sample_rate(device, freq_hz);

	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::set_freq(const uint64_t freq_hz)
{
	int result = HACKRF_SUCCESS;
	result = hackrf_set_freq(device, freq_hz);
	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::amp_enable(bool en)
{
	uint8_t amp_enable = (en) ? 1 : 0;
	int result = HACKRF_SUCCESS;
	result = hackrf_set_amp_enable(device, (uint8_t)amp_enable);
	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::set_txvga_gain(uint32_t value)
{
	int result = HACKRF_SUCCESS;
	result = hackrf_set_txvga_gain(device, value);
	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::set_lna_gain(uint32_t value)
{
	int result = HACKRF_SUCCESS;
	result = hackrf_set_lna_gain(device, value);
	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::set_vga_gain(uint32_t value)
{
	int result = HACKRF_SUCCESS;
	result = hackrf_set_vga_gain(device, value);
	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::transferAbort()
{
	int result = HACKRF_SUCCESS;
	result = hackrf_stop_rx(device);

	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::startTransmit(std::function<int(hackrf_transfer*)>* callback)
{
	int result = HACKRF_SUCCESS;
	result = hackrf_start_tx(device, HackRFdeviceCallback, callback);
	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}

void HackRFdevice::startReceive(std::function<int(hackrf_transfer*)>* callback)
{
	int result = HACKRF_SUCCESS;
	result = hackrf_start_rx(device, HackRFdeviceCallback, callback);
	if (result != HACKRF_SUCCESS) {
		throw std::string(hackrf_error_name((enum hackrf_error)result));
	}
}