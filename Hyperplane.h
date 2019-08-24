#pragma once
#include "ParametricShapes.h"

namespace Parametric
{
	class Hyperplane : public Hypersurface
	{
	protected:
		Eigen::VectorXd point;
		std::vector<Eigen::VectorXd> normals;
		std::vector<Eigen::VectorXd> directions;
	public:
		Hyperplane(const int objectDim, const int embedding) : Hypersurface(objectDim, embedding) {};

		Hyperplane(const int objectDim,
				   const int embedding,
				   const Eigen::VectorXd& point,
				   const std::vector<Eigen::VectorXd>& normals);

		Hyperplane(const int objectDim,
				   const int embedding,
				   const std::vector<Eigen::VectorXd>& points);

		~Hyperplane() {};
		bool isPointInside(const Eigen::VectorXd& point) const override;
		virtual bool intersects(const Hypersurface& otherObject) const override;
		virtual std::vector<ParametricObject> intersection(const Hypersurface& otherObject) const override;
		virtual std::vector<Eigen::VectorXd> getNormal(const Eigen::VectorXd& point) const override;
	};

	class Simplex : public Hyperplane
	{
	public:
		std::vector<Eigen::VectorXd> points;
		Simplex(const int objectDim, const int embedding, std::vector<Eigen::VectorXd> points);
		~Simplex() {};
		virtual bool intersects(const Hypersurface& otherObject) const override;
		virtual std::vector<ParametricObject> intersection(const Hypersurface& otherObject) const override;
	};
}