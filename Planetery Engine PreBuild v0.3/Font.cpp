
#include "Logger.h"

#include "Font.h"

#include "GL.h"
#include "ShaderProgram.h"
#include "ThreadEvents.h"

#include <unordered_map>

#include <locale>

/*  DISABLED IMPLENMENTATION WITH rectpack2D lib.
*  - Issue: Not working with c++20
*  - Source: https://github.com/TeamHypersomnia/rectpack2D

#include <finders_interface.h>
*/
#include <shelf-pack.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define GL_SSBO_IDENTIFIER_FONT_GLYPH uint(42)
#define CHARCODE_8_UNKNOWN_CHAR '?'
#define CHARCODE_8_ONKNOWN_OBJECT '?'
#define CHARCODE_16_UNKNOWN_CHAR char16_t(0xFFFD)
#define CHARCODE_16_ONKNOWN_OBJECT char16_t(0xFFFC)
#define CHARCODE_32_UNKNOWN_CHAR char32_t(0x0000FFFD)
#define CHARCODE_32_ONKNOWN_OBJECT char32_t(0x0000FFFC)
#define CHARCODE_NEXTLINE '\n'

#ifndef GL_MAP_BUFFER_MAX_TRY
#define GL_MAP_BUFFER_MAX_TRY 10
#endif


struct Glyph {
	ivec2 advance;
	uvec2 origin; //uvec2(uint(-1)) = This glyph failed to load (invaild glyph)
	uvec2 size;
	ivec2 bearing;
};

struct FontSet {
	gl::Texture2D* texture = nullptr;
	gl::ShaderStorageBuffer* ssbo = nullptr;
    std::string fontName;
	FontSet* fallbackFont;
	float renderedPointSize;
	float line_spacing;
	std::vector<Glyph> glyphs;
	std::unordered_map<char32_t, uint> charLookup;
	std::vector<uint> ftGlyphLookup;
};
namespace fonts {
	FontSet* defaultFont = nullptr;
    std::vector<FontSet*> fontList{};
	FT_Library ftLib{};
	FontSet* activeFont = nullptr;
	ShaderProgram* textShader = nullptr;
	gl::VertexAttributeArray* vao = nullptr;
	gl::VertexBuffer* vbo = nullptr;
}

inline FontSet* _findFont(const std::string& fontName) {
	for (auto& ft : fonts::fontList) {
		if (ft->fontName==fontName) return ft;
	}
	return nullptr;
}


//1 point = 1/72 inch
//1 inch = ppi*1 pixel
//72pt = 1i = ppi*px
//1pt = 1/72i = ppi/72px
//72/ppi pt = 1/ppi i = 1 px
vec2 FontManager::pixelToPoint(vec2 v) {
	vec2 ppi = events::ThreadEvents::getPixelPerInch();
	return {v.x*72.f/ppi.x, v.y*72.f/ppi.y};
}
vec2 FontManager::pointToPixel(vec2 v) {
	vec2 ppi = events::ThreadEvents::getPixelPerInch();
	return {v.x*ppi.x/72.f, v.y*ppi.y/72.f};
}

void FontManager::init() {

	FT_Library ftlib;
	if (FT_Init_FreeType(&ftlib)) {
		logger("ERROR::FREETYPE: Could not init FreeType Library\n");
		throw "FreeType font library init failed";
	}
	fonts::ftLib = ftlib;
	fonts::textShader = new ShaderProgram("shader/textRender.vert", "shader/textRender.geom", "shader/textRender.frag");
	if (!loadFont("LastResort", "fonts/LastResortHE-Regular.ttf")) throw "LastResort font loading failed";

	if (!loadFont("Arial", "fonts/arialuni.ttf", "LastResort")) throw "Default font loading failed";
	fonts::defaultFont = _findFont("Arial");
	assert(fonts::defaultFont!=nullptr);
	loadFont("OrangeJuice", "fonts/orange juice.ttf");
	loadFont("RemachineScript", "fonts/RemachineScript.ttf");
	loadFont("AeroviasBrasilNF", "fonts/AeroviasBrasilNF.ttf");
	loadFont("Sketsaramadhan", "fonts/Sketsaramadhan-nRLAO.otf");
	loadFont("Slick", "fonts/slick.woff");
	loadFont("Emoji", "fonts/AppleEmoji.ttf");
	loadFont("ArialBoldItalic", "fonts/ArialCEBoldItalic.ttf");

	useFont("Emoji");
	auto& vao = fonts::vao;
	auto& vbo = fonts::vbo;
	vao = new gl::VertexAttributeArray();
	vbo = new gl::VertexBuffer();
	vao->setBufferBinding(0, {vbo,sizeof(vec2) + sizeof(uint)});
	vao->setAttribute(0, {2,GL_FLOAT,0}, GL_FLOAT);
	vao->setAttribute(1, {1,GL_UNSIGNED_INT,sizeof(vec2)}, GL_INT);
	vao->bindAttributeToBufferBinding(0, 0);
	vao->bindAttributeToBufferBinding(1, 0);

}

bool FontManager::fontLoaded(const std::string& fontName) {
	return (_findFont(fontName)!=nullptr);
}

//TODO: Too slow right now. need optimisation!!
bool FontManager::loadFont(const std::string& fontName, const std::string& path, const std::string& fallbackFont, float pointSize) {

	//safety check
	if (fontName.empty()) {
		logger("Warning! Font name can not be empty!\n");
		return false;
	}
	if (path.empty()) {
		logger("Warning! Font file path can not be empty!\n");
		return false;
	}
	if (fontLoaded(fontName)) {
		logger("Warning! Font ", fontName, "already loaded.\n");
		return false;
	}

	//fallbackFont
	FontSet* fbPtr;
	if (!fallbackFont.empty()) {
		fbPtr = _findFont(fallbackFont);
		if (fbPtr == nullptr) {
			logger.newLayer();
			logger << "Warning! Requested fallback font " << fallbackFont << " for font " << fontName << " is not loaded! ";
			if (fonts::defaultFont==nullptr) {
				fbPtr = nullptr;
				logger << "This font will have no fallback font.\n";
			} else {
				fbPtr = fonts::defaultFont;
				logger << "This font will use default font " << fbPtr->fontName << " as its fallback font.\n";
			}
			logger.closeLayer();
		}
	} else {
		if (fonts::defaultFont==nullptr) {
			fbPtr = nullptr;
		} else {
			fbPtr = fonts::defaultFont;
		};
	}
	FontSet& font = *new FontSet{0, 0, fontName, fbPtr, pointSize,0};
	//-------------------------LOAD FONT FILE TO FREETYPE LIB-----------------------------
	FT_Face face;
	FT_Error err = FT_New_Face(fonts::ftLib, path.c_str(), 0, &face);
	if (err) {
		const char* text = FT_Error_String(err);
		if (text==NULL) text = "Enable Debug mode for error string.";
		logger("FontManager: Exception in font loading: Font set ", fontName, " loading failed. It is possible that ", path, " file is missing.\n",
			"Detailed error from FreeFont library:\nError Code:", err, "\n", text);
		delete& font;
		return false;
	}
	logger.newLayer();
	logger << "Face loaded:\n";
	logger(face->family_name, ": ", face->style_name, "\n");
	logger("Faces avalible: ", face->num_faces, "\n");
	logger("Face index: ", std::hex, face->face_index, std::dec, "\n");
	logger("Face flag: ", std::hex, face->face_flags, std::dec, "\n");
	logger("Scalable: ", bool(face->face_flags | FT_FACE_FLAG_SCALABLE), "\n");
	logger("Can be horizontal: ", bool(face->face_flags | FT_FACE_FLAG_HORIZONTAL), "\n");
	logger("Can be vertical: ", bool(face->face_flags | FT_FACE_FLAG_VERTICAL), "\n");
	logger("Has color: ", bool(face->face_flags | FT_FACE_FLAG_COLOR), "\n");
	logger.closeLayer();
	//1 point = 1/72 inch
	//1 inch = ppi*1 pixel
	//72pt = 1i = ppi*px
	//1pt = 1/72i = ppi/72px
	uvec2 fontPixelSize = uvec2(pointToPixel(vec2(pointSize)));
	logger("FontPixelSize: ", fontPixelSize.x, "*", fontPixelSize.y, " (", pointSize, "pt)\n");
	FT_Set_Pixel_Sizes(face, fontPixelSize.x, fontPixelSize.y);
	font.line_spacing = face->size->metrics.height;
	//-----------------------------------READ CHAR MAP------------------------------------

	logger.newMessage();
	logger << "Loading all chars...\n";

	std::vector<std::pair<char32_t, uint>> charToGId{};
	charToGId.reserve(65536);

	uint gIndex;
	char32_t charCode = FT_Get_First_Char(face, &gIndex);
	while (gIndex!=0) {
		charToGId.emplace_back(std::make_pair(charCode, gIndex));
		charCode = FT_Get_Next_Char(face, charCode, &gIndex);
	}
	if (charToGId.empty()) {
		logger("No Glyphs detected from file!! returning!\n");
		logger.closeMessage();
		FT_Done_Face(face);
		delete& font;
		return false;
	}
	std::sort(charToGId.begin(), charToGId.end(), [](std::pair<char32_t, uint>a, std::pair<char32_t, uint>b) {return a.second<b.second; });
	font.ftGlyphLookup.reserve(charToGId.size());
	for (auto& p : charToGId) {
		if (font.ftGlyphLookup.empty() || font.ftGlyphLookup.back()!=p.second)
			font.ftGlyphLookup.push_back(p.second);
	}
	font.ftGlyphLookup.shrink_to_fit();
	font.charLookup.reserve(charToGId.size());
	uint index = 0;
	for (auto& p : charToGId) {
		if (font.ftGlyphLookup[index]!=p.second) index++;
		assert(font.ftGlyphLookup[index]==p.second);
		font.charLookup.insert(std::make_pair(p.first, index));
	}

	logger("Number of registored charCodes: ", font.charLookup.size(), ", Number of registored glyphs: ", font.ftGlyphLookup.size(), "\n");
	logger.closeMessage();

	//----------------------------RENDER GLYPH BITMAPS--------------------------------------
	const uint bitmapBorder = 1;
	logger.newMessage();
	logger("Now rendering glyphs...\n");
	uint gIdSize = font.ftGlyphLookup.size();
	std::pair<std::vector<mapbox::Bin>, std::vector<FT_BitmapGlyph>> rects{};
	font.glyphs.reserve(gIdSize);
	rects.first.reserve(gIdSize);
	rects.second.reserve(gIdSize);
	for (uint gId = 0; gId<gIdSize; gId++) {
		uint ft_gId = font.ftGlyphLookup[gId];

		if (FT_Load_Glyph(face, ft_gId, FT_LOAD_RENDER)) {
			logger(gId, "(", ft_gId, ")", " rendering failed.\n");
			font.glyphs.emplace_back(ivec2(0), uvec2(uint(-1)), uvec2(0), ivec2(0));
			continue;
		}
		auto& bitmap = face->glyph->bitmap;
		if (bitmap.width!=0 && bitmap.rows!=0) {
			rects.first.emplace_back(gId, bitmap.width+bitmapBorder, bitmap.rows+bitmapBorder);
			auto& ptr = rects.second.emplace_back(nullptr);
			FT_Get_Glyph(face->glyph, (FT_Glyph*)(&ptr));
			assert(FT_Glyph(ptr)->format == FT_GLYPH_FORMAT_BITMAP);
		}
		font.glyphs.emplace_back(ivec2(face->glyph->advance.x), uvec2(uint(-1)),
			uvec2(bitmap.width, bitmap.rows), ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top));
	}
	assert(font.glyphs.size()==gIdSize);
	assert(rects.first.size()==rects.second.size());
	logger("Glyph rendering completed.\n");
	logger.closeMessage();

	//----------------------------COMPLETE GL FONT DATA---------------------------------------
	int maxSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	logger.newMessage();
	logger("Now making glyph texture maps...\n");
	mapbox::ShelfPack bitmap(maxSize, maxSize);
	mapbox::ShelfPack::PackOptions packOp;
	packOp.inPlace = true;
	logger("Using mapbox/shelf-pack-cpp to pack bitmaps...\n");
	bitmap.pack(rects.first, packOp);
	bitmap.shrink();
	logger("packing completed! packed size: ", bitmap.width(), "-", bitmap.height(), "\n");
	if (bitmap.width()>maxSize||bitmap.height()>maxSize) {

		logger("FontManager: Exception in font packing: Font set ", fontName, " texture map size too large. It is possible that font size is too large. (Maxsize:", maxSize, ")\n");
		logger.closeMessage();
		for (auto& g : rects.second) {
			FT_Done_Glyph(&g->root);
		}
		FT_Done_Face(face);
		delete& font;
		return false;
	}

	logger("Pushing glyph texture map and glyph data to GPU...\n");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //IMPORTANT!! Without this, it WILL cause seg fault!
	font.texture = new gl::Texture2D();
	glTextureParameteri(font.texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(font.texture->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(font.texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(font.texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	font.texture->setFormat(GL_R8, bitmap.width(), bitmap.height(), 1);
	assert(rects.first.size()==rects.second.size());
	for (uint i = 0; i<rects.first.size(); i++) {
		mapbox::Bin& box = rects.first[i];
		FT_BitmapGlyph& bg = rects.second[i];
		Glyph& g = font.glyphs[box.id];
		if (box.x<0 || box.y<0) {
			logger("Failed to pack font bitmap of glyph id: ", box.id, "! set font rendering size to a lower value!\n");
			logger.closeMessage();
			for (auto& g : rects.second) {
				FT_Done_Glyph(&g->root);
			}
			FT_Done_Face(face);
			font.texture->release();
			delete& font;
			return false;
		}
		g.origin = uvec2(box.x, box.y);
		font.texture->setData(box.x, box.y, bg->bitmap.width, bg->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bg->bitmap.buffer);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	/*struct glData {
		uint identifier;
		uint size;
		glGlyph glyphs[];
	};*/
	struct glGlyph {
		uvec2 origin; //uvec2(uint(-1)) = This glyph failed to load (invaild glyph)
		uvec2 size;
		ivec2 bearing;
	};
	
	uint tries = 0;
	size_t bufferSize = sizeof(uint)*2 + sizeof(glGlyph)*font.glyphs.size();
	font.ssbo = new gl::ShaderStorageBuffer();
	font.ssbo->setFormatAndData(bufferSize, GL_MAP_WRITE_BIT);
	do {
		if (tries++>=GL_MAP_BUFFER_MAX_TRY) throw "GL_BUFFER_MAX_TRY Reached!";
		char* buffer = (char*)glMapNamedBuffer(font.ssbo->id, GL_WRITE_ONLY);
		if (buffer==nullptr) throw "glMapBuffer failed! Possible OUT_OF_MEMORY!";
		*(uint*)(buffer) = GL_SSBO_IDENTIFIER_FONT_GLYPH;
		*((uint*)(buffer)+1) = font.glyphs.size();
		for (uint i = 0; i<font.glyphs.size(); i++) {
			auto& gly = font.glyphs[i];
			*((glGlyph*)(buffer+sizeof(uint)*2)+i) = {gly.origin,gly.size,gly.bearing};
		}
	} while (!glUnmapNamedBuffer(font.ssbo->id));

	//-----------------------------CLEAN UP----------------------------------------------
	fonts::fontList.push_back(&font);
	logger("Font init complete. ", byte(bufferSize), " of vRam used for glyphs meta data, ", byte(ulint(bitmap.width())*ulint(bitmap.height())), " of vRam used for glyphs texture.\n");
	for (auto& g : rects.second) {
		FT_Done_Glyph(&g->root);
	}
	FT_Done_Face(face);
	logger.closeMessage();
	return true;
}


bool FontManager::useFont(const std::string& fontName) {
	auto ptr = _findFont(fontName);
	if (ptr==nullptr) return false;
	fonts::activeFont = ptr;
	return true;
}


template <typename C, typename Vect>
inline static void _findGId(const C& c, FontSet*& font, uint& gId, Vect& lookupCache, C backupCode) {
	//Use glyph of charcode c
	auto it = lookupCache.find(c);
	if (it != lookupCache.end()) {
		font = it->second.first;
		gId = it->second.second;
	} else {
		//find glyph of charcode c
		font = fonts::activeFont;
		while (font!=nullptr) {
			auto hashIt = font->charLookup.find(c);
			if (hashIt != font->charLookup.end()) {
				gId = hashIt->second;
				break;
			}
			font = font->fallbackFont;
		}
		if (font==nullptr) {
			//failed. Now use glyph of CHARCODE_UNKNOWN_CHAR
			auto unknownCharIt = lookupCache.find(backupCode);
			if (unknownCharIt != lookupCache.end()) {
				font = unknownCharIt->second.first;
				gId = unknownCharIt->second.second;
			} else {
				//find glyph of CHARCODE_UNKNOWN_CHAR
				font = fonts::activeFont;
				if (font==nullptr) {
					throw "No active font set!";
				}
				while (font!=nullptr) {
					auto ucHashIt = font->charLookup.find(backupCode);
					if (ucHashIt != font->charLookup.end()) {
						gId = ucHashIt->second;
						break;
					}
					font = font->fallbackFont;
				}
				if (font==nullptr) {
					logger("WARNING in Font Rendering: Failed to find glyph for backupCharCode ", uint(backupCode), " from font ", fonts::activeFont->fontName, " or its fallback font(s).\n");
				}
				lookupCache.insert(std::make_pair(backupCode, std::make_pair(font, gId)));
			}
		}
		lookupCache.insert(std::make_pair(c, std::make_pair(font, gId)));
	}
}

inline static void _sendBatch(std::vector<std::pair<vec2, uint>> &gBuffer,const FontSet* const useFont, vec2 uToNormalized) {
	gl::target->bind(useFont->ssbo);
	gl::target->bind(useFont->texture, 0);
	fonts::textShader->setVec2("scale", uToNormalized/useFont->renderedPointSize);
	fonts::vbo->setFormatAndData((sizeof(vec2)+sizeof(uint))*gBuffer.size(), 0, gBuffer.data());
	gl::target->drawArrays(GL_POINTS, 0, gBuffer.size());
	gBuffer.clear();
	fonts::vbo->release();
	fonts::vbo = new gl::VertexBuffer();
	fonts::vao->setBufferBinding(0, {fonts::vbo,sizeof(vec2) + sizeof(uint)});
};

template<typename C>
concept C8 = sizeof(C) == 1;
template<typename C>
concept C16 = sizeof(C) == 2;
template<typename C>
concept C32 = sizeof(C) == 4;
template<typename C> requires C8<C>
inline static C  _getBackup() { return CHARCODE_8_UNKNOWN_CHAR; }
template<typename C> requires C16<C>
inline static C  _getBackup() { return CHARCODE_16_UNKNOWN_CHAR; }
template<typename C> requires C32<C>
inline static C  _getBackup() { return CHARCODE_32_UNKNOWN_CHAR; }

template <typename C>
static void _renderString(const std::basic_string<C>& str, vec2 pos, float pointSize, vec4 color) {
	Swapper _{gl::target->useBlend, true};
	Swapper __{gl::target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
	Swapper ___{gl::target->vao, fonts::vao};
	assert(fonts::textShader!=nullptr);
	uvec2 viewSize{gl::target->viewport.z, gl::target->viewport.w};
	vec2 uToPx{FontManager::pointToPixel(vec2(pointSize))};
	vec2 uToNormalized{(uToPx/vec2(viewSize))*2.f};
	if (fonts::activeFont==nullptr) {
		logger("Warning!!! No font is active! Using fallback font!\n");
		if (fonts::fontList.empty()) {
			logger("ERROR! No font is loaded!\n");
			throw;
		}
		fonts::activeFont = fonts::fontList[0];
	}
	fonts::textShader->enable();
	fonts::textShader->setVec4("texColor", color);

	std::unordered_map<C, std::pair<FontSet*, uint>> lookupCache;
	FontSet* useFont = nullptr;
	std::vector<std::pair<vec2, uint>> gBuffer;
	for (auto c : str) {
		FontSet* font;
		uint gId;
		//find the related glyph id and font set or use the unknown_char charcode, use lookupCache to save the result
		_findGId(c, font, gId, lookupCache, _getBackup<C>());
		if (font==nullptr) continue; //skip this char as no font can render it
		if (font != useFont && !gBuffer.empty()) _sendBatch(gBuffer, useFont, uToNormalized);
		useFont = font;
		gBuffer.emplace_back(std::make_pair(pos, gId));
		pos.x += float(font->glyphs[gId].advance.x) / 64.f * (uToNormalized/font->renderedPointSize).x;
	}
	_sendBatch(gBuffer, useFont, uToNormalized);
}

template <typename C>
static void _renderStringInBox(const std::basic_string<C>& str, vec4 box, float pointSize, vec4 color) {
	Swapper _{gl::target->useBlend, true};
	Swapper __{gl::target->blendFunc, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}};
	Swapper ___{gl::target->vao, fonts::vao};
	assert(fonts::textShader!=nullptr);
	uvec2 viewSize{gl::target->viewport.z, gl::target->viewport.w};
	vec2 uToPx{FontManager::pointToPixel(vec2(pointSize))};
	vec2 uToNormalized{(uToPx/vec2(viewSize))*2.f};
	if (fonts::activeFont==nullptr) {
		logger("Warning!!! No font is active! Using fallback font!\n");
		if (fonts::fontList.empty()) {
			logger("ERROR! No font is loaded!\n");
			throw;
		}
		fonts::activeFont = fonts::fontList[0];
	}
	fonts::textShader->enable();
	fonts::textShader->setVec4("texColor", color);

	std::unordered_map<C, std::pair<FontSet*, uint>> lookupCache;
	FontSet* useFont = nullptr;
	float lineStartX = box.x;
	std::vector<std::pair<vec2, uint>> gBuffer;
	float maxNextline = 0.f;
	for (auto c : str) {
		if (c == CHARCODE_NEXTLINE) {
			box.x = lineStartX;
			box.y -= maxNextline;
			continue;
		}
		FontSet* font;
		uint gId;
		//find the related glyph id and font set or use the unknown_char charcode, use lookupCache to save the result
		_findGId(c, font, gId, lookupCache, _getBackup<C>());
		if (font==nullptr) continue; //skip this char as no font can render it
		if (font != useFont && !gBuffer.empty()) {
			_sendBatch(gBuffer, useFont, uToNormalized);
			float newLineSpacing = font->line_spacing/64.f * (uToNormalized/font->renderedPointSize).y;
			if (maxNextline<newLineSpacing) maxNextline = newLineSpacing;
		}
		useFont = font;
		gBuffer.emplace_back(std::make_pair(vec2(box.x,box.y), gId));
		box.x += float(font->glyphs[gId].advance.x) / 64.f * (uToNormalized/font->renderedPointSize).x;
		if (box.x >= lineStartX+box.z) {
			box.x = lineStartX;
			box.y -= maxNextline;
		}
	}
	_sendBatch(gBuffer, useFont, uToNormalized);
}


void FontManager::renderString(const std::string& string, vec2 location, float pointSize, vec4 texColor) {
	_renderString(string, location, pointSize, texColor);
}
void FontManager::renderString(const std::wstring& string, vec2 location, float pointSize, vec4 texColor) {
	_renderString(string, location, pointSize, texColor);
}
void FontManager::renderString(const std::u8string& string, vec2 location, float pointSize, vec4 texColor) {
	_renderString(string, location, pointSize, texColor);
}
void FontManager::renderString(const std::u16string& string, vec2 location, float pointSize, vec4 texColor) {
	_renderString(string, location, pointSize, texColor);
}
void FontManager::renderString(const std::u32string& string, vec2 location, float pointSize, vec4 texColor) {
	_renderString(string, location, pointSize, texColor);
}


void FontManager::renderStringInBox(const std::string& string, vec4 box, float pointSize, vec4 texColor) {
	_renderStringInBox(string, box, pointSize, texColor);
}
void FontManager::renderStringInBox(const std::wstring& string, vec4 box, float pointSize, vec4 texColor) {
	_renderStringInBox(string, box, pointSize, texColor);
}
void FontManager::renderStringInBox(const std::u8string& string, vec4 box, float pointSize, vec4 texColor) {
	_renderStringInBox(string, box, pointSize, texColor);
}
void FontManager::renderStringInBox(const std::u16string& string, vec4 box, float pointSize, vec4 texColor) {
	_renderStringInBox(string, box, pointSize, texColor);
}
void FontManager::renderStringInBox(const std::u32string& string, vec4 box, float pointSize, vec4 texColor) {
	_renderStringInBox(string, box, pointSize, texColor);
}