#include "circular_buffer.h"
#include <utility>

CircularBuffer::CircularBuffer()
{
}

void CircularBuffer::setListener(std::weak_ptr<IListener> listener)
{
	listener_ = listener;
}

void CircularBuffer::bufferOverfloved()
{
	bufferPosition_ = 0;

	if (const auto listener = listener_.lock()) {
		auto nextBuffer = listener->onSentCallback();

		if (nextBuffer) {
			buffer_ = std::move(nextBuffer);
		}
	}
}

void CircularBuffer::halfCompleted()
{
	if (const auto listener = listener_.lock()) {
		auto nextBuffer = listener->onHalfSentCallback();

		if (nextBuffer) {
			buffer_ = std::move(nextBuffer);
		}
	}
}

int CircularBuffer::onHackRfCallback(hackrf_transfer* transfer)
{
	size_t bytes_to_read;

	bytes_to_read = transfer->valid_length;

	if (!buffer_)
	{
		memset(transfer->buffer, 0, bytes_to_read);
		bufferOverfloved();
	}
	else
	{
		size_t bufferLen = buffer_->size();
		int8_t* bufferData = buffer_->data();

		int position = 0;
		while (bytes_to_read > 0)
		{
			size_t remainBuf = bufferLen - bufferPosition_;
			size_t toRead = std::min(bytes_to_read, remainBuf);
			memcpy(&transfer->buffer[position], &bufferData[bufferPosition_], toRead);
			bool firstHalf = (bufferPosition_ < (bufferLen >> 1));
			bytes_to_read -= toRead;
			position += toRead;
			bufferPosition_ += toRead;

			if (firstHalf && (bufferPosition_ >= (bufferLen >> 1)))
				halfCompleted();

			if (bufferPosition_ >= bufferLen)
				bufferOverfloved();
		}
	}
	return 0;
}