module;
#include <glad/glad.h>
#include <glm/glm.hpp>
module StringBox;

import std.core;
import Define;
import Font;
import Util;
import GL;
import Logger;

static gl::RenderTarget* _targetText = nullptr;

StringBox::StringBox() : _ss() {
	pos = vec2(0, 0);
	backgroundColor = vec4(0.05,0.07,0.2,0.98);
	borderColor = vec4(0.01,0.015,0.03,1.0);
	borderThickness = vec2(0.01);
	size = vec2(0, 0);
	_ppi = vec2(0, 0);
	_textureSize = uvec2(0, 0);
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
		_textureSize = uvec2(glm::ceil(vec2(gl::target->viewport.z, gl::target->viewport.w)*size));
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
		vec2 drawHead = vec2(-1, 1);
		float maxLineHeight = -1;
		if (_isLineCentre) {
			font::drawStringLineCentre(utf::beginOfUTF8(_str), utf::endOfUTF8(_str), _pointSize, vec2(-1, 1), vec2(2, 2));
		} else {
			font::drawString(utf::beginOfUTF8(_str), utf::endOfUTF8(_str), _pointSize, vec2(-1, 1), vec2(2, 2));
		}
		_change = false; //disable this to make it always refeashing
	}
	//use render texture here
	float wOverH = float(gl::target->viewport.z)/float(gl::target->viewport.w);
	vec2 trueThick = vec2(borderThickness.x/wOverH, borderThickness.y);
	if (borderThickness==vec2(0)) {
		gl::drawRectangles({gl::GLRect{.pos = pos, .size = size*2.f}}, backgroundColor)->release();
	} else if (backgroundColor==vec4(0)) {
		gl::drawRectanglesBorder({gl::GLRect{.pos = pos-trueThick, .size = (size+trueThick)*2.f}}, trueThick, borderColor)->release();
	} else {
		gl::drawRectanglesFilledBorder({gl::GLRect{.pos = pos-trueThick, .size = (size+trueThick)*2.f}}, backgroundColor, trueThick, borderColor)->release();
	}
	gl::drawTexR8Rectangle(_tex, gl::GLRect{.pos = pos, .size = size*2.f}, vec4(0.f, 0.2f, 0.f, 1.f))->release();
}

std::string StringBox::str() const {
	//MODULE HOTFIX:
	//return _ss.str();
	return _ss;
	//MODULE HOTFIX
}
void StringBox::str(const std::string& string) {
	_change = true;
	//MODULE HOTFIX:
	//_ss.str(string);
	_ss = string;
	//MODULE HOTFIX
}
void StringBox::str(std::string&& string)
{
	_change = true;
	//MODULE HOTFIX:
	//_ss.str(std::move(string));
	_ss = std::move(string);
	//MODULE HOTFIX
}

void StringBox::clear() {
	_change = true;
	//MODULE HOTFIX:
	//_ss.str(std::string());
	//_ss.clear();
	_ss.clear();
	//MODULE HOTFIX
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
