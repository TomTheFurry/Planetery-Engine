#include "font2.h"
#include "GL.h"
#include "ShaderProgram.h"
#include "ThreadEvents.h"
#include "Logger.h"

#include <set>
#include <unordered_map>
#include <glad/glad.h>
#include <freetype/freetype.h>
#include <shelf-pack.hpp>
#include <array>

using namespace font;

FT_Library ftLib{};

struct _Glyph {
	uint _gId;
	uint _renderedPointSize;
	ivec2 _advanceUnit;
	ivec2 _bearingUnit;
	uvec2 _sizeUnit;
};

typedef unsigned int Style;
static std::array<std::string,sizeof(Style)*8> style;

extern class font::FontFace {
public:
	Style style;
	vec2 texturePPI;
	uint _lineHeightUnit = 0;
	uint _unitPerEM = 0;
	gl::Texture2D* texture = nullptr;
	gl::ShaderStorageBuffer* ssbo = nullptr;
	mapbox::ShelfPack* textureShelf = nullptr;
	FT_Face face = nullptr;
	std::unordered_map<char32_t, uint> charCodeLookup{};
	std::vector<_Glyph> glyphs{};
};
extern class font::FontSet {
public:
	std::string fontName;
	std::vector<FontSet*> fallbackFonts{};
	std::vector<FontFace*> fontFaces{};
	std::unordered_map<Style, FontFace*> cachedStyleMap{};

	FontFace* getFace(Style style) {
		auto it = cachedStyleMap.find(style);
		if (it!=cachedStyleMap.end()) return it->second;
		assert(!fontFaces.empty());
		FontFace* mostMatch = fontFaces[0];
		uint matching = __popcnt(style ^ fontFaces[0]->style);
		for (auto face : fontFaces) {
			//NOTE: This requires popcnt() which is not in standard!!
			uint v = __popcnt(style ^ face->style);
			if (v>matching) {
				matching = v;
				mostMatch = face;
			}
		}
		cachedStyleMap.emplace(style, mostMatch);
		return mostMatch;
	}
};

static std::vector<std::string> styles;
static std::vector<FontSet*> fontSets;
static FontSet* targetFont;
static FontSet* defaultFont;
ShaderProgram* textShader = nullptr;
gl::VertexAttributeArray* vao = nullptr;
gl::VertexBuffer* vbo = nullptr;

constexpr int FT_LOAD_FLAG = FT_LOAD_LINEAR_DESIGN;
static const std::array<const uint, 10> _size {{4, 8, 12, 24, 36, 48, 60, 72, 144, 288}};
static std::array<uint, 7> _texSize{{1024, 4096, 16384, 65536, 262144, 1048576, uint(-1)}};
static uint MAXTEXTURESIZE = 0;
static FT_Library _ftLib = nullptr;

inline FontSet* _getFontSet(const std::string& name) {
	for (auto ptr : fontSets) {
		if (ptr->fontName==name) return ptr;
	} return nullptr;
}


void font::init() {
	MAXTEXTURESIZE = gl::getMaxTextureSize();
	auto it = std::lower_bound(_texSize.begin(), _texSize.end(), MAXTEXTURESIZE);
	*it = MAXTEXTURESIZE;
	while (*(++it)!=uint(-1)) {
		*it = uint(-1);
	}
	if (FT_Init_FreeType(&_ftLib)) {
		logger("Failed to init FT Library!!\n");
		throw "FT_Lib_Error";
	};
}

std::vector<const std::string*> font::getAllFontSets() {
	std::vector<const std::string*> v{};
	v.reserve(fontSets.size());
	for (auto& ptr : fontSets) {
		v.emplace_back(&ptr->fontName);
	}
	return v;
}
const std::vector<std::string>& font::getAllFontStyles() {
	return styles;
}


bool font::addFont(const std::string& fileLocation, const std::string& fontSetName, const std::vector<std::string>& style) {
	logger("Loading font file: ", fileLocation, "...\n");
	Style styl = 0;
	for (auto& sty : style) {
		uint i;
		for (i = 0; i<styles.size(); i++) {
			if (sty==styles[i]) break;
		}
		styles.emplace_back(sty);
		styl += i<<i;
	}
	FontSet* fontSet = _getFontSet(fontSetName);
	if (fontSet==nullptr) {
		fontSet = fontSets.emplace_back(new FontSet{fontSetName});
	}
	if (!fontSet->fontFaces.empty() && fontSet->getFace(styl)->style==styl) {
		logger("FontSet ", fontSetName, " with this style aready exist. Unload that first!\n");
		return false;
	}
	FontFace* face = fontSet->fontFaces.emplace_back(new FontFace{styl, gl::target->pixelPerInch});
	if (FT_New_Face(_ftLib, fileLocation.data(), 0, &face->face)) {
		logger("Loading failed.\n");
		return false;
	};

	{	//Logging some stuff about fontface and set stuff in face
		face->_unitPerEM = face->face->units_per_EM;
		face->_lineHeightUnit = face->face->height;
		logger.newLayer();
		logger << "Face meta data loaded:\n";
		logger(face->face->family_name, ": ", face->face->style_name, "\n");
		logger("Scalable: ", bool(face->face->face_flags | FT_FACE_FLAG_SCALABLE), "\n");
		logger("Can be horizontal: ", bool(face->face->face_flags | FT_FACE_FLAG_HORIZONTAL), "\n");
		logger("Can be vertical: ", bool(face->face->face_flags | FT_FACE_FLAG_VERTICAL), "\n");
		logger("Has color: ", bool(face->face->face_flags | FT_FACE_FLAG_COLOR), "\n");
		if (face->face->num_faces>1) logger("Warning!! ", face->face->num_faces, " faces in a single font file detected! Currently only supports reading one font face per file!\n");
		if (face->face->charmap->encoding != FT_ENCODING_UNICODE) logger("Warning!! This font file does not seem to support unicode! Currently only supports unicode charcode mapping! Treating it as unicode mapping.\n");
		logger.closeLayer();
	}
	
	{   //Load charmap
		logger("Now loading charmaps. Will take some time if font supports large number of charcodes...\n");
		std::vector<std::pair<char32_t, uint>> charToGId{};
		charToGId.reserve(65536);
		{
			uint gIndex;
			char32_t charCode = FT_Get_First_Char(face->face, &gIndex);
			while (gIndex!=0) {
				charToGId.emplace_back(std::make_pair(charCode, gIndex));
				charCode = FT_Get_Next_Char(face->face, charCode, &gIndex);
			}
		}
		if (charToGId.empty()) {
			logger("No charcodes mapping detected from file!! returning!\n");
			return false;
		}
		std::sort(charToGId.begin(), charToGId.end(), [](std::pair<char32_t, uint>a, std::pair<char32_t, uint>b) {return a.second<b.second; });
		face->glyphs.reserve(charToGId.size());
		//Not actually setting the render resolution. Just that it sets the returned unit to be in font units. Not using FT_LOAD_NO_SCALE because it sets FT_LOAD_NO_HINTING
		FT_Set_Pixel_Sizes(face->face, face->_unitPerEM, face->_unitPerEM);
		for (auto& p : charToGId) {
			if (face->glyphs.empty() || face->glyphs.back()._gId!=p.second) {
				//get glyph metadata
				if (FT_Load_Glyph(face->face, p.second, FT_LOAD_NO_BITMAP | FT_LOAD_LINEAR_DESIGN)) {
					throw;
				}
				face->glyphs.emplace_back(_Glyph{
					p.second,
					0,
					ivec2(face->face->glyph->linearHoriAdvance, face->face->glyph->linearHoriAdvance),
					ivec2(-face->face->glyph->bitmap_left, face->face->glyph->bitmap_top),
					uvec2(face->face->glyph->bitmap.width, face->face->glyph->bitmap.rows)}
				);
			}
			face->charCodeLookup.emplace(p.first, face->glyphs.size()-1);
		}
		face->glyphs.shrink_to_fit();
		logger("Loading completed. Number of registored charCodes: ", face->charCodeLookup.size(), ", Number of registored glyphs: ", face->glyphs.size(), "\n");
	}
	return true;
}

FontFaceData font::getFontFaceData(FontFace* fontFace, float pointSize) {
	return FontFaceData{
		fontFace,
		float(double(fontFace->_lineHeightUnit)*double(pointSize)/double(fontFace->_unitPerEM)/72.*double(gl::target->pixelPerInch.y)/double(gl::target->viewport.w)*2.-1.)
	};
}

thread_local std::unordered_map<FontFace*,std::unordered_map<GlyphId, uint>>_requireRender{};

GlyphData font::getGlyphData(FontFace* fontFace, GlyphId gId, float pointSize) { //5 (1,1) -> (5,5)
	auto& g = fontFace->glyphs[gId];

	// >1 = enlarge (not enough resolution),  <1 = minify (enough resolution)
	//NOTE: Divade by zero may cause issue if not using IEEE float!
	vec2 textureRelativeScale = (pointSize*gl::target->pixelPerInch) / (float(g._renderedPointSize)*fontFace->texturePPI);
	float& higherValue = textureRelativeScale.x>textureRelativeScale.y ? textureRelativeScale.x : textureRelativeScale.y;
	if (g._renderedPointSize!=_size.back() && (higherValue>1)) { // the '1' is the thresold point where it decides it doesn't have enough resolution and requires reRendering
		const auto& SIZE = _size;
		auto sizeIt = std::lower_bound(SIZE.begin(), SIZE.end(), uint(ceil(g._renderedPointSize*higherValue)));
		if (sizeIt==SIZE.end()) sizeIt--;
		g._renderedPointSize = *sizeIt;
		_requireRender[fontFace].emplace(gId, *sizeIt);
		//recaculate scale
		textureRelativeScale = (pointSize*gl::target->pixelPerInch) / (float(g._renderedPointSize)*fontFace->texturePPI);
		assert(textureRelativeScale.x<=1 && textureRelativeScale.y<=1);
	}
	return GlyphData{
		gId,
		gl::target->normalize(dvec2(g._advanceUnit)*(double(pointSize)/double(fontFace->_unitPerEM)/72.)*dvec2(gl::target->pixelPerInch)),
		textureRelativeScale
	};
}

void font::renderRequiredGlyph() {
	struct glGlyph {
		uvec2 origin; //Texture origin (pixel)
		uvec2 size; //Texture size (pixel)
		vec2 bearing; //relative bearing (relative to size)
	};
	struct glData {
		uint identifier = 43; //New identifier: textRender v 1.1
		uint size;
		glGlyph glyphs[];
	};
	struct _Packing {
		GlyphId gId;
		float size;
		mapbox::Bin* bin;
	};
	for (auto& fontPair : _requireRender) {
		auto& font = fontPair.first;
		auto& glyphs = fontPair.second;
		if (glyphs.empty()) continue;
		if (font->textureShelf == nullptr) {
			font->textureShelf = new mapbox::ShelfPack(_texSize[0], _texSize[0]);
		};
		auto& shelf = *font->textureShelf;
		std::vector<_Packing> packing{};
		packing.reserve(glyphs.size());
		for (auto& gPair : glyphs) {
			mapbox::Bin* oldBin = shelf.getBin(gPair.first);
			if (oldBin != nullptr) {
				shelf.unref(*oldBin);
			}



			//oldBin->w = 

			//packing.emplace_back(_Packing{gPair.first,gPair.second,oldBin});
		}
	}
}

void font::_renderBatch(FontFace* f, std::vector<_RenderData> d, float pointSize) {
	textShader->enable();
	gl::target->bind(f->ssbo);
	gl::target->bind(f->texture, 0);
	vbo->setFormatAndData(sizeof(_RenderData)*d.size(), 0, d.data());
	gl::target->drawArrays(GL_POINTS, 0, d.size());
	vbo->release();
	vbo = new gl::VertexBuffer();
	vao->setBufferBinding(0, {vbo,sizeof(_RenderData)});
}

std::pair<FontFace*, GlyphId> font::getGlyphFromChar(char32_t c) {
	std::set<FontFace*> searched{};
	FontFace* font = targetFont->getFace(0);
	while (font != nullptr) {
		auto itS = searched.lower_bound(font);
		if (itS != searched.end() && *itS == font) break;
		searched.emplace_hint(itS, font);
		auto it = font->charCodeLookup.find(c);
		if (it != font->charCodeLookup.end())
			return std::make_pair(font, it->second);
		font = font->fallbackFont;
	}
	font = defaultFont;
	while (font != nullptr) {
		auto itS = searched.lower_bound(font);
		if (itS != searched.end() && *itS == font) break;
		searched.emplace_hint(itS, font);
		auto it = font->charCodeLookup.find(c);
		if (it != font->charCodeLookup.end())
			return std::make_pair(font, it->second);
		font = font->fallbackFont;
	}
	return std::make_pair(nullptr, 0);
}