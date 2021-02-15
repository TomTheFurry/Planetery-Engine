#include "StringBox.h"
#include "Logger.h"
#include "GL.h"
#include "Font.h"
#include "UTF.h"

#include <glad/glad.h>

static gl::RenderTarget* _targetText = nullptr;

StringBox::StringBox() : _ss() {
	pos = vec2(0, 0);
	size = vec2(0, 0);
	_ppi = vec2(0, 0);
	_textureSize = vec2(0, 0);
	_pointSize = 12.f;
	_change = true;
	_isLineCentre = false;
	_tex = nullptr;
	_fbo = nullptr;
}

void StringBox::render() {
	if (_change) {
		//logger("ReRendering...\n");
		//render text here
		_ppi = gl::target->pixelPerInch;
		_textureSize = vec2(gl::target->viewport.z, gl::target->viewport.w)*size;
		if (_textureSize.x<=0 || _textureSize.y<=0) {
			_change = false;
			return;
		}
		
		if (_targetText==nullptr) {
			_targetText = new gl::RenderTarget();
		}
		if (_fbo == nullptr)
			_fbo = new gl::FrameBuffer();
		_targetText->bind(_fbo);
		_targetText->viewport = vec4(vec2(0.f), _textureSize);
		_targetText->pixelPerInch = _ppi;
		if (_tex != nullptr)
			_tex->release();
		_tex = new gl::Texture2D();
		_tex->setFormat(GL_R8, _textureSize.x, _textureSize.y, 1);
		_tex->setTextureSamplingFilter(gl::Texture::SamplingFilter::linear, gl::Texture::SamplingFilter::linear);
		_fbo->attach(_tex, GL_COLOR_ATTACHMENT0, 0);
		Swapper _{gl::target, _targetText};

		gl::target->activateFrameBuffer();
		glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		std::string _str{str()};
		vec2 drawHead = vec2(-1,1);
		float maxLineHeight = -1;
		if (_isLineCentre) {
			font::drawStringLineCentre(utf::beginOfUTF8(_str), utf::endOfUTF8(_str), _pointSize, vec2(-1, 1), vec2(2, 2));
		} else {
			font::drawString(utf::beginOfUTF8(_str), utf::endOfUTF8(_str), _pointSize, vec2(-1, 1), vec2(2, 2));
		}
		_change = false; //disable this to make it always refeashing
	}
	//use render texture here
	gl::drawRectangleR8Color(_tex, pos, size*2.f, vec4(0.f,0.2f,0.f,1.f));
}

std::string StringBox::str() const {
	return _ss.str();
}
void StringBox::str(const std::string& string) {
	_change = true;
	_ss.str(string);
}
void StringBox::str(std::string&& string) {
	_change = true;
	_ss.str(std::move(string));
}

void StringBox::clear() {
	_change = true;
	_ss.str(std::string());
	_ss.clear();
}

void StringBox::setSize(vec2 s) {
	_change = true;
	size = s;
}
vec2 StringBox::getSize() const {
	return size;
}
void StringBox::setTextSize(float p) {
	_change = true;
	_pointSize = p;
}

void StringBox::notifyPPIChanged() {
	_change = true;
}

void StringBox::setLineCenter(bool v) {
	_isLineCentre = v;
	_change = true;
}
