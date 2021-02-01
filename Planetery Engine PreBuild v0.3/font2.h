#pragma once
#include "Define.h"
#include "DefineMath.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <string_view>

#define GL_SSBO_IDENTIFIER_FONT_GLYPH uint(42)
#define CHARCODE_8_UNKNOWN_CHAR '?'
#define CHARCODE_8_ONKNOWN_OBJECT '?'
#define CHARCODE_16_UNKNOWN_CHAR char16_t(0xFFFD)
#define CHARCODE_16_ONKNOWN_OBJECT char16_t(0xFFFC)
#define CHARCODE_32_UNKNOWN_CHAR char32_t(0x0000FFFD)
#define CHARCODE_32_ONKNOWN_OBJECT char32_t(0x0000FFFC)
#define CHARCODE_NEXTLINE '\n'


namespace font {
	using GlyphId = uint;
	class FontSet;
	class FontFace;

	struct FontFaceData {
		FontFace* _font;
		float lineHeight;
	};
	struct GlyphData {
		GlyphId gId;
		vec2 advance; //relative to screen size
		vec2 textureResolutionScale; // >1 = enlarge, <1 = minifly
	};
	struct Data {
		char32_t charCode;
		const FontFaceData* face;
		const GlyphData* glyph;
	};
	struct _RenderData {
		GlyphId gId;
		vec2 pos;
	};

	//return value:            float: scale of char           vec2: pos of char    
	using PositionFunction = std::function<std::pair<float,vec2>(Data)>;
	constexpr float DONT_RENDER_CHAR = 0.f;
	constexpr float DEFAULT_SCALE = 1.f; 

	template <typename T>
	concept CharType = requires(T t) {
		{t} -> std::convertible_to<char32_t>;
	};

	template <typename Iter>
	concept CharTypeIter = requires(Iter a) {
		{*a} -> std::convertible_to<char32_t>;
	};

	extern void init();

	extern std::vector<const std::string*> getAllFontSets();
	extern const std::vector<std::string> getAllFontStyles();
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
	extern void _renderBatch(FontFace* f, std::vector<_RenderData> d, float pointSize);
	extern std::pair<FontFace*, GlyphId> getGlyphFromChar(char32_t c);
	
	template <CharTypeIter Iter>
	static void drawChar(Iter it, Iter itEnd, float pointSize, PositionFunction func) {
		std::unordered_map<FontFace*, std::vector<_RenderData>> glyphsToRender{};
		std::map<FontFace*, FontFaceData> fonts{};
		std::map<std::pair<FontFaceData*, GlyphId>, GlyphData> glyphs{};
		std::map<char32_t, std::pair<FontFaceData*, GlyphData*>> cacheMap{};
		for (; it!=itEnd; it++) {
			auto cache = cacheMap.lower_bound(char32_t(*it));
			if (cache==cacheMap.end() || cache->first!=*it) {
				auto pair = getGlyphFromChar(*it);
				if (pair.first==nullptr) //Font/Glyph set is NOT found, use BACKUPCHARCODE
					pair = getGlyphFromChar(CHARCODE_32_UNKNOWN_CHAR);
				FontFaceData* font = nullptr;
				GlyphData* glyph = nullptr;
				if (pair.first != nullptr) {
					auto fontIt = fonts.lower_bound(pair.first);
					if (fontIt==fonts.end() || fontIt->first!=pair.first) {
						fontIt = fonts.emplace_hint(fontIt, pair.first, getFontFaceData(pair.first, pointSize));
					}
					font = &fontIt->second;
					auto glyphIt = glyphs.lower_bound(std::make_pair(font, pair.second));
					if (glyphIt==glyphs.end() || glyphIt->first!=std::make_pair(font, pair.second)) {
						glyphIt = glyphs.emplace_hint(glyphIt, std::make_pair(font, pair.second), getGlyphData(pair.first, pair.second, pointSize));
					}
					glyph = &glyphIt->second;
				}
				cache = cacheMap.emplace_hint(cache, *it, std::make_pair(font, glyph));
			}
			if (cache->second.first==nullptr) {
				continue;
			}
			auto& p = cache->second;
			auto result = func(Data{char32_t(*it),p.first,p.second});
			if (result.first!=DONT_RENDER_CHAR)
				glyphsToRender[p.first->_font].emplace_back(_RenderData{p.second->gId ,result.second});
		}
		renderRequiredGlyph(); //reRender Glyphs that needs higher resolution
		//Finally render char
		for (auto& font : glyphsToRender) {
			_renderBatch(font.first, font.second, pointSize);
		}
	}

	static void drawString(const std::string& string, float pointSize, vec2 position, float maxWidth) {
		vec2 drawHead = position;
		float lineHeight = 0;

		drawChar(string.begin(), string.end(), pointSize,
			[&](Data d) {
				if (d.charCode=='\n') {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
					lineHeight = 0;
					return std::make_pair(0.f, vec2(0));
				}
				drawHead.x += d.glyph->advance.x;
				if (drawHead.x > position.x+maxWidth) {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
				}
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				return std::make_pair(1.f, drawHead);
			}
		);
	}
	static void drawString(const std::string& string, float pointSize, vec2 position) {
		vec2 drawHead = position;
		float lineHeight = 0;

		drawChar(string.begin(), string.end(), pointSize,
			[&](Data d) {
				if (d.charCode=='\n') {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
					lineHeight = 0;
					return std::make_pair(0.f, vec2(0));
				}
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				drawHead.x += d.glyph->advance.x;
				return std::make_pair(1.f, drawHead);
			}
		);
	}
	static void drawString(const std::wstring& string, float pointSize, vec2 position, float maxWidth) {
		vec2 drawHead = position;
		float lineHeight = 0;

		drawChar(string.begin(), string.end(), pointSize,
			[&](Data d) {
				if (d.charCode=='\n') {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
					lineHeight = 0;
					return std::make_pair(0.f, vec2(0));
				}
				drawHead.x += d.glyph->advance.x;
				if (drawHead.x > position.x+maxWidth) {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
				}
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				return std::make_pair(1.f, drawHead);
			}
		);
	}
	static void drawString(const std::wstring& string, float pointSize, vec2 position) {
		vec2 drawHead = position;
		float lineHeight = 0;

		drawChar(string.begin(), string.end(), pointSize,
			[&](Data d) {
				if (d.charCode=='\n') {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
					lineHeight = 0;
					return std::make_pair(0.f, vec2(0));
				}
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				drawHead.x += d.glyph->advance.x;
				return std::make_pair(1.f, drawHead);
			}
		);
	}
}

