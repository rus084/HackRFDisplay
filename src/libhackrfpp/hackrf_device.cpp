#include "hackrf_device.h"
#include <stdexcept>
#include <string>

namespace {
	void assertSuccess(int status) {
		hackrf_error error = static_cast<hackrf_error>(status);

		if (error != HACKRF_SUCCESS) {
			throw std::runtime_error(hackrf_error_name(error));
		}
	}
}

HackRFdevice::HackRFdevice() {
	assertSuccess(hackrf_init());
}

HackRFdevice::~HackRFdevice() {
	close();
}

void HackRFdevice::openAny() {
	assertSuccess(hackrf_open(&device));
}

void HackRFdevice::close() {
	if (device != nullptr)
	{
		assertSuccess(hackrf_close(device));

		device = nullptr;
	}
}

void HackRFdevice::set_sample_rate(const double freq_hz) {
	assertSuccess(hackrf_set_sample_rate(device, freq_hz));
}

void HackRFdevice::set_freq(const uint64_t freq_hz) {
	assertSuccess(hackrf_set_freq(device, freq_hz));
}

void HackRFdevice::amp_enable(bool en) {
	uint8_t amp_enable = (en) ? 1 : 0;
	assertSuccess(hackrf_set_amp_enable(device, amp_enable));
}

void HackRFdevice::set_txvga_gain(uint32_t value) {
	assertSuccess(hackrf_set_txvga_gain(device, value));
}

void HackRFdevice::set_lna_gain(uint32_t value) {
	assertSuccess(hackrf_set_lna_gain(device, value));
}

void HackRFdevice::set_vga_gain(uint32_t value) {
	assertSuccess(hackrf_set_vga_gain(device, value));
}

void HackRFdevice::transferAbort() {
	if (receive_) {
		assertSuccess(hackrf_stop_rx(device));
		receive_ = false;
	}

	if (transmit_) {
		assertSuccess(hackrf_stop_tx(device));
		transmit_ = false;
	}
}

void HackRFdevice::startTransmit()
{
	if (transmit_ || receive_) {
		throw std::logic_error("transfer in ongoing");
	}

	transmit_ = true;

	assertSuccess(hackrf_start_tx(device, hackRFdeviceCallback, this));
}

void HackRFdevice::startReceive()
{
	if (transmit_ || receive_) {
		throw std::logic_error("transfer in ongoing");
	}

	receive_ = true;

	assertSuccess(hackrf_start_rx(device, hackRFdeviceCallback, this));
}

void HackRFdevice::setCallbackListener(std::weak_ptr<ICallbackListener> callbackListener) {
	callbackListener_ = callbackListener;
}

int HackRFdevice::hackRFdeviceCallback(hackrf_transfer* transfer) {
	void* ctx = nullptr;

	if (transfer->rx_ctx)
	{
		ctx = transfer->rx_ctx;
	}
	else if (transfer->tx_ctx)
	{
		ctx = transfer->tx_ctx;
	}

	if (ctx == nullptr)
		return -1;

	HackRFdevice* self = reinterpret_cast<HackRFdevice*>(ctx);

	if (const auto callbackListener = self->callbackListener_.lock()) {
		return callbackListener->onHackRfCallback(transfer);
	}

	return -1;
}