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
	extern class FontSet;
	extern class FontFace;

	struct FontFaceData {
		FontFace* _font;
		float lineHeight;
	};
	struct GlyphData {
		GlyphId gId;
		vec2 advance; //normalized
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

	using PositionFunction = std::function<std::pair<float,vec2>(Data)>;
	constexpr float DONT_RENDER_CHAR = 0.f;
	constexpr float DEFAULT_SCALE = 1.f;

	template <typename Iter>
	concept Char_Iter = requires(Iter a) {
		{*a} -> std::convertible_to<char32_t>;
	};

	static void init();
	static std::vector<const std::string*> getAllFontSets();
	static const std::vector<std::string>& getAllFontStyles();
	static bool addFont(const std::string& fileLocation, const std::string& fontSetName, const std::vector<std::string>& style = {});
	static void useFont(const std::string& fontSet, const std::vector<std::string>& style = {});
	static void closeFontSet(const std::string& fontSet);
	static void closeFont(const std::string& fontSet, const std::vector<std::string>& style = {});
	static void setDefaultFontSet(const std::string& fontSet);


	static FontFaceData getFontFaceData(FontFace* fontFace, float pointSize);
	static GlyphData getGlyphData(FontFace* fontFace, GlyphId gId, float pointSize);
	static void renderRequiredGlyph();
	static void _renderBatch(FontFace* f, std::vector<_RenderData> d, float pointSize);
	static std::pair<FontFace*, GlyphId> getGlyphFromChar(char32_t c);
	
	template <Char_Iter Iter>
	static void drawChar(Iter it, Iter itEnd, float pointSize, PositionFunction func) {
		std::unordered_map<FontFace*, std::vector<_RenderData>> glyphsToRender{};
		std::unordered_map<FontFace*, FontFaceData> fonts{};
		std::unordered_map<std::pair<FontFaceData*, GlyphId>, GlyphData> glyphs{};
		std::unordered_map<char32_t, std::pair<FontFaceData*, GlyphData*>> cacheMap{};
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
						fontIt = fonts.emplace_hint(fontIt, getFontFaceData(pair.first, pointSize));
					}
					font = &fontIt->second;
					auto glyphIt = glyphs.lower_bound(std::make_pair(font, pair.second));
					if (glyphIt==glyphs.end()) {
						glyphIt = glyphs.emplace_hint(glyphIt, std::make_pair(font, pair.second), getGlyphData(pair.first, pair.second, pointSize));
					}
					glyph = &glyphIt->second;
				}
				cache = cacheMap.emplace_hint(cache, *it, font, glyph);
			}
			if (cache->second.first==nullptr) {
				continue;
			}
			auto& p = cache->second;
			auto result = func(Data{*it,p.first,p.second});
			if (result.first!=DONT_RENDER_CHAR)
				glyphsToRender[p.first->_font].emplace_back(_RenderData{p.second->gId,result.second});
		}
		renderRequiredGlyph(); //reRender Glyphs that needs higher resolution
		//Finally render char
		for (auto& font : glyphsToRender) {
			_renderBatch(font.first, font.second, pointSize);
		}
	}
}

