#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/rotate_vector.hpp>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <vector>
#include <limits>

static class LinearAlgebraUtils
{
public:
	static void init();
	static glm::mat4 getTransformFrom2Points(glm::vec3 point1, glm::vec3 point2);
	static glm::mat4 getTransformFrom3Points(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	static glm::mat4 getTransformFrom4Points(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);

	template <typename embeddingDimVector>
	static double getSimplexOrientation(std::vector<embeddingDimVector> points)
	{
		int columns = max(points.size(), points[0].length());
		int leftover = columns - points[0].length();

		Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> matrix(columns, columns);

		for (int i = 0; i < points.size(); ++i)
		{
			int j = 0;
			for (; j < points[i].length(); ++j)
			{
				matrix(i, j) = points[i][j];
			}

			for (int k = 0; k < leftover; ++k, ++j)
			{
				matrix(i, j) = 1;
			}
		}

		return sign(matrix.determinant());
	}

	static Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> originalEdgePseudo;
	static Eigen::Matrix4d originalTrianglePseudo;
	static Eigen::Matrix4d originalTetraPseudo;

	template <typename embeddingDimVector>
	static bool isPointInsideSimplex(std::vector<embeddingDimVector> simplexVertices, embeddingDimVector point)
	{
		int columns = point.length();
		Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> gramianMatrix(columns, columns);

		int j = 0;
		for (int a = 0, rowCount = 0; rowCount < columns; ++a)
		{
			for (int b = 0; a != j && b < columns; ++b)
			{
				double result = static_cast<double>(simplexVertices[a][b]) - static_cast<double>(simplexVertices[0][b]);
				gramianMatrix(rowCount, b) = result;
			}

			if (a != j)
			{
				rowCount++;
			}
		}

//		std::cout << gramianMatrix << std::endl << std::endl;
		
		double det11 = sign(gramianMatrix.determinant());
		
		for (int j = 0; j < columns; ++j)
		{
			Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> otherMatrix(columns, columns);

			for (int a = 0, rowCount = 0; rowCount < columns; ++a)
			{
				for (int b = 0; a != j && b < columns; ++b)
				{
					double result = static_cast<double>(simplexVertices[a][b]) - static_cast<double>(point[b]);

					otherMatrix(rowCount, b) = result;
				}

				if (a != j)
				{
					rowCount++;
				}
			}

			double detj0 = sign(otherMatrix.determinant());
			if (j % 2)
			{
				detj0 *= -1;
			}
			
/*			std::cout << otherMatrix << std::endl;*/
			
			if (detj0 != 0.0 && det11 != detj0)
			{
//				std::cout << det11 << " " << detj0 << std::endl;
				return false;
			}
		}
//		std::cout << "PASSED" << std::endl;
		return true;
	}
};

