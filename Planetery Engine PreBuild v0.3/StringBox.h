#pragma once
#include "DefineMath.h"
#include "Font.h"
#include <sstream>
#include "GL.h"

class StringBox : protected std::stringstream {
public:
	StringBox();
	vec2 pos;
	vec2 size;

	void render();
	std::string str() const;
	void str(const std::string& string);

	template <typename T>
	StringBox& operator<< (T&& t) {
		_change = true;
		this->std::stringstream::operator<<(std::forward<T>(t));
	}
	template <typename T>
	StringBox& operator>> (T&& t) {
		_change = true;
		this->std::stringstream::operator>>(std::forward<T>(t));
	}
protected:
	bool _change;
	gl::RenderBuffer* _rbo;
	gl::FrameBuffer* _fbo;
};

