#include "framebuffer_base.h"
#include <memory>
#include <vector>

void FrameBufferBase::setListener(std::weak_ptr<IListener> listener) {
	listener_ = listener;
}
