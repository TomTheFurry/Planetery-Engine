#pragma once
#include <list>
#include <string>
#include <glm/glm.hpp>

#include "Global.h"

#include "ShaderProgram.h"
#include "ModelBase.h"

class RenderGui : public ShaderProgram
{
public:
    RenderGui();
    void updateValue();
    void render(ModelBase* data);
    void postRender();

};

class GuiBase : public ModelBase
{
	GuiBase();
	virtual ~GuiBase();
	virtual uint gVao();
	virtual uint gVeo();
	virtual uint gLength();
	virtual mat4 gMat();
	virtual GLenum gType();
	virtual void renderUpdate(float delta);
	std::vector<GuiBase*> childs;
	vec4 location;
};

class GuiContainer : public GuiBase
{
	GuiContainer();
	virtual ~GuiContainer();
	virtual uint gVao();
	virtual uint gVeo();
	virtual uint gLength();
	virtual mat4 gMat();
	virtual GLenum gType();
	virtual void renderUpdate(float delta);
	vec4 bgColor;
	vec4 fgColor;
	vec4 outlineColor;
	float edgeCurve;
};

class GuiTableContainer : public GuiContainer
{
	GuiTableContainer();
	virtual ~GuiTableContainer();
	virtual uint gVao();
	virtual uint gVeo();
	virtual uint gLength();
	virtual mat4 gMat();
	virtual GLenum gType();
	virtual void renderUpdate(float delta);
	void addGui(vec2 tableLocation, GuiBase* gui);
	vec2 tableSize;
};

class GuiTextBox : public GuiContainer
{
	GuiTextBox();
	virtual ~GuiTextBox();
	virtual uint gVao();
	virtual uint gVeo();
	virtual uint gLength();
	virtual mat4 gMat();
	virtual GLenum gType();
	virtual void renderUpdate(float delta);
	float textSize;
	std::string font; //TODO: font type here
	std::string text;
	
};

class GuiButton : public GuiContainer
{
	GuiButton();
	virtual ~GuiButton();
	virtual uint gVao();
	virtual uint gVeo();
	virtual uint gLength();
	virtual mat4 gMat();
	virtual GLenum gType();
	virtual void renderUpdate(float delta);
	vec4 bgColorOnClick;
	vec4 fgColorOnClick;
	vec4 outlineColorOnClink;
	void (*onClick)(GuiBase*) = 0;
	void (*onHover)(GuiBase*) = 0;
};