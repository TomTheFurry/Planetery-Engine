#pragma once
#include "Define.h"
#include "DefineMath.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <string_view>

#define GL_SSBO_IDENTIFIER_FONT_GLYPH uint(42)
constexpr auto CHARCODE_8_UNKNOWN_CHAR = '?';
constexpr auto CHARCODE_8_ONKNOWN_OBJECT = '?';
constexpr auto CHARCODE_16_UNKNOWN_CHAR = char16_t(0xFFFD);
constexpr auto CHARCODE_16_ONKNOWN_OBJECT = char16_t(0xFFFC);
constexpr auto CHARCODE_32_UNKNOWN_CHAR = char32_t(0x0000FFFD);
constexpr auto CHARCODE_32_ONKNOWN_OBJECT = char32_t(0x0000FFFC);
constexpr auto CHARCODE_NEXTLINE = '\n';


namespace font {
	using GlyphId = uint;
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

	//return value:            float: scale of char           vec2: pos of char    
	typedef std::function<std::pair<float, vec2>(Data)> PositionFunction;
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
	extern void close();

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
	extern void _renderBatch(FontFace* f, std::vector<RenderData> d, float pointSize);
	extern std::pair<FontFace*, GlyphId> getGlyphFromChar(char32_t c);
	extern std::pair<FontFace*, GlyphId> getGlyphFromChar(char32_t c, char32_t replacementChar);

	template <CharTypeIter Iter>
	static void drawChar(Iter it, Iter itEnd, float pointSize, PositionFunction func) {
		std::unordered_map<FontFace*, std::vector<RenderData>> glyphsToRender{};
		std::map<FontFace*, FontFaceData> fonts{};
		std::map<std::pair<FontFaceData*, GlyphId>, GlyphData> glyphs{};
		std::map<char32_t, std::pair<FontFaceData*, GlyphData*>> cacheMap{};
		for (; it!=itEnd; it++) {
			auto cache = cacheMap.lower_bound(char32_t(*it));
			if (cache==cacheMap.end() || cache->first!=*it) {
				auto pair = getGlyphFromChar(*it); //OPTI: cache the char->FontSet w/ gId pair in FontSet
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
				glyphsToRender[p.first->_font].emplace_back(RenderData{p.second->gId ,result.second});
		}
		renderRequiredGlyph(); //reRender Glyphs that needs higher resolution
		//Finally render char
		for (auto& font : glyphsToRender) {
			_renderBatch(font.first, font.second, pointSize);
		}
	}
	
	namespace fullStringFunction {
		typedef std::vector<std::pair<FontFaceData, std::vector<GlyphData>>> FontData; // std::vector<std::pair<FontFaceData, std::vector<GlyphData>>>
		typedef std::vector<std::tuple<char32_t, uint, uint>> Chars; // std::vector<std::tuple<char32_t, uint, uint>>
		typedef std::vector<std::pair<FontFace*, std::vector<GlyphId>>> IndexLookup; // std::vector<std::pair<FontFace*, std::vector<GlyphId>>>
		typedef std::vector<std::vector<RenderData>> Output; // std::vector<std::vector<RenderData>>
		typedef std::function<void(
			const FontData&, const Chars&, const IndexLookup&, Output&
			)> FullStringFunction;
	}
	typedef fullStringFunction::FullStringFunction FullStringFunction;

	template <CharTypeIter Iter>
	static void drawChar(Iter it, Iter itEnd, float pointSize, FullStringFunction func) {
		fullStringFunction::FontData fontData{};
		fullStringFunction::Chars chars{};
		fullStringFunction::IndexLookup indexLookup{};
		std::map<char32_t, std::pair<uint, uint>> cacheCharMap{};

		for (; it!=itEnd; it++) {
			auto cacheIt = cacheCharMap.lower_bound(char32_t(*it));
			if (cacheIt==cacheCharMap.end() || cacheIt->first!=char32_t(*it)) {
				auto lookupPair = getGlyphFromChar(char32_t(*it), CHARCODE_16_UNKNOWN_CHAR);
				if (lookupPair.first==nullptr) {
					cacheCharMap.emplace_hint(cacheIt, char32_t(*it), std::pair{-1, -1});
					continue;
				}
				uint facePos;
				{
					FontFace* ff = lookupPair.first;
					auto faceIndexIt = std::find_if(indexLookup.begin(), indexLookup.end(), [ff](const auto& t) {return t.first==ff; });
					if (faceIndexIt==indexLookup.end()) {
						indexLookup.emplace_back(lookupPair.first, std::vector<GlyphId>{});
						fontData.emplace_back(getFontFaceData(lookupPair.first, pointSize), std::vector<GlyphData>{});
						facePos = indexLookup.size()-1;
						assert(indexLookup.size()==fontData.size());
						assert(facePos+1==fontData.size());
					} else {
						facePos = faceIndexIt-indexLookup.begin();
						assert(facePos<fontData.size());
					}
				}
				uint glyphPos;
				{
					auto& glyphIndexLookup = indexLookup.at(facePos).second;
					auto glyphIndexIt = std::find(glyphIndexLookup.begin(), glyphIndexLookup.end(), lookupPair.second);
					if (glyphIndexIt==glyphIndexLookup.end()) {
						glyphIndexLookup.emplace_back(lookupPair.second);
						auto& glyphData = fontData.at(facePos).second;
						glyphData.emplace_back(getGlyphData(lookupPair.first, lookupPair.second, pointSize));
						glyphPos = glyphIndexLookup.size()-1;
						assert(glyphIndexLookup.size()==glyphData.size());
						assert(glyphPos+1==glyphData.size());
					} else {
						glyphPos = glyphIndexIt-glyphIndexLookup.begin();
						assert(glyphPos<fontData.at(facePos).second.size());
					}
				}
				cacheIt = cacheCharMap.emplace_hint(cacheIt, char32_t(*it), std::pair{facePos, glyphPos});
			}
			uint& faceIndex = cacheIt->second.first;
			uint& glyphIndex = cacheIt->second.second;
			//if (faceIndex == -1) continue; //not skipping invalid charcode as it may be used as user defined control code
			{ //Safety check
				assert(faceIndex < indexLookup.size());
				assert(faceIndex < fontData.size());
				assert(indexLookup.size()==fontData.size());
				assert(glyphIndex < indexLookup.at(faceIndex).second.size());
				assert(glyphIndex < fontData.at(faceIndex).second.size());
				assert(indexLookup.at(faceIndex).second.size()==fontData.at(faceIndex).second.size());
				assert(cacheIt->first==char32_t(*it));
			}
			chars.emplace_back(char32_t(*it), faceIndex, glyphIndex);
		}
		renderRequiredGlyph(); //reRender Glyphs that needs higher resolution
		//Call user function
		fullStringFunction::Output output{};
		func(fontData, chars, indexLookup, output);

		//Finally render char
		if (output.size() > indexLookup.size()) throw "Invalid output from FullStringFunction() call in font::drawChar()!";

		for (uint i = 0; i<output.size(); i++) {
			_renderBatch(indexLookup.at(i).first, output.at(i), pointSize);
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
				vec2 loc = drawHead;
				drawHead.x += d.glyph->advance.x;
				if (drawHead.x > position.x+maxWidth) {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
				}
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				return std::make_pair(1.f, loc);
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
				vec2 loc = drawHead;
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				drawHead.x += d.glyph->advance.x;
				return std::make_pair(1.f, loc);
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
				vec2 loc = drawHead;
				drawHead.x += d.glyph->advance.x;
				if (drawHead.x > position.x+maxWidth) {
					drawHead.x = position.x;
					drawHead.y -= lineHeight;
				}
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				return std::make_pair(1.f, loc);
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
				vec2 loc = drawHead;
				if (d.face->lineHeight > lineHeight) lineHeight = d.face->lineHeight;
				drawHead.x += d.glyph->advance.x;
				return std::make_pair(1.f, loc);
			}
		);
	}
}

