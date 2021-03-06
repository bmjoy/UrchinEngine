#include <stdexcept>
#include <cmath>
#include <memory>
#include <limits>

#include "OmnidirectionalLight.h"

namespace urchin
{

	OmnidirectionalLight::OmnidirectionalLight(const Point3<float> &position) :
			Light(),
			position(position),
			attenuationNoEffect(ConfigService::instance()->getFloatValue("light.attenuationNoEffect")),
			exponentialAttenuation(0.01),
			sphereScope(nullptr),
			bboxScope(nullptr)
	{
		computeScope();
	}

	OmnidirectionalLight::~OmnidirectionalLight()
	{
		delete sphereScope;
		delete bboxScope;
	}

	void OmnidirectionalLight::setPosition(const Point3<float> &position)
	{
		this->position = position;

		computeScope();

		notifyObservers(this, Light::LIGHT_MOVE);
	}

	const Point3<float> &OmnidirectionalLight::getPosition() const
	{
		return position;
	}

	std::vector<Vector3<float>> OmnidirectionalLight::getDirections() const
	{
		std::vector<Vector3<float>> directions;

		directions.emplace_back(Vector3<float>(1.0, 0.0, 0.0)); //Left
		directions.emplace_back(Vector3<float>(-1.0, 0.0, 0.0)); //Right
		directions.emplace_back(Vector3<float>(0.0, 1.0, 0.0)); //Top
		directions.emplace_back(Vector3<float>(0.0, -1.0, 0.0)); //Bottom
		directions.emplace_back(Vector3<float>(0.0, 0.0, 1.0)); //Back
		directions.emplace_back(Vector3<float>(0.0, 0.0, -1.0)); //Front

		return directions;
	}

	Light::LightType OmnidirectionalLight::getLightType() const
	{
		return OMNIDIRECTIONAL;
	}

	bool OmnidirectionalLight::hasParallelBeams() const
	{
		return false;
	}

	const AABBox<float> &OmnidirectionalLight::getAABBox() const
	{
		return *bboxScope;
	}

	void OmnidirectionalLight::setAttenuation(float exponentialAttenuation)
	{
		if(exponentialAttenuation < std::numeric_limits<float>::epsilon())
		{
			throw std::domain_error("Exponential attenuation must be greater than zero.");
		}
		this->exponentialAttenuation = exponentialAttenuation;

		computeScope();
	}

	float OmnidirectionalLight::getExponentialAttenuation() const
	{
		return exponentialAttenuation;
	}

	const AABBox<float> &OmnidirectionalLight::getAABBoxScope() const
	{
		return *bboxScope;
	}

	const Sphere<float> &OmnidirectionalLight::getSphereScope() const
	{
		return *sphereScope;
	}

	/**
	 * Computes the sphere scope representing light affectation zone
	 */
	void OmnidirectionalLight::computeScope()
	{
		delete sphereScope;
		delete bboxScope;

		float radius = -std::log(attenuationNoEffect) / getExponentialAttenuation();
		sphereScope = new Sphere<float>(radius, getPosition());
		bboxScope = new AABBox<float>(getPosition()-radius, getPosition()+radius);

		notifyOctreeableMove();
	}

}
