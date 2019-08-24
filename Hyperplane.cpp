#include "Hyperplane.h"
#include <Eigen/Dense>
#include <Eigen/QR>

namespace Parametric
{
	Hyperplane::Hyperplane(const int objectDim,
						   const int embedding,
						   const Eigen::VectorXd& point,
						   const std::vector<Eigen::VectorXd>& normals) :
		Hypersurface(objectDim, embedding), point(point)
	{
		Eigen::MatrixXd normalBases;

		for (const auto& normal : normals)
		{
			Eigen::VectorXd tempNormal(normal);

			tempNormal.normalize();

			normalBases.conservativeResize(normalBases.rows() + 1, embedding);
			normalBases.row(normalBases.rows() - 1) = tempNormal.transpose();

			this->normals.push_back(tempNormal);
		}

		Eigen::FullPivLU<Eigen::MatrixXd> lu(normalBases);
		Eigen::MatrixXd nullSpace = lu.kernel();

		for (int iter = 0; iter < nullSpace.cols(); ++iter)
		{
			directions.push_back(nullSpace.col(iter).normalized());
		}
	}

	Hyperplane::Hyperplane(const int objectDim, const int embedding, const std::vector<Eigen::VectorXd>& points) :
		Hypersurface(objectDim, embedding), point(points[0])
	{
		Eigen::MatrixXd basisMatrix;
		basisMatrix.conservativeResize(points.size() - 1, embedding);

		for (int iter = 0; iter < objectDim; ++iter)
		{
			basisMatrix.row(iter) = (points[iter + 1] - points[iter]).transpose();
		}

		Eigen::FullPivLU<Eigen::MatrixXd> lu(basisMatrix);
		Eigen::MatrixXd nullSpace = lu.kernel();

		for (int iter = 0; iter < nullSpace.cols(); ++iter)
		{
			normals.push_back(nullSpace.col(iter).normalized());
		}

		basisMatrix = basisMatrix.transpose().householderQr().householderQ();

		for (int iter = 0; iter < nullSpace.cols(); ++iter)
		{
			directions.push_back(basisMatrix.col(iter));
		}
	}

	bool Hyperplane::intersects(const Hypersurface& otherObject) const
	{
		try
		{
			const Hyperplane& hp = dynamic_cast<const Hyperplane&>(otherObject);

			for (const auto& direction : directions)
			{
				for (const auto& normal : hp.normals)
				{
					if (direction.dot(normal) != 0.0)
					{
						return false;
					}
				}
			}

			return true;
		}
		catch (...)
		{
			// Try sphere, then simplex
		}
		
		return false;
	}

	std::vector<ParametricObject> Hyperplane::intersection(const Hypersurface& otherObject) const
	{
		std::vector<Parametric::ParametricObject> t;

		if (intersects(otherObject))
		{
			try
			{
				const Hyperplane& hp = dynamic_cast<const Hyperplane&>(otherObject);

				Eigen::MatrixXd solutionMatrix;

				for (const auto& direction : hp.directions)
				{
					solutionMatrix.conservativeResize(embedding, solutionMatrix.cols() + 1);
					solutionMatrix.col(solutionMatrix.cols() - 1) = direction.transpose();
				}

				for (const auto& direction : directions)
				{
					solutionMatrix.conservativeResize(embedding, solutionMatrix.cols() + 1);
					solutionMatrix.col(solutionMatrix.cols() - 1) = - direction.transpose();
				}

				Eigen::MatrixXd x = solutionMatrix.fullPivLu().solve(point - hp.point);

				for (int iter = 0; iter < x.rows(); ++iter)
				{
					t.push_back(Point(x.row(iter)));
				}
			}
			catch (...)
			{
				// Try sphere, then simplex
			}
		}

		return t;
	}

	std::vector<Eigen::VectorXd> Hyperplane::getNormal(const Eigen::VectorXd& point) const
	{
		return normals;
	}

	bool Hyperplane::isPointInside(const Eigen::VectorXd& point) const
	{
		auto dirVec = point - this->point;

		for (const auto& normal : normals)
		{
			if (normal.dot(dirVec) != 0)
			{
				return false;
			}
		}

		return true;
	}

	Simplex::Simplex(const int objectDim, const int embedding, std::vector<Eigen::VectorXd> points) :
		Hyperplane(objectDim, embedding, points), points(points)
	{
	}

	std::vector<ParametricObject> Simplex::intersection(const Hypersurface& otherObject) const
	{
/*		float t = Hyperplane::intersection(origin, direction);
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

		return INFINITY;*/
		return {};
	}

	bool Simplex::intersects(const Hypersurface& otherObject) const
	{
/*		float intersections[6] = { intersection(triangle.point1, triangle.point2 - triangle.point1),
			intersection(triangle.point2, triangle.point3 - triangle.point2),
			intersection(triangle.point3, triangle.point1 - triangle.point3),
			triangle.intersection(point1, point2 - point1),
			triangle.intersection(point2, point3 - point2),
			triangle.intersection(point3, point1 - point3) };

		const Simplex& otherSimplex = dynamic_cast<const Simplex&>(otherObject);


		for (int iter = 0; iter < otherSimplex.points.size(); ++iter)
		{
			auto intersect = intersection(Hyperplane(1, embedding, { points[iter], points[iter + 1] }));
			double value = intersect[0].getVector()[0];
			if (value > 0.0 && value < 1.0)
			{
				return true;
			}
		}
*/
		return false;
	}
}