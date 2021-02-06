#include "font.h"
#include "GL.h"
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

class font::FontFace {
public:
	FontSet* fontSet;
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
class font::FontSet {
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
	~FontSet() {
		for (auto faces : fontFaces) {
			delete faces;
		}
	}
};

//TODO: invalidate all FontSet.cachedStyleMap on changing _style.
static std::array<std::string, sizeof(Style)*8> _style;
static std::vector<FontSet*> _fontSets;
static FontSet* _targetFont = nullptr;
static FontSet* _defaultFont = nullptr;
static Style _targetStyle = 0;
static gl::ShaderProgram* _textShader = nullptr;
static gl::VertexAttributeArray* _vao = nullptr;
static gl::VertexBuffer* _vbo = nullptr;
static const std::array<const uint, 14> _size {{2, 4, 6, 8, 12, 16, 20, 24, 36, 48, 60, 72, 144, 288}};
static std::array<uint, 7> _texSize{{1024, 4096, 16384, 65536, 262144, 1048576, uint(-1)}};
static uint MAXTEXTURESIZE = 0;
static FT_Library _ftLib = nullptr;
constexpr uint GLYP_SPRITE_BORDER_WIDTH = 2;
thread_local std::unordered_map<FontFace*, std::unordered_map<GlyphId, uint>>_requireRender{};

inline FontSet* _getFontSet(const std::string& name) {
	for (auto ptr : _fontSets) {
		if (ptr->fontName==name) return ptr;
	} return nullptr;
}
inline bool _checkFont(char32_t c, FontSet* f, std::set<FontSet*>& searched, std::pair<FontFace*, GlyphId>& result) {
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


void font::init() {
	MAXTEXTURESIZE = gl::getMaxTextureSize();
	auto it = std::lower_bound(_texSize.begin(), _texSize.end(), MAXTEXTURESIZE);
	*it = MAXTEXTURESIZE;
	while (*(++it)!=uint(-1)) {
		*it = uint(-1);
	}
	for (auto& str : _style) {
		str = std::string{};
	}
	if (FT_Init_FreeType(&_ftLib)) {
		logger("Failed to init FT Library!!\n");
		throw "FT_Lib_Error";
	};
	_textShader = gl::makeShaderProgram("textRender2", true);

	if (!addFont("LastResort", "fonts/LastResortHE-Regular.ttf")) throw "LastResort font loading failed";
	if (!addFont("Arial", "fonts/arialuni.ttf")) throw "Default font loading failed";
	if (!setDefaultFontSet("Arial")) throw "Default font linking failed";
	if (!linkFallbackFont("Arial", "LastResort")) throw "Default font to fallback font linking failed";
	assert(_defaultFont!=nullptr);
	//addFont("OrangeJuice", "fonts/orange juice.ttf");
	//addFont("RemachineScript", "fonts/RemachineScript.ttf");
	//addFont("AeroviasBrasilNF", "fonts/AeroviasBrasilNF.ttf");
	//addFont("Sketsaramadhan", "fonts/Sketsaramadhan-nRLAO.otf");
	//addFont("Slick", "fonts/slick.woff");
	//addFont("Emoji", "fonts/AppleEmoji.ttf");
	//addFont("ArialBoldItalic", "fonts/ArialCEBoldItalic.ttf");

	_vao = new gl::VertexAttributeArray();
	_vbo = new gl::VertexBuffer();
	_vao->setBufferBinding(0, _vbo, sizeof(vec2) + sizeof(uint));
	_vao->setAttribute(0, 1, gl::DataType::UnsignedInt, 0, gl::DataType::UnsignedInt);
	_vao->setAttribute(1, 2, gl::DataType::Float, sizeof(uint), gl::DataType::Float);
	_vao->bindAttributeToBufferBinding(0, 0);
	_vao->bindAttributeToBufferBinding(1, 0);
}

void font::close() {
	for (auto fontSets : _fontSets) {
		delete fontSets;
	}
	_vao->release();
	_vbo->release();
	_textShader->release();
	FT_Done_FreeType(_ftLib);
}

std::vector<const std::string*> font::getAllFontSets() {
	std::vector<const std::string*> v{};
	v.reserve(_fontSets.size());
	for (auto& ptr : _fontSets) {
		v.emplace_back(&ptr->fontName);
	}
	return v;
}
const std::vector<std::string> font::getAllFontStyles() {
	//wil return empty string...
	return std::vector(std::begin(_style), std::end(_style));
}
bool font::addFont(const std::string& fontSetName, const std::string& fileLocation, const std::vector<std::string>& style) {
	logger("Loading font file: ", fileLocation, "...\n");
	Style styl = 0;
	for (auto& sty : style) {
		uint i;
		for (i = 0;i < sizeof(Style)*8 && !_style[i].empty(); i++) {
			if (sty==_style[i]) break;
		}
		if (i >= sizeof(Style)*8) {
			logger(sizeof(Style)*8, " styles type limit reached!\n");
		} else {
			_style[i] = sty;
		}
		styl += Style(1)<<i;
	}
	FontSet* fontSet = _getFontSet(fontSetName);
	if (fontSet==nullptr) {
		fontSet = _fontSets.emplace_back(new FontSet{fontSetName});
	}
	if (!fontSet->fontFaces.empty() && fontSet->getFace(styl)->style==styl) {
		logger("FontSet ", fontSetName, " with this style aready exist. Unload that first!\n");
		return false;
	}
	FontFace* face = fontSet->fontFaces.emplace_back(new FontFace{fontSet, styl, gl::target->pixelPerInch});
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
			face->charCodeLookup.emplace(p.first, uint(face->glyphs.size()-1));
		}
		face->glyphs.shrink_to_fit();
		logger("Loading completed. Number of registored charCodes: ", face->charCodeLookup.size(), ", Number of registored glyphs: ", face->glyphs.size(), "\n");
	}
	return true;
}
bool font::linkFallbackFont(const std::string& fontSet, const std::string& fallbackFontSet) {
	FontSet* tFont = nullptr;
	FontSet* bFont = nullptr;
	for (auto& f : _fontSets) {
		assert(f!=nullptr);
		if (f->fontName==fontSet) {
			tFont = f;
			if (tFont!=nullptr && bFont!=nullptr) break;
		}
		if (f->fontName==fallbackFontSet) {
			bFont = f;
			if (tFont!=nullptr && bFont!=nullptr) break;
		}
	}
	if (tFont!=nullptr && bFont!=nullptr) {
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
			if (font->fontName==fontSet) {
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
		for (i = 0; i < sizeof(Style)*8 && !_style[i].empty(); i++) {
			if (sty==_style[i]) break;
		}
		if (i >= sizeof(Style)*8) {
			logger(sizeof(Style)*8, " styles type limit reached! Not adding ", sty, " into style list.\n");
		} else {
			_style[i] = sty;
			_targetStyle += Style(1)<<i;
		}
	}
}
bool font::toggleStyle(const std::string& style, bool setTo) {
	uint i;
	for (i = 0; i < sizeof(Style)*8 && !_style[i].empty(); i++) {
		if (style==_style[i]) {
			bool v = _targetStyle & Style(1)<<i;
			if (setTo != v)
				_targetStyle ^= Style(1)<<i; //flap bit if not equal
			return v;
		}
	}
	if (i >= sizeof(Style)*8) {
		logger(sizeof(Style)*8, " styles type limit reached! Not adding ", style, " into style list.\n");
		return false;
	} else {
		_style[i] = style;
		if (setTo)
			_targetStyle |= Style(1)<<i;
		return false;
	}
}
bool font::setDefaultFontSet(const std::string& fontSet) {
	for (auto& font : _fontSets) {
		if (font->fontName==fontSet) {
			_defaultFont = font;
			return true;
		}
	}
	return false;
}

FontFaceData font::getFontFaceData(FontFace* fontFace, float pointSize) {
	return FontFaceData{
		fontFace,
		float(fontFace->_lineHeightUnit)/float(fontFace->_unitPerEM)*pointSize/72.f*gl::target->pixelPerInch.y/gl::target->viewport.w*2.f
	};
}
GlyphData font::getGlyphData(FontFace* fontFace, GlyphId gId, float pointSize) { //5 (1,1) -> (5,5)
	auto& g = fontFace->glyphs[gId];

	if (fontFace->texturePPI==vec2{0}) 
		fontFace->texturePPI = gl::target->pixelPerInch;
	// >1 = enlarge (not enough resolution),  <1 = minify (enough resolution)
	//NOTE: Divade by zero may cause issue if not using IEEE float!
	vec2 textureRelativeScale = (pointSize*gl::target->pixelPerInch) / (float(g._renderedPointSize)*fontFace->texturePPI);
	float& higherValue = textureRelativeScale.x>textureRelativeScale.y ? textureRelativeScale.x : textureRelativeScale.y;
	if (textureRelativeScale.x > textureRelativeScale.y) {
		if (g._renderedPointSize!=_size.back() && (g._renderedPointSize==0 || textureRelativeScale.x>2)) {
			uint targetSize = uint(ceil(pointSize * (gl::target->pixelPerInch.x/fontFace->texturePPI.x)));
			auto sizeIt = std::lower_bound(_size.begin(), _size.end(), targetSize);
			if (sizeIt==_size.end()) sizeIt--;
			g._renderedPointSize = *sizeIt;
			_requireRender[fontFace].emplace(gId, *sizeIt);
			//recaculate scale
			textureRelativeScale = (pointSize*gl::target->pixelPerInch) / (float(g._renderedPointSize)*fontFace->texturePPI);
		}
	} else {
		if (g._renderedPointSize!=_size.back() && (g._renderedPointSize==0 || textureRelativeScale.y>2)) {
			uint targetSize = uint(ceil(pointSize * (gl::target->pixelPerInch.y/fontFace->texturePPI.y)));
			auto sizeIt = std::lower_bound(_size.begin(), _size.end(), targetSize);
			if (sizeIt==_size.end()) sizeIt--;
			g._renderedPointSize = *sizeIt;
			_requireRender[fontFace].emplace(gId, *sizeIt);
			//recaculate scale
			textureRelativeScale = (pointSize*gl::target->pixelPerInch) / (float(g._renderedPointSize)*fontFace->texturePPI);
		}
	}
	assert(textureRelativeScale.x<=1 && textureRelativeScale.y<=1);
	return GlyphData{
		gId,
		gl::target->normalizeLength(vec2(g._advanceUnit)/float(fontFace->_unitPerEM)*pointSize/72.f*gl::target->pixelPerInch),
		textureRelativeScale
	};
}
void font::renderRequiredGlyph() {
	struct alignas(vec2) glGlyph {
		//uint gId = -1;
		uvec2 origin = {0,0}; //Texture origin (pixel)
		uvec2 size = {0,0}; //Texture size (pixel)
		vec2 emBearing = {0,0}; //bearing (in em size)
		vec2 emSize = {0,0}; //size (relative to the em square)
	};
	struct alignas(uint) glData {
		uint identifier = 43; //New identifier: textRender v 1.1
		uint size;
		glGlyph glyphs[]; //NOTE: using non statandard extension
	};
	for (auto& fontPair : _requireRender) {
		auto& font = fontPair.first;
		auto& glyphs = fontPair.second;
		if (glyphs.empty()) continue;
		if (font->textureShelf == nullptr) {
			font->textureShelf = new mapbox::ShelfPack(_texSize[0], _texSize[0]);
			font->texture = new gl::Texture2D();
			glTextureParameteri(font->texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(font->texture->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(font->texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(font->texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			font->texture->setFormat(GL_R8, _texSize[0], _texSize[0], 1);
		};
		auto& shelf = *font->textureShelf;
		glData* ssboData;
		if (font->ssbo == nullptr) {
			font->ssbo = new gl::ShaderStorageBuffer();
			font->ssbo->setFormatAndData(sizeof(glData)+sizeof(glGlyph)*font->glyphs.size(), GL_MAP_WRITE_BIT);
			ssboData = (glData*)font->ssbo->map(GL_WRITE_ONLY);
			ssboData->identifier = 43;
			ssboData->size = font->glyphs.size();
		} else {
			ssboData = (glData*)font->ssbo->map(GL_WRITE_ONLY);
		}


		for (auto& gPair : glyphs) {
			mapbox::Bin* bin = shelf.getBin(gPair.first);
			if (bin != nullptr) {
				//NOTE: Not sure how mapbox::shelfpack works in unref-ing bins.
				//Does it only ever deallocate memory when shelf is destroyed?
				//if so we have a memory problem here if you run the game too long.
				shelf.unref(*bin);
			}
			_Glyph& _glyph = font->glyphs[gPair.first];
			uint& _pointSize = gPair.second;
			auto& g = ssboData->glyphs[gPair.first];
			//g.gId = gPair.first;
			uvec2 pixelSize = uvec2(ceil(float(_pointSize)*font->texturePPI.x/72.f), ceil(float(_pointSize)* font->texturePPI.y/72.f));
			pixelSize *= 2; //Subsampling rate
			FT_Set_Pixel_Sizes(font->face, pixelSize.x, pixelSize.y);
			if (FT_Load_Glyph(font->face, _glyph._gId, FT_LOAD_DEFAULT)) {
				logger("Warning: Glyph id ", gPair.first, " at ", font->face->family_name, " ", font->face->style_name, " failed to load. Skipping...");
				g.origin = uvec2{0,0};
				g.size = uvec2{0,0};
				g.emBearing = vec2{0,0};
				g.emSize = vec2{0,0};
				continue;
			}if (FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL)) { //change this for color rendering)) {
				logger("Warning: Glyph id ", gPair.first, " at ", font->face->family_name, " ", font->face->style_name, " failed to render. Skipping...");
				g.origin = uvec2{0,0};
				g.size = uvec2{0,0};
				g.emBearing = vec2{0,0};
				g.emSize = vec2{0,0};
				continue;
			}
			g.size = uvec2{font->face->glyph->bitmap.width, font->face->glyph->bitmap.rows};
			g.emBearing = vec2{vec2(_glyph._bearingUnit)/float(font->_unitPerEM)};
			g.emSize = vec2(vec2(_glyph._sizeUnit)/float(font->_unitPerEM));
			if (g.size.x==0 || g.size.y==0) {
				continue; //a glyph that's empty (like space:' ')
			}

			bin = shelf.packOne(gPair.first, g.size.x + GLYP_SPRITE_BORDER_WIDTH, g.size.y + GLYP_SPRITE_BORDER_WIDTH);
			if (bin == nullptr) {
				//do resize
				uint oldSize = shelf.width();
				while (bin == nullptr) {
					if (shelf.width() == MAXTEXTURESIZE) break;
					//get new size
					uint i;
					for (i = 0; i<_texSize.size(); i++) {
						if (_texSize[i]>shelf.width()) break;
					}

					shelf.resize(_texSize[i], _texSize[i]);
					bin = shelf.packOne(gPair.first, g.size.x + GLYP_SPRITE_BORDER_WIDTH, g.size.y + GLYP_SPRITE_BORDER_WIDTH);
				}
				//check if resize successful
				if (bin == nullptr) {
					bool successful = shelf.resize(oldSize, oldSize);
					assert(successful);
					logger("Max texture size reached. Font glyph sprite map creation failed. Skipping glyph sprite...\n");
					g.origin = uvec2{0,0};
					g.size = uvec2{0,0};
					g.emBearing = vec2{0,0};
					g.emSize = vec2{0,0};
					continue;
				}
				//resize texture
				assert(font->texture!=nullptr);
				gl::Texture2D* newTexture = new gl::Texture2D();
				newTexture->setFormat(GL_R8, shelf.width(), shelf.width(), 1);
				newTexture->cloneData(font->texture, uvec2{0}, uvec2{oldSize}, 0);
				font->texture->release();
				font->texture = newTexture;
				glTextureParameteri(font->texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(font->texture->id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(font->texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(font->texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			assert(bin != nullptr);
			g.origin = uvec2(bin->x, bin->y);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //IMPORTANT!! Without this, it WILL cause seg fault!
			font->texture->setData(bin->x, bin->y, g.size.x, g.size.y, 0, GL_RED, GL_UNSIGNED_BYTE, font->face->glyph->bitmap.buffer);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			//logger("Glyph id ", gPair.first, " at ", font->face->family_name, " ", font->face->style_name, " render successfully.");
		}
		font->ssbo->unmap();
	}
	_requireRender.clear();
}
void font::_renderBatch(FontFace* f, std::vector<_RenderData> d, float pointSize) {
	Swapper _{gl::target->vao, _vao};
	Swapper __{gl::target->useBlend, true};
	Swapper ___{gl::target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
	_textShader->use();
	_textShader->setUniform("emSize", gl::target->normalizeLength((pointSize/72.f)*gl::target->pixelPerInch));
	_textShader->setUniform("texColor", vec4(0.0,0.0,0.0,1.0));
	gl::target->bind(f->ssbo);
	gl::target->bind(f->texture, 0);
	_vbo->setFormatAndData(sizeof(_RenderData)*d.size(), 0, d.data());
	gl::target->drawArrays(gl::GeomType::Points, 0, d.size());
	_vbo->release();
	_vbo = new gl::VertexBuffer();
	gl::target->vao->setBufferBinding(0, _vbo, sizeof(_RenderData));
}
std::pair<FontFace*, GlyphId> font::getGlyphFromChar(char32_t c) {
	std::pair<FontFace*, GlyphId> result;
	std::set<FontSet*> searched{};
	if (_targetFont != nullptr) {
		if (_checkFont(c, _targetFont, searched, result))
			return result;
	}
	if (_defaultFont != nullptr) {
		if (_checkFont(c, _defaultFont, searched, result))
			return result;
	}
	return std::make_pair(nullptr, 0);
}