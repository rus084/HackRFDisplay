#pragma once

#include "../libhackrfpp/hackrf_device.h"

#include <functional>
#include <vector>
#include <memory>

class CircularBuffer : public HackRFdevice::ICallbackListener
{
public:
	class IListener {
	public:
		virtual std::shared_ptr<std::vector<int8_t> > onSentCallback() = 0;
		virtual std::shared_ptr<std::vector<int8_t> > onHalfSentCallback() = 0;
	};

public:
	CircularBuffer();

	int onHackRfCallback(hackrf_transfer* transfer) override;

	void setListener(std::weak_ptr<IListener> listener);

private:
	size_t bufferPosition_ = 0;

	std::shared_ptr<std::vector<int8_t> > buffer_;

	std::weak_ptr<IListener> listener_;

	void bufferOverfloved();
	void halfCompleted();
};

