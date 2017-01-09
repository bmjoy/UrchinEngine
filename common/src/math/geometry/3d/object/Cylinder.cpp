#include <limits>

#include "Cylinder.h"

namespace urchin
{

	template<class T> Cylinder<T>::Cylinder():
		cylinderShape(CylinderShape<T>(0.0, 0.0, CylinderShape<T>::CYLINDER_X)),
		centerPosition(Point3<T>(0.0, 0.0, 0.0))
	{
		axis[0] = Vector3<T>(0.0, 0.0, 0.0);
		axis[1] = Vector3<T>(0.0, 0.0, 0.0);
		axis[2] = Vector3<T>(0.0, 0.0, 0.0);
	}

	template<class T> Cylinder<T>::Cylinder(T radius, T height, typename CylinderShape<T>::CylinderOrientation cylinderOrientation,
			const Point3<T> &centerPosition, const Quaternion<T> &orientation) :
		cylinderShape(CylinderShape<T>(radius, height, cylinderOrientation)),
		centerPosition(centerPosition),
		orientation(orientation)
	{
		axis[0] = orientation.rotatePoint(Point3<T>(1.0, 0.0, 0.0)).toVector();
		axis[1] = orientation.rotatePoint(Point3<T>(0.0, 1.0, 0.0)).toVector();
		axis[2] = orientation.rotatePoint(Point3<T>(0.0, 0.0, 1.0)).toVector();
	}

	template<class T> Cylinder<T>::~Cylinder()
	{

	}

	template<class T> T Cylinder<T>::getRadius() const
	{
		return cylinderShape.getRadius();
	}

	template<class T> T Cylinder<T>::getHeight() const
	{
		return cylinderShape.getHeight();
	}

	template<class T> typename CylinderShape<T>::CylinderOrientation Cylinder<T>::getCylinderOrientation() const
	{
		return cylinderShape.getCylinderOrientation();
	}

	template<class T> const Point3<T> &Cylinder<T>::getCenterPosition() const
	{
		return centerPosition;
	}

	template<class T> const Quaternion<T> &Cylinder<T>::getOrientation() const
	{
		return orientation;
	}

	/**
	 * @return Cylinder normalized axis for given index
	 */
	template<class T> const Vector3<T> &Cylinder<T>::getAxis(unsigned int index) const
	{
		return axis[index];
	}

	template<class T> Point3<T> Cylinder<T>::getSupportPoint(const Vector3<T> &direction) const
	{
		Vector3<T> normalizedDirection;
		if(direction.X==0.0 && direction.Y==0.0 && direction.Z==0.0)
		{
			normalizedDirection = Vector3<T>(1.0, 0.0, 0.0);
		}else
		{
			normalizedDirection = direction.normalize();
		}
		Vector3<T> projectedDirectionOnCircle = normalizedDirection - (normalizedDirection.dotProduct(axis[getCylinderOrientation()]) * axis[getCylinderOrientation()]);
		if(projectedDirectionOnCircle.squareLength() > std::numeric_limits<T>::epsilon())
		{
			projectedDirectionOnCircle = projectedDirectionOnCircle.normalize();
		}

		Point3<T> cirlcePosition1 = centerPosition.translate(axis[getCylinderOrientation()] * (T)(getHeight() / (T)2.0));
		Point3<T> supportPoint1 = cirlcePosition1.translate(projectedDirectionOnCircle * getRadius());

		Point3<T> cirlcePosition2 = centerPosition.translate(axis[getCylinderOrientation()] * (T)(-getHeight() / (T)2.0));
		Point3<T> supportPoint2 = cirlcePosition2.translate(projectedDirectionOnCircle * getRadius());

		if(normalizedDirection.dotProduct(centerPosition.vector(supportPoint1).normalize()) > normalizedDirection.dotProduct(centerPosition.vector(supportPoint2).normalize()))
		{
			return supportPoint1;
		}

		return supportPoint2;
	}

	//explicit template
	template class Cylinder<float>;
	template class Cylinder<double>;

}
