#include <stdexcept>

#include "SceneLight.h"
#include "resources/light/LightReaderWriter.h"

namespace urchin
{

	SceneLight::SceneLight() :
			lightManager(nullptr),
			light(nullptr)
	{

	}

	SceneLight::~SceneLight()
	{
		lightManager->removeLight(light);
	}

	void SceneLight::setLightManager(LightManager *lightManager)
	{
		if(this->lightManager!=nullptr)
		{
			throw std::invalid_argument("Cannot add the scene light on two different light managers.");
		}
		if(lightManager==nullptr)
		{
			throw std::invalid_argument("Cannot specify a null light manager for a scene light.");
		}

		this->lightManager = lightManager;

		lightManager->addLight(light);
	}

	void SceneLight::loadFrom(std::shared_ptr<XmlChunk> chunk, const XmlParser &xmlParser)
	{
		this->name = chunk->getAttributeValue(NAME_ATTR);

		setLight(LightReaderWriter().loadFrom(chunk, xmlParser));
	}

	void SceneLight::writeOn(std::shared_ptr<XmlChunk> chunk, XmlWriter &xmlWriter) const
	{
		chunk->setAttribute(XmlAttribute(NAME_ATTR, this->name));

		LightReaderWriter().writeOn(chunk, light, xmlWriter);
	}

	std::string SceneLight::getName() const
	{
		return name;
	}

	void SceneLight::setName(const std::string &name)
	{
		this->name = name;
	}

	Light *SceneLight::getLight() const
	{
		return light;
	}

	void SceneLight::setLight(Light *light)
	{
		if(light==nullptr)
		{
			throw std::invalid_argument("Cannot set a null light on scene light.");
		}

		if(lightManager!=nullptr)
		{
			lightManager->removeLight(this->light);
			lightManager->addLight(light);
		}else
		{
			delete this->light;
		}

		this->light = light;
	}

}
