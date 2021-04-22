#pragma once
#include "Define.h"
#include "DefineMath.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <string_view>
#include "UTF.h"

#define GL_SSBO_IDENTIFIER_FONT_GLYPH uint(42)
constexpr auto CHARCODE_8_UNKNOWN_CHAR = '?';
constexpr auto CHARCODE_8_ONKNOWN_OBJECT = '?';
constexpr auto CHARCODE_16_UNKNOWN_CHAR = char16_t(0xFFFD);
constexpr auto CHARCODE_16_ONKNOWN_OBJECT = char16_t(0xFFFC);
constexpr auto CHARCODE_32_UNKNOWN_CHAR = char32_t(0x0000FFFD);
constexpr auto CHARCODE_32_ONKNOWN_OBJECT = char32_t(0x0000FFFC);
constexpr auto CHARCODE_NEXTLINE = '\n';
constexpr uint TAB_SPACE = 4;

namespace font {
	using GlyphId = uint;
	constexpr float DONT_RENDER_CHAR = 0.f;
	constexpr float DEFAULT_SCALE = 1.f;

	template <typename Iter>
	concept CharTypeIter = requires(Iter a) {
		{*a} -> std::convertible_to<char32_t>;
	};

	class FontSet;
	class FontFace;

	struct FontFaceData {
		FontFace* _font;
		float lineHeight;
		float maxDescend;
		float maxAccend;
		float maxLeft;
		float maxRight;
		vec2 maxAdvance;
		float underlinePosition; //the center of the line
		float underlineThickness;
	};
	struct GlyphData {
		GlyphId gId;
		float descend;
		float accend;
		float left;
		float right;
		vec2 advance; //relative to screen size
		vec2 textureResolutionScale; // >1 = enlarge, <1 = minifly
	};
	struct Data {
		char32_t charCode;
		const FontFaceData* face;
		const GlyphData* glyph;
	};
	struct RenderData {
		GlyphId gId;
		vec2 pos;
	};
	class Reader {
	public:
		Reader(float fontSize);
		std::vector<std::pair<FontFaceData, std::vector<GlyphData>>> fontData{};
		std::vector<std::tuple<char32_t, uint, uint>> chars{};
		std::vector<std::pair<FontFace*, std::vector<GlyphId>>> indexLookup{};
		std::map<char32_t, std::pair<uint, uint>> cacheCharMap{};
		Reader& operator<<(char32_t c);
	private:
		float _fontSize;
	};

	typedef std::vector<std::vector<RenderData>> Output;
	typedef std::function<void(Reader& data, Output& output)> FullStringFunction;

	extern void init();
	extern void close();

	extern std::vector<const std::string*> getAllFontSets();
	extern std::vector<std::string> getAllFontStyles();
	extern bool addFont(const std::string& fontSetName, const std::string& fileLocation, const std::vector<std::string>& style = {});
	extern bool linkFallbackFont(const std::string& fontSet, const std::string& fallbackFontSet);
	extern bool useFont(const std::string& fontSet = {}); //leave empty as default
	extern void useStyle(const std::vector<std::string>& style = {});
	extern bool toggleStyle(const std::string& style, bool setTo); //return previous value
	extern void closeFontSet(const std::string& fontSet);
	extern void closeFont(const std::string& fontSet, const std::vector<std::string>& style = {});
	extern bool setDefaultFontSet(const std::string& fontSet);


	extern FontFaceData getFontFaceData(FontFace* fontFace, float pointSize);
	extern GlyphData getGlyphData(FontFace* fontFace, GlyphId gId, float pointSize);
	extern void renderRequiredGlyph();
	extern void _renderBatch(FontFace* f, std::vector<RenderData> d, float pointSize);
	extern std::pair<FontFace*, GlyphId> getGlyphFromChar(char32_t c, char32_t replacementChar);

	template <CharTypeIter Iter>
	static void drawChar(Iter it, Iter itEnd, float pointSize, FullStringFunction func) {
		Reader stringReader(pointSize);
		while (it!=itEnd) stringReader << *(it++);
		renderRequiredGlyph(); //reRender Glyphs that needs higher resolution
		//Call user function
		Output output{};
		func(stringReader, output);
		//Finally render char
		if (output.size() > stringReader.indexLookup.size()) throw "Invalid output from FullStringFunction() call in font::drawChar()!";
		for (uint i = 0; i<output.size(); i++) {
			_renderBatch(stringReader.indexLookup.at(i).first, output.at(i), pointSize);
		}
	}

	extern void _drawStringFunction(Reader& r, Output& output, vec2 topLeftPos, vec2 maxSize);
	template <CharTypeIter Iter>
	static void drawString(Iter it, Iter itEnd, float pointSize, vec2 topLeftPos, vec2 maxSize) {
		vec2 drawHead = vec2(-1, 1);
		float maxLineHeight = -1;
		
		Reader stringReader(pointSize);
		stringReader << U' '; //Add space reading request
		while (it!=itEnd) {
			stringReader << *(it++);
		}
		renderRequiredGlyph(); //reRender Glyphs that needs higher resolution
		//Do Pos Function
		Output output{};
		_drawStringFunction(stringReader, output, topLeftPos, maxSize);

		//Finally render char
		if (output.size() > stringReader.indexLookup.size()) throw "Invalid output from FullStringFunction() call in font::drawChar()!";
		for (uint i = 0; i<output.size(); i++) {
			_renderBatch(stringReader.indexLookup.at(i).first, output.at(i), pointSize);
		}
	}
	extern void _drawStringCentreFunction(Reader& r, Output& output, vec2 topLeftPos, vec2 maxSize);
	template <CharTypeIter Iter>
	static void drawStringLineCentre(Iter it, Iter itEnd, float pointSize, vec2 topLeftPos, vec2 maxSize) {
		vec2 drawHead = vec2(-1, 1);
		float maxLineHeight = -1;

		Reader stringReader(pointSize);
		stringReader << U' '; //Add space reading request
		while (it!=itEnd) {
			stringReader << *(it++);
		}
		renderRequiredGlyph(); //reRender Glyphs that needs higher resolution
		//Do Pos Function
		Output output{};
		_drawStringCentreFunction(stringReader, output, topLeftPos, maxSize);

		//Finally render char
		if (output.size() > stringReader.indexLookup.size()) throw "Invalid output from FullStringFunction() call in font::drawChar()!";
		for (uint i = 0; i<output.size(); i++) {
			_renderBatch(stringReader.indexLookup.at(i).first, output.at(i), pointSize);
		}
	}
}
