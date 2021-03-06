#include <GL/glew.h>
#include "UrchinCommon.h"

#include "scene/GUI/widget/button/Button.h"
#include "scene/GUI/GUISkinService.h"
#include "utils/display/quad/QuadDisplayerBuilder.h"

namespace urchin
{

	Button::Button(Position position, Size size, const std::string &nameSkin, const std::string &buttonText)
		: Widget(position, size),
		  nameSkin(nameSkin),
		  text(nullptr),
		  buttonText(buttonText)
	{
		createOrUpdateWidget();
	}

	void Button::createOrUpdateWidget()
	{
		//skin informations
		std::shared_ptr<XmlChunk> buttonChunk = GUISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "button", XmlAttribute("nameSkin", nameSkin));

		std::shared_ptr<XmlChunk> skinDefaultChunk = GUISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "skin", XmlAttribute("type", "default"), buttonChunk);
		texInfoDefault = GUISkinService::instance()->createTexWidget(getWidth(), getHeight(), skinDefaultChunk);

		std::shared_ptr<XmlChunk> skinFocusChunk = GUISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "skin", XmlAttribute("type", "focus"), buttonChunk);
		texInfoOnFocus = GUISkinService::instance()->createTexWidget(getWidth(), getHeight(), skinFocusChunk);

		std::shared_ptr<XmlChunk> skinClickChunk = GUISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "skin", XmlAttribute("type", "click"), buttonChunk);
		texInfoOnClick = GUISkinService::instance()->createTexWidget(getWidth(), getHeight(), skinClickChunk);

		if(!buttonText.empty())
		{
			std::shared_ptr<XmlChunk> textChunk = GUISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "textSkin", XmlAttribute(), buttonChunk);
			removeChild(text);
			text = new Text(Position(0, 0, Position::PIXEL), textChunk->getStringValue());
			text->setText(buttonText);
			text->setPosition(Position((int)(getWidth() - text->getWidth())/2, (int)(getHeight() - text->getHeight())/2, Position::PIXEL));
			addChild(text);
		}

		//visual
		quadDisplayer = std::make_unique<QuadDisplayerBuilder>()
				->vertexData(GL_UNSIGNED_INT, new unsigned int[8]{0, 0, getWidth(), 0, getWidth(), getHeight(), 0, getHeight()})
				->textureData(GL_FLOAT, new float[8]{0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0})
				->build();

		textureID = texInfoDefault->getTextureID();
	}

	unsigned int Button::getTextureId()
	{
		if(getWidgetState()==FOCUS)
		{
			return texInfoOnFocus->getTextureID();
		}else if(getWidgetState()==CLICKING)
		{
			textureID = texInfoOnClick->getTextureID();
		}

		return texInfoDefault->getTextureID();
	}

	bool Button::onKeyDownEvent(unsigned int key)
	{
		textureID = getTextureId();
		return true;
	}

	bool Button::onKeyUpEvent(unsigned int key)
	{
		textureID = getTextureId();
		return true;
	}

	bool Button::onMouseMoveEvent(int mouseX, int mouseY)
	{
		textureID = getTextureId();
		return true;
	}

	void Button::display(int translateDistanceLoc, float invFrameRate)
	{
		glBindTexture(GL_TEXTURE_2D, textureID);

		quadDisplayer->display();

		Widget::display(translateDistanceLoc, invFrameRate);
	}
	
}
