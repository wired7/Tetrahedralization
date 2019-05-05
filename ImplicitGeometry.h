#pragma once
#include <vector>
#include <iostream>
#include "glm.hpp"

namespace ImplicitGeo
{
	class ImplicitGeometry
	{
	public:
		ImplicitGeometry() {};
		~ImplicitGeometry() {};
		virtual bool intersects(glm::vec3 origin, glm::vec3 direction) const = 0;
		virtual float intersection(glm::vec3 origin, glm::vec3 direction) const = 0;
		virtual glm::vec3 getNormal(glm::vec3 point) = 0;
		virtual void setPosition(glm::vec3 pos) = 0;
		virtual void translate(glm::vec3 trans) = 0;
	};

	class Plane : public ImplicitGeometry
	{
	public:
		glm::vec3 point;
		glm::vec3 normal;
		Plane() {};
		Plane(glm::vec3, glm::vec3);
		Plane(glm::vec3, glm::vec3, glm::vec3);
		~Plane() {};
		virtual bool intersects(glm::vec3, glm::vec3) const override;
		virtual float intersection(glm::vec3, glm::vec3) const override;
		glm::vec3 getNormal(glm::vec3);
		void setPosition(glm::vec3) {};
		void translate(glm::vec3) {};
	};

	class Sphere : public ImplicitGeometry
	{
	public:
		glm::vec3 center;
		float radius;
		Sphere(glm::vec3, double);
		~Sphere() {};
		bool intersects(glm::vec3, glm::vec3) const override;
		float intersection(glm::vec3, glm::vec3) const override;
		glm::vec3 getNormal(glm::vec3);
		double getRadius();
		void setPosition(glm::vec3);
		void translate(glm::vec3);
	};

	class Triangle : public Plane
	{
	public:
		glm::vec3 point1;
		glm::vec3 point2;
		glm::vec3 point3;
		Triangle(glm::vec3, glm::vec3, glm::vec3);
		~Triangle() {};
		float area(glm::vec3, glm::vec3, glm::vec3);
		bool intersects(const Triangle&) const;
		bool intersects(glm::vec3, glm::vec3) const override { return false; };
		float intersection(glm::vec3 origin, glm::vec3 direction) const override;
	};
}