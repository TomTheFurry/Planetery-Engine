#pragma once
#include "Define.h"
#include "DefineMath.h"

class FontManager {
public:
	//unit convertion
	static vec2 pixelToPoint(vec2 v);
	static vec2 pointToPixel(vec2 v);


	static void init();
	static bool fontLoaded(const std::string& fontName);
    static bool loadFont(const std::string& fontName, const std::string& path, const std::string& fallbackFont = "", float pointSize = 72.f);
	static bool useFont(const std::string& fontName);
	static void renderString(const std::string& string, vec2 locationNormalized, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderString(const std::wstring& string, vec2 locationNormalized, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderString(const std::u8string& wstring, vec2 locationNormalized, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderString(const std::u16string& wstring, vec2 locationNormalized, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderString(const std::u32string& wstring, vec2 locationNormalized, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));

	//render in a box area. vec4 box = vec4(x, y, boxWidth, lineHeight)
	static void renderStringInBox(const std::string& string, vec4 box, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderStringInBox(const std::wstring& string, vec4 box, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderStringInBox(const std::u8string& wstring, vec4 box, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderStringInBox(const std::u16string& wstring, vec4 box, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
	static void renderStringInBox(const std::u32string& wstring, vec4 box, float pointSize, vec4 texColor = vec4(0, 0, 0, 1));
};

class FontSetting {
	vec4 color;
	float hieght;

};
