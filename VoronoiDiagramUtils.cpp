#pragma once
#include "VoronoiDiagramUtils.h"

#pragma once
#include <map>
#include <Eigen/Dense>

ImplicitGeo::Sphere VoronoiDiagramUtils::getCircumcircle(glm::vec3 points[3])
{
	return ImplicitGeo::Sphere(glm::vec3(), 0);
}

ImplicitGeo::Sphere VoronoiDiagramUtils::getCircumsphere(glm::vec3 points[4])
{
	Eigen::Matrix4d a;

	a << points[0].x, points[0].y, points[0].z, 1.0,
		 points[1].x, points[1].y, points[1].z, 1.0,
		 points[2].x, points[2].y, points[2].z, 1.0,
		 points[3].x, points[3].y, points[3].z, 1.0;

	double detA = a.determinant();

	double lengths[4];

	for (int j = 0; j < 4; j++)
	{
		lengths[j] = pow(length(points[j]), 2.0f);
	}

	Eigen::Matrix4d x[3];
	x[0] << lengths[0], points[0].y, points[0].z, 1.0f,
			lengths[1], points[1].y, points[1].z, 1.0f,
			lengths[2], points[2].y, points[2].z, 1.0f,
			lengths[3], points[3].y, points[3].z, 1.0f;

	x[1] << lengths[0], points[0].x, points[0].z, 1.0f,
			lengths[1], points[1].x, points[1].z, 1.0f,
			lengths[2], points[2].x, points[2].z, 1.0f,
			lengths[3], points[3].x, points[3].z, 1.0f;

	x[2] << lengths[0], points[0].x, points[0].y, 1.0f,
			lengths[1], points[1].x, points[1].y, 1.0f,
			lengths[2], points[2].x, points[2].y, 1.0f,
			lengths[3], points[3].x, points[3].y, 1.0f;

	Eigen::Vector3d detX(x[0].determinant(), -x[1].determinant(), x[2].determinant());
	detX /= (2.0 * detA);
	glm::vec3 center(detX[0], detX[1], detX[2]);

	double radius = glm::length(points[3] - center);

	return ImplicitGeo::Sphere(center, radius);
}

bool VoronoiDiagramUtils::isSpaceDegenerate(glm::vec3 points[4])
{
	glm::vec3 diff1 = points[0] - points[3];
	glm::vec3 diff2 = points[1] - points[3];
	glm::vec3 diff3 = points[2] - points[3];

	Eigen::Matrix3d volumeSpace;
	volumeSpace << diff1.x, diff2.x, diff3.x,
				   diff1.y, diff2.y, diff3.y,
				   diff1.z, diff2.z, diff3.z;

	auto det = volumeSpace.determinant();
	return det > -0.00001 && det < 0.00001;
}

bool VoronoiDiagramUtils::isPointWithinSphere(glm::vec3 point, glm::vec3 points[4])
{
	double lengths[4];

	for (int j = 0; j < 4; j++)
	{
		lengths[j] = pow(glm::length(points[j]), 2.0f);
	}

	double pLength = pow(glm::length(point), 2.0f);

	Eigen::Matrix<double, 5, 5> c(5, 5);
	c << pLength, point.x, point.y, point.z, 1,
		 lengths[0], points[0].x, points[0].y, points[0].z, 1,
		 lengths[1], points[1].x, points[1].y, points[1].z, 1,
		 lengths[2], points[2].x, points[2].y, points[2].z, 1,
		 lengths[3], points[3].x, points[3].y, points[3].z, 1;

	double detC = c.fullPivLu().determinant();

	return detC > 0;
}

std::vector<glm::ivec4> VoronoiDiagramUtils::calculateDelaunayTetrahedra(const std::vector<glm::vec3>& points)
{
	DelaunayTree<glm::vec3, glm::ivec4> tree(points);
//	std::cout << "BEGIN" << std::endl;
	auto output = tree.calculate();
//	std::cout << "END" << std::endl;
	return output;
}