#include "StringBox.h"
#include "ThreadRender.h"
#include "ThreadEvents.h"
#include "Logger.h"
#include "GL.h"

#include <glad/glad.h>

StringBox::StringBox() {
	pos = vec2(0, 0);
	size = vec2(0, 0);
	_change = true;
	_rbo = new gl::RenderBuffer();
	_fbo = new gl::FrameBuffer();
	uvec2 windowSize{events::ThreadEvents::getFramebufferSize()};
	_rbo->setFormat(GL_R8, windowSize.x, windowSize.y);
	_fbo->attach(_rbo, GL_COLOR_ATTACHMENT0);
}

void StringBox::render() {
	if (_change) {
		//render text here
		auto t = new gl::RenderTarget();
		t = gl::target->swapTarget(t);

		gl::target->activateFrameBuffer();
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

	}

	//use render texture here

}

std::string StringBox::str() const {
	return this->std::stringstream::str();
}

void StringBox::str(const std::string& string) {
	_change = true;
	this->std::stringstream::str(string);
}
