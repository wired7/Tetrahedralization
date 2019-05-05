#pragma once
#include "ImplicitGeometry.h"

namespace ImplicitGeo
{
	Plane::Plane(glm::vec3 point, glm::vec3 normal)
	{
		this->point = point;
		this->normal = glm::normalize(normal);
	}

	Plane::Plane(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3)
	{
		point = point1;
		normal = glm::normalize(cross(point2 - point1, point3 - point1));
	}

	bool Plane::intersects(glm::vec3 origin, glm::vec3 direction) const
	{
		return glm::dot(normal, direction);
	}

	float Plane::intersection(glm::vec3 origin, glm::vec3 direction) const
	{
		float t = INFINITY;
		if (Plane::intersects(origin, direction) != 0.0)
		{
			t = glm::dot(normal, point - origin) / dot(normal, direction);
		}
		return t;
	}

	glm::vec3 Plane::getNormal(glm::vec3 point)
	{
		return normal;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Sphere::Sphere(glm::vec3 center, double radius)
	{
		this->center = center;
		this->radius = radius;
	}

	bool Sphere::intersects(glm::vec3 origin, glm::vec3 direction) const
	{
		double discriminant = pow(glm::dot(direction, origin - center), 2) - pow(glm::length(direction), 2) *
							 (pow(glm::length(origin - center), 2) - radius * radius);
		return discriminant > 0;
	}

	float Sphere::intersection(glm::vec3 origin, glm::vec3 direction) const
	{
		double t = -1;
		if (intersects(origin, direction))
		{
			double d = glm::dot(direction, origin - center);
			double e = sqrt(pow(d, 2) - pow(glm::length(direction), 2) * (pow(glm::length(origin - center), 2) - radius * radius));
			double t1 = (e - d) / pow(glm::length(direction), 2);
			double t2 = -(d + e) / pow(glm::length(direction), 2);

			if (t1 > 0 && (t1 <= t2 || t2 < 0))
				t = t1;
			else if (t2 > 0 && (t2 <= t1 || t1 < 0))
				t = t2;
		}
		return t;
	}

	glm::vec3 Sphere::getNormal(glm::vec3 point)
	{
		return glm::normalize(point - center);
	}

	double Sphere::getRadius()
	{
		return radius;
	}

	void Sphere::setPosition(glm::vec3 pos) {
		center = pos;
	}

	void Sphere::translate(glm::vec3 trans) {
		center += trans;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Triangle::Triangle(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3)
	{
		point1 = pt1;
		point2 = pt2;
		point3 = pt3;
		point = point1;
		normal = glm::normalize(glm::cross(point2 - point1, point3 - point2));
	}

	float Triangle::area(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3)
	{
		return 0.5f * glm::length(glm::cross(pt2 - pt1, pt3 - pt1));
	}

	float Triangle::intersection(glm::vec3 origin, glm::vec3 direction) const
	{
		float t = Plane::intersection(origin, direction);
		if (abs(t) != INFINITY)
		{
			glm::vec3 p = origin + t * direction;

			glm::vec3 v0 = point3 - point1;
			glm::vec3 v1 = point2 - point1;
			glm::vec3 v2 = p - point1;

			// Compute dot products	
			float dot00 = glm::dot(v0, v0);
			float dot01 = glm::dot(v0, v1);
			float dot02 = glm::dot(v0, v2);
			float dot11 = glm::dot(v1, v1);
			float dot12 = glm::dot(v1, v2);

			// Compute barycentric coordinates
			float d = dot00 * dot11 - dot01 * dot01;
			float u = (dot11 * dot02 - dot01 * dot12) / d;
			float v = (dot00 * dot12 - dot01 * dot02) / d;

			// Check if point is in triangle
			if ((u >= -0.001f) && (v >= -0.001f) && (u + v < 1.001f))
			{
				return t;
			}
		}

		return INFINITY;
	}

	bool Triangle::intersects(const Triangle& triangle) const
	{
		float intersections[6] = { intersection(triangle.point1, triangle.point2 - triangle.point1),
								   intersection(triangle.point2, triangle.point3 - triangle.point2),
								   intersection(triangle.point3, triangle.point1 - triangle.point3),
								   triangle.intersection(point1, point2 - point1),
								   triangle.intersection(point2, point3 - point2),
								   triangle.intersection(point3, point1 - point3) };

		for (int i = 0; i < 6; ++i)
		{
			if (abs(intersections[i]) <= 1.001f)
			{
				return true;
			}
		}

		return false;
	}
}