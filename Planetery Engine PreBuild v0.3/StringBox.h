#pragma once
#include "DefineMath.h"
#include "Font.h"
#include <sstream>
#include "GL.h"

class StringBox : protected std::stringstream {
public:
	StringBox();
	vec2 pos; //normalized (-1 - 1)

	void render();
	std::string str() const;
	void str(const std::string& string);

	void setSize(vec2 size); //true normalized length (0 - 1)
	vec2 getSize(); //true normalized length (0 - 1)
	void setTextSize(float pointSize);
	void notifyPPIChanged();

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
	vec2 size; //true normalized length (0 - 1)
	bool _change;
	float _pointSize;
	vec2 _ppi;
	vec2 _textureSize;
	gl::Texture2D* _tex;
	gl::FrameBuffer* _fbo;
};

