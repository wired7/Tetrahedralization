#pragma once
#include "LinearAlgebraUtils.h"

Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> LinearAlgebraUtils::originalEdgePseudo;
Eigen::Matrix4d LinearAlgebraUtils::originalTrianglePseudo;
Eigen::Matrix4d LinearAlgebraUtils::originalTetraPseudo;

void LinearAlgebraUtils::init()
{
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> originalEdgePointMatrix(6, 4);
	originalEdgePointMatrix <<  0, 0, 0, 1,
								0, 1, 0, 1,
								0, 0, 1, 1,
								1, 0, 0, 1,
								1, 1, 0, 1,
								1, 0, 1, 1;
	originalEdgePseudo = originalEdgePointMatrix.completeOrthogonalDecomposition().pseudoInverse();

	Eigen::Matrix4d originalTrianglePointMatrix;
	originalTrianglePointMatrix << -1, 0, 0, 1,
									0, 1, 0, 1,
									1, 0, 0, 1,
									0, 0, 1, 0;
	originalTrianglePseudo = originalTrianglePointMatrix.completeOrthogonalDecomposition().pseudoInverse().transpose();

	Eigen::Matrix4d originalTetraPointMatrix;
	originalTetraPointMatrix << -1, 0, 0, 1,
								 1, 0, 0, 1,
								 0, 1, 0, 1,
								 0, 0, 1, 1;

	originalTetraPseudo = originalTetraPointMatrix.completeOrthogonalDecomposition().pseudoInverse();
}

glm::mat4 LinearAlgebraUtils::getTransformFrom2Points(glm::vec3 p1, glm::vec3 p2)
{
	glm::vec3 dir = glm::normalize(p2 - p1);
	glm::vec3 pDir;

	for (int i = 0; i < 3; ++i)
	{
		if (dir[i] != 0)
		{
			pDir[(i + 1) % 3] = 1;
			pDir[(i + 2) % 3] = 1;
			pDir[i] = -(dir[(i + 1) % 3] + dir[(i + 2) % 3]) / dir[i];
		}
	}

	pDir = normalize(pDir);

	glm::vec3 pDir2 = normalize(glm::cross(pDir, dir));

	glm::vec3 p3 = p1 + pDir;
	glm::vec3 p4 = p1 + pDir2;
	glm::vec3 p5 = p2 + pDir;
	glm::vec3 p6 = p2 + pDir2;

	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> transformedPointMatrix(6, 4);
	transformedPointMatrix << p1.x, p1.y, p1.z, 1,
							  p3.x, p3.y, p3.z, 1,
							  p4.x, p4.y, p4.z, 1,
							  p2.x, p2.y, p2.z, 1,
							  p5.x, p5.y, p5.z, 1,
							  p6.x, p6.y, p6.z, 1;

	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> edgeTransform =
		(originalEdgePseudo * transformedPointMatrix).transpose();
	glm::mat4 glmEdgeTransform;

	for (int rows = 0; rows < 4; ++rows)
	{
		for (int cols = 0; cols < 4; ++cols)
		{
			glmEdgeTransform[cols][rows] = edgeTransform.coeff(rows, cols);
		}
	}

	return glmEdgeTransform;
}

glm::mat4 LinearAlgebraUtils::getTransformFrom3Points(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
	glm::vec3 n = glm::cross(p2 - p1, p3 - p2);

	Eigen::Matrix4d transformedPointMatrix;
	transformedPointMatrix << p1.x, p2.x, p3.x, n.x,
							  p1.y, p2.y, p3.y, n.y,
							  p1.z, p2.z, p3.z, n.z,
							  1, 1, 1, 0;

	Eigen::Matrix4d triangleTransform = transformedPointMatrix * originalTrianglePseudo;
	glm::mat4 glmTriangleTransform;

	for (int rows = 0; rows < 4; ++rows)
	{
		for (int cols = 0; cols < 4; ++cols)
		{
			glmTriangleTransform[cols][rows] = triangleTransform.coeff(rows, cols);
		}
	}

	return glmTriangleTransform;
}

glm::mat4 LinearAlgebraUtils::getTransformFrom4Points(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4)
{
	Eigen::Matrix4d transformedPointMatrix;

	transformedPointMatrix << p1.x, p1.y, p1.z, 1.0,
							  p2.x, p2.y, p2.z, 1.0,
							  p3.x, p3.y, p3.z, 1.0,
							  p4.x, p4.y, p4.z, 1.0;

	Eigen::Matrix4d tetraTransform = (originalTetraPseudo * transformedPointMatrix).transpose();
	glm::mat4 glmTetraTransform;

	for (int rows = 0; rows < 4; ++rows)
	{
		for (int cols = 0; cols < 4; ++cols)
		{
			glmTetraTransform[cols][rows] = tetraTransform.coeff(rows, cols);
		}
	}

	return glmTetraTransform;
}