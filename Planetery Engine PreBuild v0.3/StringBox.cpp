#include "StringBox.h"
#include "Logger.h"
#include "GL.h"
#include "Font.h"

#include <glad/glad.h>

static gl::RenderTarget* _targetText = nullptr;

StringBox::StringBox() : _ss() {
	pos = vec2(0, 0);
	size = vec2(0, 0);
	_ppi = vec2(0, 0);
	_textureSize = vec2(0, 0);
	_pointSize = 12.f;
	_change = true;
	_tex = nullptr;
	_fbo = nullptr;
}

void StringBox::render() {
	if (_change) {
		logger("ReRendering...\n");
		//render text here
		_ppi = gl::target->pixelPerInch;
		_textureSize = vec2(gl::target->viewport.z, gl::target->viewport.w)*size;
		if (_textureSize.x<=0 || _textureSize.y<=0) {
			_change = false;
			return;
		}
		
		if (_targetText==nullptr) {
			_targetText = new gl::RenderTarget();
		}
		if (_fbo == nullptr)
			_fbo = new gl::FrameBuffer();
		_targetText->bind(_fbo);
		_targetText->viewport = vec4(vec2(0.f), _textureSize);
		_targetText->pixelPerInch = _ppi;
		if (_tex != nullptr)
			_tex->release();
		_tex = new gl::Texture2D();
		_tex->setFormat(GL_R8, _textureSize.x, _textureSize.y, 1);
		_fbo->attach(_tex, GL_COLOR_ATTACHMENT0, 0);
		Swapper _{gl::target, _targetText};

		gl::target->activateFrameBuffer();
		glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		std::string _str{str()};
		vec2 drawHead = vec2(-1,1);
		float maxLineHeight = -1;
		font::drawChar(_str.begin(), _str.end(), _pointSize,
			[](const font::fullStringFunction::FontData& data,
				const font::fullStringFunction::Chars& chars,
				const font::fullStringFunction::IndexLookup& indexLookup,
				font::fullStringFunction::Output& output) {
				if (chars.empty()) return;
				output.reserve(data.size());
				float maxAccend = 0;
				float lineHeight = 0;
				for (uint i = 0; i<data.size(); i++) {
					output.emplace_back();
					if (data.at(i).first.maxAccend>maxAccend) maxAccend = data.at(i).first.maxAccend;
					if (data.at(i).first.lineHeight>lineHeight) lineHeight = data.at(i).first.lineHeight;
				}
				auto charIt = chars.begin();
				vec2 drawHead = vec2(-1, 1);
				drawHead.y -= maxAccend;
				while (charIt!=chars.end()) {
					constexpr char32_t CONTROL_PICTURES = u'\u2400';
					while (charIt!=chars.end()) {

						char32_t c = std::get<0>(*charIt);
						uint fontI = std::get<1>(*charIt);
						uint glyphI = std::get<2>(*charIt);
						font::GlyphId gId = indexLookup.at(fontI).second.at(glyphI);
						if (drawHead.x+data.at(fontI).second.at(glyphI).right > 1) goto NextLine;
						bool nextLine = false;
						bool tab = false;
						bool noRender = false;
						switch (c) {
						case u'\0':
							noRender = true;
							break;

						case u'\t':
							noRender = true;
							[[fallthrough]];
						case u'\t'+CONTROL_PICTURES:
							tab = true;
							break;

						case u'\n':
							noRender = true;
							[[fallthrough]];
						case u'\n'+CONTROL_PICTURES:
							nextLine = true;
							break;

						case u'\v':
							noRender = true;
							[[fallthrough]];
						case u'\v'+CONTROL_PICTURES:
							nextLine = true;
							break;

						case u'\f':
							noRender = true;
							[[fallthrough]];
						case u'\f'+CONTROL_PICTURES:
							nextLine = true;
							break;

						case u'\r':
							noRender = true;
							[[fallthrough]];
						case u'\r'+CONTROL_PICTURES:
							if (charIt+1==chars.end() ||
								(std::get<0>(*(charIt+1))!=u'\n' && std::get<0>(*(charIt+1))!=u'\n'+CONTROL_PICTURES)) {
								nextLine = true;
							}
							break;

						case u'\u0085':
						case u'\u2828':
						case u'\u2029':
							noRender = true;
							nextLine = true;
							break;

						case u'\u009f':
							noRender = true;
							[[fallthrough]];
						case u'\u009f'+CONTROL_PICTURES:
							//logger("Font Engine: Warning: This DrawMode does not support font engine control commend!\n");
							break;
							
						default:
							break;
						}

						if (fontI!=-1 && !noRender) {
							if (drawHead.x-data.at(fontI).second.at(glyphI).left<-1.f)
								drawHead.x += data.at(fontI).second.at(glyphI).left;
							assert(drawHead.x>=-1 && drawHead.x<=1);
							assert(drawHead.y+maxAccend>=-1 && drawHead.y<=1);
							//gl::drawRectangle(nullptr, drawHead, vec2(0.01));
							output.at(fontI).emplace_back(indexLookup.at(fontI).second.at(glyphI), drawHead);
							drawHead.x += data.at(fontI).second.at(glyphI).advance.x;
						}
						charIt++;
						if (nextLine) break;
					}
NextLine:
					drawHead.y -= lineHeight;
					drawHead.x = -1.f;
					if (drawHead.y+maxAccend<-1) break; //out of space. drawing at below screen
				}
			}
		);
		_change = false;
	}
	//use render texture here
	gl::drawRectangleR8Color(_tex, pos, size*2.f, vec4(0.f,0.2f,0.f,1.f));
}

std::string StringBox::str() const {
	return _ss.str();
}
void StringBox::str(const std::string& string) {
	_change = true;
	_ss.str(string);
}
void StringBox::str(std::string&& string) {
	_change = true;
	_ss.str(std::move(string));
}

void StringBox::clear() {
	_change = true;
	_ss.str(std::string());
	_ss.clear();
}

void StringBox::setSize(vec2 s) {
	_change = true;
	size = s;
}
vec2 StringBox::getSize() const {
	return size;
}
void StringBox::setTextSize(float p) {
	_change = true;
	_pointSize = p;
}

void StringBox::notifyPPIChanged() {
	_change = true;
}
