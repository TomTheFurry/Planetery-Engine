#include "font.h"
#ifdef USE_OPENGL
#	include "GL.h"

#include "ThreadEvents.h"
#include "Logger.h"
#include "DefineUtil.h"

#include <set>
#include <unordered_map>
#include <glad/glad.h>
#include <freetype/freetype.h>
#include <shelf-pack.hpp>
#include <array>
#include <memory_resource>

using namespace font;

FT_Library ftLib{};

struct _Glyph {
	uint _gId;
	uint _renderedPointSize;  // 0 = Needs Loading the unit stuff
	ivec2 _advanceUnit;
	ivec2 _bearingUnit;
	uvec2 _sizeUnit;
};

typedef unsigned int Style;

class font::FontFace
{
  public:
	FontSet* fontSet;
	Style style;
	vec2 texturePPI;
	uint _lineHeightUnit = 0;
	uint _maxDescendUnit = 0;
	uint _maxAccendUnit = 0;
	uint _maxLeftUnit = 0;
	uint _maxRightUnit = 0;
	uvec2 _maxAdvanceUnit = uvec2(0);
	uint _underlinePositionUnit = 0;
	uint _underlineThicknessUnit = 0;
	uint _unitPerEM = 0;
	gl::Texture2D* texture = nullptr;
	gl::ShaderStorageBuffer* ssbo = nullptr;
	mapbox::ShelfPack* textureShelf = nullptr;
	FT_Face face = nullptr;
	std::pmr::monotonic_buffer_resource pmr_r;
	std::pmr::unordered_map<char32_t, uint> charCodeLookup{};
	std::vector<_Glyph> glyphs{};
	~FontFace() {
		if (ssbo) ssbo->release();
		if (texture) texture->release();
		if (textureShelf) {
			textureShelf->clear();
			delete textureShelf;
		}
		if (face) FT_Done_Face(face);
	}
};
class font::FontSet
{
  public:
	std::string fontName;
	std::vector<FontSet*> fallbackFonts{};
	std::vector<FontFace*> fontFaces{};

	FontFace* getFace(Style style) {
		assert(!fontFaces.empty());
		FontFace* mostMatch = fontFaces[0];
		uint matching = __popcnt(style ^ fontFaces[0]->style);
		for (auto face : fontFaces) {
			// NOTE: This requires popcnt() which is not in standard!!
			uint v = __popcnt(style ^ face->style);
			if (v > matching) {
				matching = v;
				mostMatch = face;
			}
		}
		return mostMatch;
	}
	~FontSet() {
		for (auto faces : fontFaces) { delete faces; }
	}
};

// TODO: invalidate all FontSet.cachedStyleMap on changing _style.
static std::array<std::string, sizeof(Style) * 8> _style;
static std::vector<FontSet*> _fontSets;
static FontSet* _targetFont = nullptr;
static FontSet* _defaultFont = nullptr;
static Style _targetStyle = 0;
static gl::ShaderProgram* _textShader = nullptr;
static gl::VertexAttributeArray* _vao = nullptr;
static gl::VertexBuffer* _vbo = nullptr;
static const std::array<const uint, 14> _size{
  {2, 4, 6, 8, 12, 16, 20, 24, 36, 48, 60, 72, 144, 288}};
static std::array<uint, 7> _texSize{
  {1024, 4096, 16384, 65536, 262144, 1048576, uint(-1)}};
static uint MAXTEXTURESIZE = 0;
static FT_Library _ftLib = nullptr;
constexpr uint GLYP_SPRITE_BORDER_WIDTH = 2;
// THE BELOW thread_local MAP MAY NOT CALL DESTRUCTOR!!!!!
thread_local std::pmr::monotonic_buffer_resource _pmr_r_reRender;
thread_local std::pmr::unordered_map<FontFace*,
  std::pmr::unordered_map<GlyphId, uint>>
  _requireRender{&_pmr_r_reRender};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
static constexpr const std::array _dChar{U'⎕', U'⌈', U'⊥', U'⌋',  // 4
  U'⌁', U'⊠', U'✓', U'⍾',										  // 8
  U'⤺', U'⪫', U'≡', U'⩛',										  // C
  U'↡', U'⪪', U'⊗', U'⊙',										  // 10
  U'⊟', U'◷', U'◶', U'◵',										  // 14
  U'◴', U'⍻', U'⎍', U'⊣',										  // 18
  U'⧖', U'⍿', U'␦', U'⊖',										  // 1C
  U'◰', U'◱', U'◲', U'◳',										  // 20
  U'△'};

inline char32_t _replaceSpecialChar(char32_t c) {
//#define USE_ISO_2047_STANDARD
#ifdef USE_ISO_2047_STANDARD
	if (c < _dChar.size()) { return _dChar.at(c); }
	switch (c) {
	case U'\u007F': return U'▨';
	}
	return c;
#else
	if (c <= U'\u0026') { return c + 0x2400; }
	return c;
#endif
}
inline FontSet* _getFontSet(const std::string& name) {
	for (auto ptr : _fontSets) {
		if (ptr->fontName == name) return ptr;
	}
	return nullptr;
}
inline bool _checkFont(char32_t c, FontSet* f, std::set<FontSet*>& searched,
  std::pair<FontFace*, GlyphId>& result) {
	auto itS = searched.lower_bound(f);
	if (itS != searched.end() && *itS == f) return false;
	searched.emplace_hint(itS, f);
	FontFace* fontFace = f->getFace(_targetStyle);
	auto it = fontFace->charCodeLookup.find(c);
	if (it != fontFace->charCodeLookup.end()) {
		result = std::make_pair(fontFace, it->second);
		return true;
	}
	for (auto fontSet : f->fallbackFonts) {
		if (_checkFont(c, fontSet, searched, result)) return true;
	}
	return false;
}

font::Reader::Reader(float fontSize): _fontSize(fontSize) {}
Reader& Reader::operator<<(char32_t c) {
	// c = _replaceSpecialChar(c);
	auto cacheIt = cacheCharMap.lower_bound(c);
	if (cacheIt == cacheCharMap.end() || cacheIt->first != c) {
		auto lookupPair = getGlyphFromChar(c, CHARCODE_16_UNKNOWN_CHAR);
		if (lookupPair.first == nullptr) {
			cacheCharMap.emplace_hint(cacheIt, c, std::pair{-1, -1});
			return *this;
		}
		uint facePos;
		{
			FontFace* ff = lookupPair.first;
			auto faceIndexIt = std::find_if(indexLookup.begin(),
			  indexLookup.end(), [ff](const auto& t) { return t.first == ff; });
			if (faceIndexIt == indexLookup.end()) {
				indexLookup.emplace_back(
				  lookupPair.first, std::vector<GlyphId>{});
				fontData.emplace_back(
				  getFontFaceData(lookupPair.first, _fontSize),
				  std::vector<GlyphData>{});
				facePos = indexLookup.size() - 1;
				assert(indexLookup.size() == fontData.size());
				assert(facePos == fontData.size() - 1);
			} else {
				facePos = faceIndexIt - indexLookup.begin();
				assert(facePos < fontData.size());
			}
		}
		uint glyphPos;
		{
			auto& glyphIndexLookup = indexLookup.at(facePos).second;
			auto glyphIndexIt = std::find(glyphIndexLookup.begin(),
			  glyphIndexLookup.end(), lookupPair.second);
			if (glyphIndexIt == glyphIndexLookup.end()) {
				glyphIndexLookup.emplace_back(lookupPair.second);
				auto& glyphData = fontData.at(facePos).second;
				glyphData.emplace_back(
				  getGlyphData(lookupPair.first, lookupPair.second, _fontSize));
				glyphPos = glyphIndexLookup.size() - 1;
				assert(glyphIndexLookup.size() == glyphData.size());
				assert(glyphPos == glyphData.size() - 1);
			} else {
				glyphPos = glyphIndexIt - glyphIndexLookup.begin();
				assert(glyphPos < fontData.at(facePos).second.size());
			}
		}
		cacheIt =
		  cacheCharMap.emplace_hint(cacheIt, c, std::pair{facePos, glyphPos});
	}
	uint& faceIndex = cacheIt->second.first;
	uint& glyphIndex = cacheIt->second.second;
	// if (faceIndex == -1) continue; //not skipping invalid charcode as it may
	// be used as user defined control code
	{  // Safety check
		assert(faceIndex < indexLookup.size());
		assert(faceIndex < fontData.size());
		assert(indexLookup.size() == fontData.size());
		assert(glyphIndex < indexLookup.at(faceIndex).second.size());
		assert(glyphIndex < fontData.at(faceIndex).second.size());
		assert(indexLookup.at(faceIndex).second.size()
			   == fontData.at(faceIndex).second.size());
		assert(cacheIt->first == c);
	}
	chars.emplace_back(c, faceIndex, glyphIndex);
	return *this;
}

void font::init() {
	util::Timer initTime;
	MAXTEXTURESIZE = gl::getMaxTextureSize();
	auto it =
	  std::lower_bound(_texSize.begin(), _texSize.end(), MAXTEXTURESIZE);
	*it = MAXTEXTURESIZE;
	while (*(++it) != uint(-1)) { *it = uint(-1); }
	for (auto& str : _style) { str = std::string{}; }
	if (FT_Init_FreeType(&_ftLib)) {
		logger("Failed to init FT Library!!\n");
		throw "FT_Lib_Error";
	};
	_textShader = gl::makeShaderProgram("textRender2", true);

	util::Timer lastResortTime;
	if (!addFont("LastResort", "fonts/LastResortHE-Regular.ttf"))
		throw "LastResort font loading failed";
	logger("LastResort Init Time: ", nanoSec(lastResortTime.time()), "\n");
	util::Timer arialTime;
	if (!addFont("Arial", "fonts/arialuni.ttf"))
		throw "Default font loading failed";
	logger("Arial Init Time: ", nanoSec(arialTime.time()), "\n");
	if (!setDefaultFontSet("Arial")) throw "Default font linking failed";
	if (!linkFallbackFont("Arial", "LastResort"))
		throw "Default font to fallback font linking failed";
	assert(_defaultFont != nullptr);
	// addFont("OrangeJuice", "fonts/orange juice.ttf");
	// addFont("RemachineScript", "fonts/RemachineScript.ttf");
	// addFont("AeroviasBrasilNF", "fonts/AeroviasBrasilNF.ttf");
	// addFont("Sketsaramadhan", "fonts/Sketsaramadhan-nRLAO.otf");
	// addFont("Slick", "fonts/slick.woff");
	// addFont("Emoji", "fonts/AppleEmoji.ttf");
	// addFont("ArialBoldItalic", "fonts/ArialCEBoldItalic.ttf");

	_vao = new gl::VertexAttributeArray();
	_vbo = new gl::VertexBuffer();
	_vao->setBufferBinding(0, _vbo, sizeof(vec2) + sizeof(uint));
	_vao->setAttribute(
	  0, 1, gl::DataType::UnsignedInt, 0, gl::DataType::UnsignedInt);
	_vao->setAttribute(
	  1, 2, gl::DataType::Float, sizeof(uint), gl::DataType::Float);
	_vao->bindAttributeToBufferBinding(0, 0);
	_vao->bindAttributeToBufferBinding(1, 0);
	logger("Font Init Time: ", nanoSec(initTime.time()), "\n");
}
void font::close() {
	for (auto fontSets : _fontSets) { delete fontSets; }
	_vao->release();
	_vbo->release();
	_textShader->release();
	FT_Done_FreeType(_ftLib);
}

std::vector<const std::string*> font::getAllFontSets() {
	std::vector<const std::string*> v{};
	v.reserve(_fontSets.size());
	for (auto& ptr : _fontSets) { v.emplace_back(&ptr->fontName); }
	return v;
}
std::vector<std::string> font::getAllFontStyles() {
	// wil return empty string...
	return std::vector(std::begin(_style), std::end(_style));
}
bool font::addFont(const std::string& fontSetName,
  const std::string& fileLocation, const std::vector<std::string>& style) {
	logger("Loading font file: ", fileLocation, "...\n");
	Style styl = 0;
	for (auto& sty : style) {
		uint i;
		for (i = 0; i < sizeof(Style) * 8 && !_style[i].empty(); i++) {
			if (sty == _style[i]) break;
		}
		if (i >= sizeof(Style) * 8) {
			logger(sizeof(Style) * 8, " styles type limit reached!\n");
		} else {
			_style[i] = sty;
		}
		styl += Style(1) << i;
	}
	FontSet* fontSet = _getFontSet(fontSetName);
	if (fontSet == nullptr) {
		fontSet = _fontSets.emplace_back(new FontSet{fontSetName});
	}
	if (!fontSet->fontFaces.empty() && fontSet->getFace(styl)->style == styl) {
		logger("FontSet ", fontSetName,
		  " with this style aready exist. Unload that first!\n");
		return false;
	}
	FontFace* face = fontSet->fontFaces.emplace_back(
	  new FontFace{fontSet, styl, gl::target->pixelPerInch});
	if (FT_New_Face(_ftLib, fileLocation.data(), 0, &face->face)) {
		logger("Loading failed.\n");
		return false;
	};

	{  // Logging some stuff about fontface and set stuff in face
		face->_unitPerEM = face->face->units_per_EM;
		face->_lineHeightUnit = face->face->height;
		face->_maxDescendUnit = -face->face->bbox.yMin;
		face->_maxAccendUnit = face->face->bbox.yMax;
		face->_maxLeftUnit = -face->face->bbox.xMin;
		face->_maxRightUnit = face->face->bbox.xMax;
		face->_maxAdvanceUnit =
		  uvec2(face->face->max_advance_width, face->face->max_advance_height);
		face->_underlinePositionUnit = face->face->underline_position;
		face->_underlineThicknessUnit = face->face->underline_thickness;
		/*logger.newLayer();
		logger << "Face meta data loaded:\n";
		logger(face->face->family_name, ": ", face->face->style_name, "\n");
		logger("Scalable: ",
		  bool(face->face->face_flags | FT_FACE_FLAG_SCALABLE), "\n");
		logger("Can be horizontal: ",
		  bool(face->face->face_flags | FT_FACE_FLAG_HORIZONTAL), "\n");
		logger("Can be vertical: ",
		  bool(face->face->face_flags | FT_FACE_FLAG_VERTICAL), "\n");
		logger("Has color: ", bool(face->face->face_flags | FT_FACE_FLAG_COLOR),
		  "\n");
		*/
		if (face->face->num_faces > 1)
			logger("Warning!! ", face->face->num_faces,
			  " faces in a single font file detected! Currently only supports "
			  "reading one font face per file!\n");
		if (face->face->charmap->encoding != FT_ENCODING_UNICODE)
			logger("Warning!! This font file does not seem to support unicode! "
				   "Currently only supports unicode charcode mapping! Treating "
				   "it as unicode mapping.\n");
		// logger.closeLayer();
	}

	{  // Load charmap
		logger("Now loading charmaps. Will take some time if font supports "
			   "large number of charcodes...\n");
		std::vector<std::pair<char32_t, uint>> charToGId{};
		charToGId.reserve(65536);
		{
			uint gIndex;
			char32_t charCode = FT_Get_First_Char(face->face, &gIndex);
			while (gIndex != 0) {
				charToGId.emplace_back(std::make_pair(charCode, gIndex));
				charCode = FT_Get_Next_Char(face->face, charCode, &gIndex);
			}
		}
		if (charToGId.empty()) {
			logger("No charcodes mapping detected from file!! returning!\n");
			return false;
		}
		std::sort(charToGId.begin(), charToGId.end(),
		  [](std::pair<char32_t, uint> a, std::pair<char32_t, uint> b) {
			  return a.second < b.second;
		  });
		face->glyphs.reserve(charToGId.size());
		face->charCodeLookup =
		  std::pmr::unordered_map<char32_t, uint>(&face->pmr_r);
		face->charCodeLookup.reserve(charToGId.size());
		// Not actually setting the render resolution. Just that it sets the
		// returned unit to be in font units. Not using FT_LOAD_NO_SCALE because
		// it sets FT_LOAD_NO_HINTING
		// FT_Set_Pixel_Sizes(face->face, face->_unitPerEM, face->_unitPerEM);
		for (auto& p : charToGId) {
			if (face->glyphs.empty() || face->glyphs.back()._gId != p.second) {
				// get glyph metadata

				/*
				if (FT_Load_Glyph(face->face, p.second,
					  FT_LOAD_NO_BITMAP | FT_LOAD_LINEAR_DESIGN)) {
					throw;
				}
				face->glyphs.emplace_back(_Glyph{p.second, 0,
				  ivec2(face->face->glyph->linearHoriAdvance,
					face->face->glyph->linearHoriAdvance),
				  ivec2(-face->face->glyph->bitmap_left,
					face->face->glyph->bitmap_top),
				  uvec2(face->face->glyph->bitmap.width,
					face->face->glyph->bitmap.rows)});
				*/
				face->glyphs.emplace_back(
				  _Glyph{p.second, 0, ivec2(0), ivec2(0), uvec2(0)});
			}
			face->charCodeLookup.emplace(
			  p.first, uint(face->glyphs.size() - 1));
		}
		face->glyphs.shrink_to_fit();
		logger("Loading completed. Number of registored charCodes: ",
		  face->charCodeLookup.size(),
		  ", Number of registored glyphs: ", face->glyphs.size(), "\n");
	}
	return true;
}
bool font::linkFallbackFont(
  const std::string& fontSet, const std::string& fallbackFontSet) {
	FontSet* tFont = nullptr;
	FontSet* bFont = nullptr;
	for (auto& f : _fontSets) {
		assert(f != nullptr);
		if (f->fontName == fontSet) {
			tFont = f;
			if (tFont != nullptr && bFont != nullptr) break;
		}
		if (f->fontName == fallbackFontSet) {
			bFont = f;
			if (tFont != nullptr && bFont != nullptr) break;
		}
	}
	if (tFont != nullptr && bFont != nullptr) {
		tFont->fallbackFonts.emplace_back(bFont);
		return true;
	}
	return false;
}
bool font::useFont(const std::string& fontSet) {
	if (fontSet.empty()) {
		_targetFont = _defaultFont;
		return true;
	} else {
		for (auto& font : _fontSets) {
			if (font->fontName == fontSet) {
				_targetFont = font;
				return true;
			}
		}
	}
	return false;
}
void font::useStyle(const std::vector<std::string>& style) {
	_targetStyle = 0;
	for (auto& sty : style) {
		uint i;
		for (i = 0; i < sizeof(Style) * 8 && !_style[i].empty(); i++) {
			if (sty == _style[i]) break;
		}
		if (i >= sizeof(Style) * 8) {
			logger(sizeof(Style) * 8, " styles type limit reached! Not adding ",
			  sty, " into style list.\n");
		} else {
			_style[i] = sty;
			_targetStyle += Style(1) << i;
		}
	}
}
bool font::toggleStyle(const std::string& style, bool setTo) {
	uint i;
	for (i = 0; i < sizeof(Style) * 8 && !_style[i].empty(); i++) {
		if (style == _style[i]) {
			bool v = _targetStyle & Style(1) << i;
			if (setTo != v)
				_targetStyle ^= Style(1) << i;	// flap bit if not equal
			return v;
		}
	}
	if (i >= sizeof(Style) * 8) {
		logger(sizeof(Style) * 8, " styles type limit reached! Not adding ",
		  style, " into style list.\n");
		return false;
	} else {
		_style[i] = style;
		if (setTo) _targetStyle |= Style(1) << i;
		return false;
	}
}
bool font::setDefaultFontSet(const std::string& fontSet) {
	for (auto& font : _fontSets) {
		if (font->fontName == fontSet) {
			_defaultFont = font;
			return true;
		}
	}
	return false;
}

FontFaceData font::getFontFaceData(FontFace* fontFace, float pointSize) {
	return FontFaceData{._font = fontFace,
	  .lineHeight = float(fontFace->_lineHeightUnit)
				  / float(fontFace->_unitPerEM) * pointSize / 72.f
				  * gl::target->pixelPerInch.y / gl::target->viewport.w * 2.f,
	  .maxDescend = float(fontFace->_maxDescendUnit)
				  / float(fontFace->_unitPerEM) * pointSize / 72.f
				  * gl::target->pixelPerInch.y / gl::target->viewport.w * 2.f,
	  .maxAccend = float(fontFace->_maxAccendUnit) / float(fontFace->_unitPerEM)
				 * pointSize / 72.f * gl::target->pixelPerInch.y
				 / gl::target->viewport.w * 2.f,
	  .maxLeft = float(fontFace->_maxLeftUnit) / float(fontFace->_unitPerEM)
			   * pointSize / 72.f * gl::target->pixelPerInch.x
			   / gl::target->viewport.z * 2.f,
	  .maxRight = float(fontFace->_maxRightUnit) / float(fontFace->_unitPerEM)
				* pointSize / 72.f * gl::target->pixelPerInch.x
				/ gl::target->viewport.z * 2.f,
	  .maxAdvance = vec2(fontFace->_maxAdvanceUnit) / vec2(fontFace->_unitPerEM)
				  * pointSize / 72.f * gl::target->pixelPerInch
				  / vec2(gl::target->viewport.z, gl::target->viewport.w) * 2.f,
	  .underlinePosition = float(fontFace->_underlinePositionUnit)
						 / float(fontFace->_unitPerEM) * pointSize / 72.f
						 * gl::target->pixelPerInch.y / gl::target->viewport.w
						 * 2.f,
	  .underlineThickness = float(fontFace->_underlineThicknessUnit)
						  / float(fontFace->_unitPerEM) * pointSize / 72.f
						  * gl::target->pixelPerInch.y / gl::target->viewport.w
						  * 2.f};
}
GlyphData font::getGlyphData(
  FontFace* fontFace, GlyphId gId, float pointSize) {  // 5 (1,1) -> (5,5)
	auto& g = fontFace->glyphs[gId];

	// Load the data of the glyph as it is now needed
	if (g._renderedPointSize == 0) {
		auto& ft_f = fontFace->face;
		FT_Set_Pixel_Sizes(ft_f, fontFace->_unitPerEM, fontFace->_unitPerEM);
		if (FT_Load_Glyph(
			  ft_f, g._gId, FT_LOAD_NO_BITMAP | FT_LOAD_LINEAR_DESIGN)) {
			// logger(
			//  "Font Engine Error: Failed to read glyph data: ", g._gId, "\n");
			throw;	// TODO: Needs actual error handling
		}
		g._advanceUnit =
		  ivec2(ft_f->glyph->linearHoriAdvance, ft_f->glyph->linearHoriAdvance);
		g._bearingUnit =
		  ivec2(-ft_f->glyph->bitmap_left, ft_f->glyph->bitmap_top);
		g._sizeUnit =
		  uvec2(ft_f->glyph->bitmap.width, ft_f->glyph->bitmap.rows);
	}

	if (fontFace->texturePPI == vec2{0})
		fontFace->texturePPI = gl::target->pixelPerInch;
	// >1 = enlarge (not enough resolution),  <1 = minify (enough resolution)
	// NOTE: Divade by zero may cause issue if not using IEEE float!
	vec2 textureRelativeScale =
	  (pointSize * gl::target->pixelPerInch)
	  / (float(g._renderedPointSize) * fontFace->texturePPI);
	float& higherValue = textureRelativeScale.x > textureRelativeScale.y
						 ? textureRelativeScale.x
						 : textureRelativeScale.y;

	if (textureRelativeScale.x > textureRelativeScale.y) {
		if (g._renderedPointSize != _size.back()
			&& (g._renderedPointSize == 0 || textureRelativeScale.x > 1)) {
			uint targetSize = uint(
			  ceil(pointSize
				   * (gl::target->pixelPerInch.x / fontFace->texturePPI.x)));
			auto sizeIt =
			  std::lower_bound(_size.begin(), _size.end(), targetSize);
			if (sizeIt == _size.end()) sizeIt--;
			g._renderedPointSize = *sizeIt;
			{
				auto it = _requireRender.find(fontFace);
				if (it == _requireRender.end()) {
					it = _requireRender
						   .emplace(fontFace,
							 std::pmr::unordered_map<font::GlyphId, uint>(
							   &_pmr_r_reRender))
						   .first;
				}
				it->second.emplace(gId, *sizeIt);
			}
			// recaculate scale
			textureRelativeScale =
			  (pointSize * gl::target->pixelPerInch)
			  / (float(g._renderedPointSize) * fontFace->texturePPI);
		}
	} else {
		if (g._renderedPointSize != _size.back()
			&& (g._renderedPointSize == 0 || textureRelativeScale.y > 1)) {
			uint targetSize = uint(
			  ceil(pointSize
				   * (gl::target->pixelPerInch.y / fontFace->texturePPI.y)));
			auto sizeIt =
			  std::lower_bound(_size.begin(), _size.end(), targetSize);
			if (sizeIt == _size.end()) sizeIt--;
			g._renderedPointSize = *sizeIt;
			{
				auto it = _requireRender.find(fontFace);
				if (it == _requireRender.end()) {
					it = _requireRender
						   .emplace(fontFace,
							 std::pmr::unordered_map<font::GlyphId, uint>(
							   &_pmr_r_reRender))
						   .first;
				}
				it->second.emplace(gId, *sizeIt);
			}
			// recaculate scale
			textureRelativeScale =
			  (pointSize * gl::target->pixelPerInch)
			  / (float(g._renderedPointSize) * fontFace->texturePPI);
		}
	}
	assert(textureRelativeScale.x <= 1 && textureRelativeScale.y <= 1);

	auto result = GlyphData{.gId = gId,
	  .descend = float(int(g._sizeUnit.y) - int(g._bearingUnit.y))
			   / float(fontFace->_unitPerEM) * pointSize / 72.f
			   * gl::target->pixelPerInch.y / gl::target->viewport.w * 2.f,
	  .accend = float(g._bearingUnit.y) / float(fontFace->_unitPerEM)
			  * pointSize / 72.f * gl::target->pixelPerInch.y
			  / gl::target->viewport.w * 2.f,
	  .left = float(g._bearingUnit.x) / float(fontFace->_unitPerEM) * pointSize
			/ 72.f * gl::target->pixelPerInch.x / gl::target->viewport.z * 2.f,
	  .right = float(int(g._sizeUnit.x) - int(g._bearingUnit.x))
			 / float(fontFace->_unitPerEM) * pointSize / 72.f
			 * gl::target->pixelPerInch.x / gl::target->viewport.z * 2.f,
	  .advance = gl::target->normalizeLength(
		vec2(g._advanceUnit) / float(fontFace->_unitPerEM) * pointSize / 72.f
		* gl::target->pixelPerInch),
	  .textureResolutionScale = textureRelativeScale};
#ifdef FONT_ASSERT_BREAKING
	assert(result.gId != -1);
	assert(result.advance.x > -2 && result.advance.x < 2);
	assert(result.advance.y > -2 && result.advance.y < 2);
	assert(result.descend >= -1 && result.descend < 1);
	assert(result.accend >= -1 && result.accend < 1);
	assert(result.left >= -1 && result.left < 1);
	assert(result.right >= -1 && result.right < 1);
#endif
	return result;
}
void font::renderRequiredGlyph() {
	struct alignas(vec2) glGlyph {
		// uint gId = -1;
		uvec2 origin = {0, 0};	  // Texture origin (pixel)
		uvec2 size = {0, 0};	  // Texture size (pixel)
		vec2 emBearing = {0, 0};  // bearing (in em size)
		vec2 emSize = {0, 0};	  // size (relative to the em square)
	};
	struct alignas(uint) glData {
		uint identifier = 43;  // New identifier: textRender v 1.1
		uint size;
		glGlyph glyphs[];  // NOTE: using non statandard extension
	};
	for (auto& fontPair : _requireRender) {
		auto& font = fontPair.first;
		auto& glyphs = fontPair.second;
		if (glyphs.empty()) continue;
		if (font->textureShelf == nullptr) {
			font->textureShelf =
			  new mapbox::ShelfPack(_texSize[0], _texSize[0]);
			font->texture = new gl::Texture2D();
			font->texture->setTextureSamplingFilter(
			  gl::Texture::SamplingFilter::linear,
			  gl::Texture::SamplingFilter::linear);
			font->texture->setFormat(GL_R8, _texSize[0], _texSize[0], 1);
		};
		auto& shelf = *font->textureShelf;
		glData* ssboData;
		if (font->ssbo == nullptr) {
			font->ssbo = new gl::ShaderStorageBuffer();
			font->ssbo->setFormatAndData(
			  sizeof(glData) + sizeof(glGlyph) * font->glyphs.size(),
			  GL_MAP_WRITE_BIT);
			ssboData = (glData*)font->ssbo->map(gl::MapAccess::WriteOnly);
			ssboData->identifier = 43;
			ssboData->size = font->glyphs.size();
		} else {
			ssboData = (glData*)font->ssbo->map(gl::MapAccess::WriteOnly);
		}

		for (auto& gPair : glyphs) {
			mapbox::Bin* bin = shelf.getBin(gPair.first);
			if (bin != nullptr) {
				// NOTE MEMLEAK: Not sure how mapbox::shelfpack works in
				// unref-ing bins. Does it only ever deallocate memory when
				// shelf is destroyed? if so we have a memory problem here if
				// you run the game too long.
				shelf.unref(*bin);
			}
			_Glyph& _glyph = font->glyphs[gPair.first];
			uint& _pointSize = gPair.second;
			auto& g = ssboData->glyphs[gPair.first];
			// g.gId = gPair.first;
			uvec2 pixelSize =
			  uvec2(ceil(float(_pointSize) * font->texturePPI.x / 72.f),
				ceil(float(_pointSize) * font->texturePPI.y / 72.f));
			pixelSize *= 2;	 // Subsampling rate
			FT_Set_Pixel_Sizes(font->face, pixelSize.x, pixelSize.y);
			if (FT_Load_Glyph(font->face, _glyph._gId, FT_LOAD_DEFAULT)) {
				logger("Warning: Glyph id ", gPair.first, " at ",
				  font->face->family_name, " ", font->face->style_name,
				  " failed to load. Skipping...");
				g.origin = uvec2{0, 0};
				g.size = uvec2{0, 0};
				g.emBearing = vec2{0, 0};
				g.emSize = vec2{0, 0};
				continue;
			}
			if (FT_Render_Glyph(font->face->glyph,
				  FT_RENDER_MODE_NORMAL)) {	 // change this for color
											 // rendering)) {
				logger("Warning: Glyph id ", gPair.first, " at ",
				  font->face->family_name, " ", font->face->style_name,
				  " failed to render. Skipping...");
				g.origin = uvec2{0, 0};
				g.size = uvec2{0, 0};
				g.emBearing = vec2{0, 0};
				g.emSize = vec2{0, 0};
				continue;
			}
			g.size = uvec2{
			  font->face->glyph->bitmap.width, font->face->glyph->bitmap.rows};
			g.emBearing =
			  vec2{vec2(_glyph._bearingUnit) / float(font->_unitPerEM)};
			g.emSize = vec2(vec2(_glyph._sizeUnit) / float(font->_unitPerEM));
			if (g.size.x == 0 || g.size.y == 0) {
				continue;  // a glyph that's empty (like space:' ')
			}

			bin =
			  shelf.packOne(gPair.first, g.size.x + GLYP_SPRITE_BORDER_WIDTH,
				g.size.y + GLYP_SPRITE_BORDER_WIDTH);
			if (bin == nullptr) {
				// do resize
				uint oldSize = shelf.width();
				while (bin == nullptr) {
					if (shelf.width() == MAXTEXTURESIZE) break;
					// get new size
					uint i;
					for (i = 0; i < _texSize.size(); i++) {
						if (_texSize[i] > shelf.width()) break;
					}

					shelf.resize(_texSize[i], _texSize[i]);	 //????
					bin = shelf.packOne(gPair.first,
					  g.size.x + GLYP_SPRITE_BORDER_WIDTH,
					  g.size.y + GLYP_SPRITE_BORDER_WIDTH);
				}
				// check if resize successful
				if (bin == nullptr) {
					bool successful = shelf.resize(oldSize, oldSize);
					assert(successful);
					logger("Max texture size reached. Font glyph sprite map "
						   "creation failed. Skipping glyph sprite...\n");
					g.origin = uvec2{0, 0};
					g.size = uvec2{0, 0};
					g.emBearing = vec2{0, 0};
					g.emSize = vec2{0, 0};
					continue;
				}
				// resize texture
				assert(font->texture != nullptr);
				gl::Texture2D* newTexture = new gl::Texture2D();
				newTexture->setFormat(GL_R8, shelf.width(), shelf.width(), 1);
				newTexture->cloneData(
				  font->texture, uvec2{0}, uvec2{oldSize}, 0);
				font->texture->release();
				font->texture = newTexture;
				font->texture->setTextureSamplingFilter(
				  gl::Texture::SamplingFilter::linear,
				  gl::Texture::SamplingFilter::linear);
			}
			assert(bin != nullptr);
			g.origin = uvec2(bin->x, bin->y);
			glPixelStorei(GL_UNPACK_ALIGNMENT,
			  1);  // IMPORTANT!! Without this, it WILL cause seg fault!
			font->texture->setData(bin->x, bin->y, g.size.x, g.size.y, 0,
			  GL_RED, gl::DataType::UnsignedByte,
			  font->face->glyph->bitmap.buffer);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			// logger("Glyph id ", gPair.first, " at ", font->face->family_name,
			// " ", font->face->style_name, " render successfully.");
		}
		font->ssbo->unmap();
	}
	_requireRender.clear();
	_pmr_r_reRender.release();
}
void font::_renderBatch(
  FontFace* f, std::vector<RenderData> d, float pointSize) {
	if (d.empty()) return;
	Swapper _{gl::target->vao, _vao};
	Swapper __{gl::target->useBlend, true};
	Swapper ___{gl::target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
	Swapper ____{gl::target->spo, _textShader};
	gl::target->spo->use();
	gl::target->spo->setUniform(
	  "emSize", gl::target->normalizeLength(
				  (pointSize / 72.f) * gl::target->pixelPerInch));
	gl::target->spo->setUniform("texColor", vec4(0.0, 0.0, 0.0, 1.0));
	gl::target->bind(f->ssbo);
	gl::target->bind(f->texture, 0);
	_vbo->setFormatAndData(sizeof(RenderData) * d.size(), 0, d.data());
	gl::target->drawArrays(gl::GeomType::Points, 0, d.size());
	_vbo->release();
	_vbo = new gl::VertexBuffer();
	gl::target->vao->setBufferBinding(0, _vbo, sizeof(RenderData));
}
std::pair<FontFace*, GlyphId> font::getGlyphFromChar(char32_t c, char32_t d) {
	std::pair<FontFace*, GlyphId> result;
	std::set<FontSet*> searched{};
	if (_targetFont != nullptr) {
		if (_checkFont(c, _targetFont, searched, result)) return result;
	}
	if (_defaultFont != nullptr) {
		if (_checkFont(c, _defaultFont, searched, result)) return result;
	}
	searched.clear();
	if (_targetFont != nullptr) {
		if (_checkFont(d, _targetFont, searched, result)) return result;
	}
	if (_defaultFont != nullptr) {
		if (_checkFont(d, _defaultFont, searched, result)) return result;
	}
	return std::make_pair(nullptr, 0);
}

struct CharFlags {
	bool nextLine	   : 1 = false;
	bool tab		   : 1 = false;
	bool noRender	   : 1 = false;
	bool controlCommend: 1 = false;
};
struct MultiCharState {
	bool windowsSkipNextLine: 1 = false;
};
inline CharFlags _charDecode(char32_t c, MultiCharState& s) {
	constexpr char32_t CONTROL_PICTURES = u'\u2400';
	CharFlags f;
	switch (c) {
	case U'\0': f.noRender = true; break;

	case U'\t': f.noRender = true; [[fallthrough]];
	case U'\t' + CONTROL_PICTURES:
	case _dChar[U'\t']: f.tab = true; break;

	case U'\n': f.noRender = true; [[fallthrough]];
	case U'\n' + CONTROL_PICTURES:
	case _dChar[U'\n']:
		if (!s.windowsSkipNextLine) f.nextLine = true;
		break;

	case U'\v': f.noRender = true; [[fallthrough]];
	case U'\v' + CONTROL_PICTURES:
	case _dChar[U'\v']: f.nextLine = true; break;

	case U'\f': f.noRender = true; [[fallthrough]];
	case U'\f' + CONTROL_PICTURES:
	case _dChar[U'\f']: f.nextLine = true; break;

	case U'\r': f.noRender = true; [[fallthrough]];
	case U'\r' + CONTROL_PICTURES:
	case _dChar[U'\r']: f.nextLine = true; break;

	case U'\u0085':
	case U'\u2828':
	case U'\u2029':
		f.noRender = true;
		f.nextLine = true;
		break;

	case U'\u009f': f.noRender = true; [[fallthrough]];
	case U'\u009f' + CONTROL_PICTURES: f.controlCommend = true; break;

	default: break;
	}
	s.windowsSkipNextLine = (c == U'\r');
	return f;
}

void font::_drawStringFunction(
  Reader& r, Output& output, vec2 topLeftPos, vec2 maxSize) {
	assert(!r.chars.empty());
	output.reserve(r.fontData.size());
	float maxAccend = 0;
	float lineHeight = 0;
	for (uint i = 0; i < r.fontData.size(); i++) {
		output.emplace_back();
		if (r.fontData.at(i).first.maxAccend > maxAccend)
			maxAccend = r.fontData.at(i).first.maxAccend;
		if (r.fontData.at(i).first.lineHeight > lineHeight)
			lineHeight = r.fontData.at(i).first.lineHeight;
	}
	auto charIt = r.chars.begin();
	auto space = charIt++;
	// assert(std::get<0>(*space) == U' ');
	float spaceAdvance = r.fontData.at(std::get<1>(*space))
						   .second.at(std::get<2>(*space))
						   .advance.x;
	vec2 drawHead = topLeftPos;
	drawHead.y -= maxAccend;
	MultiCharState s{};
	while (charIt != r.chars.end()) {
		while (charIt != r.chars.end()) {
			char32_t c = std::get<0>(*charIt);
			uint fontI = std::get<1>(*charIt);
			uint glyphI = std::get<2>(*charIt);
			font::GlyphId gId = r.indexLookup.at(fontI).second.at(glyphI);
			if (drawHead.x + r.fontData.at(fontI).second.at(glyphI).right
				> topLeftPos.x + maxSize.x)
				goto NextLine;

			auto flag = _charDecode(c, s);

			if (fontI != -1 && !flag.noRender) {
				if (drawHead.x - r.fontData.at(fontI).second.at(glyphI).left
					< topLeftPos.x)
					drawHead.x += r.fontData.at(fontI).second.at(glyphI).left;
				// gl::drawRectangle(nullptr, drawHead, vec2(0.01));
				output.at(fontI).emplace_back(
				  r.indexLookup.at(fontI).second.at(glyphI), drawHead);
				drawHead.x += r.fontData.at(fontI).second.at(glyphI).advance.x;
			}
			if (flag.tab) { drawHead.x += spaceAdvance * TAB_SPACE; }
			charIt++;
			if (flag.nextLine) break;
		}
NextLine:
		drawHead.y -= lineHeight;
		drawHead.x = topLeftPos.x;
		if (drawHead.y + maxAccend < topLeftPos.y - maxSize.y)
			break;	// out of space. drawing at below screen
	}
}

void font::_drawStringCentreFunction(
  Reader& r, Output& output, vec2 topLeftPos, vec2 maxSize) {
	assert(!r.chars.empty());
	output.reserve(r.fontData.size());
	float maxAccend = 0;
	float lineHeight = 0;
	for (uint i = 0; i < r.fontData.size(); i++) {
		output.emplace_back();
		if (r.fontData.at(i).first.maxAccend > maxAccend)
			maxAccend = r.fontData.at(i).first.maxAccend;
		if (r.fontData.at(i).first.lineHeight > lineHeight)
			lineHeight = r.fontData.at(i).first.lineHeight;
	}
	auto charIt = r.chars.begin();
	auto space = charIt++;
	// assert(std::get<0>(*space) == U' ');
	float spaceAdvance = r.fontData.at(std::get<1>(*space))
						   .second.at(std::get<2>(*space))
						   .advance.x;
	vec2 drawHead = topLeftPos;
	drawHead.y -= maxAccend;
	std::vector<std::tuple<uint, GlyphId, vec2>> currentLine{};
	currentLine.reserve(128);
	MultiCharState s{};

	while (charIt != r.chars.end()) {
		float lineLeftPos = 0;
		float lineRightPos = 0;
		while (charIt != r.chars.end()) {
			char32_t c = std::get<0>(*charIt);
			uint fontI = std::get<1>(*charIt);
			uint glyphI = std::get<2>(*charIt);
			font::GlyphId gId = r.indexLookup.at(fontI).second.at(glyphI);
			if (drawHead.x + r.fontData.at(fontI).second.at(glyphI).right
				> topLeftPos.x + maxSize.x)
				goto NextLine;

			auto flag = _charDecode(c, s);

			if (fontI != -1 && !flag.noRender) {
				auto& gData = r.fontData.at(fontI).second.at(glyphI);
				if (drawHead.x - gData.left < topLeftPos.x)
					drawHead.x += gData.left;
				// gl::drawRectangle(nullptr, drawHead, vec2(0.01));
				if (currentLine.empty()) {
					lineLeftPos = drawHead.x - gData.left;
					lineRightPos = drawHead.x + gData.right;
				}
				if (drawHead.x - gData.left < lineLeftPos)
					lineLeftPos = drawHead.x - gData.left;
				if (drawHead.x + gData.right > lineRightPos)
					lineRightPos = drawHead.x + gData.right;
				currentLine.emplace_back(
				  fontI, r.indexLookup.at(fontI).second.at(glyphI), drawHead);
				drawHead.x += r.fontData.at(fontI).second.at(glyphI).advance.x;
			}
			if (flag.tab) { drawHead.x += spaceAdvance * TAB_SPACE; }
			charIt++;
			if (flag.nextLine) break;
		}
NextLine:
		drawHead.y -= lineHeight;
		drawHead.x = topLeftPos.x;
		{
			float lineWidth = lineRightPos - lineLeftPos;
			float xShift =
			  lineLeftPos - topLeftPos.x + maxSize.x / 2.f - lineWidth / 2.f;
			for (auto& tuple : currentLine) {
				output.at(std::get<0>(tuple))
				  .emplace_back(
					std::get<1>(tuple), std::get<2>(tuple) + vec2(xShift, 0.f));
			}
			currentLine.clear();
		}
		if (drawHead.y + maxAccend < topLeftPos.y - maxSize.y)
			break;	// out of space. drawing at below screen
	}
}
#endif
#ifdef USE_VULKAN
#	include "VK.h"
#endif