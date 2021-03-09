#pragma once
#include "DefineMath.h"
#include "Font.h"
#include <iostream>
#include <sstream>
#include "GL.h"

class StringBox
{
  public:
	StringBox();
	vec2 pos;  // normalized (-1 - 1)
	vec4 backgroundColor;
	vec4 borderColor;
	vec2 borderThickness;
	void render();
	std::string str() const;
	void str(const std::string& string);
	void str(std::string&& string);
	void clear();

	void setSize(vec2 size);  // true normalized length (0 - 1)
	vec2 getSize() const;	  // true normalized length (0 - 1)
	void setTextSize(float pointSize);
	void notifyPPIChanged();
	void setLineCenter(bool v);

	template<typename T> StringBox& operator<<(T&& t) {
		_change = true;
		_ss << (std::forward<T>(t));
		return *this;
	}
  protected:
	vec2 size;	// true normalized length (0 - 1)
	bool _change;
	bool _isLineCentre;
	float _pointSize;
	vec2 _ppi;
	uvec2 _textureSize;
	gl::Texture2D* _tex;
	gl::FrameBuffer* _fbo;
	std::ostringstream _ss;
};
