module;

export module StringBox;
import std.core;
import Define;
import Font;
import GL;

export class StringBox {
public:
	StringBox();
	vec2 pos; //normalized (-1 - 1)
	vec4 backgroundColor;
	vec4 borderColor;
	vec2 borderThickness;
	void render();
	std::string str() const;
	void str(const std::string& string);
	void str(std::string&& string);
	void clear();

	void setSize(vec2 size); //true normalized length (0 - 1)
	vec2 getSize() const; //true normalized length (0 - 1)
	void setTextSize(float pointSize);
	void notifyPPIChanged();
	void setLineCenter(bool v);

	/*MODULE HOTFIX:
	template <typename T>
	StringBox& operator<< (T&& t) {
		_change = true;
		_ss << (std::forward<T>(t));
		return *this;
	}*/

	template <typename T>
	requires std::convertible_to<T, std::string>
		StringBox& operator<< (T&& t) {
		_change = true;
		_ss += t;
		return *this;
	}
	template <typename T>
	requires !std::convertible_to<T, std::string>
	StringBox& operator<< (T&& t) {
		_change = true;
		_ss += std::to_string(t);
		return *this;
	}


	//MODULE HOTFIX

protected:
	vec2 size; //true normalized length (0 - 1)
	bool _change;
	bool _isLineCentre;
	float _pointSize;
	vec2 _ppi;
	uvec2 _textureSize;
	gl::Texture2D* _tex;
	gl::FrameBuffer* _fbo;
	//MODULE HOTFIX:
	//std::ostringstream _ss;
	std::string _ss;
	//MODULE HOTFIX
};

