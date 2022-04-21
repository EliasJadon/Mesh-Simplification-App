#pragma once
#include "Utils\OptimizationUtils.h"
#include "Utils\console_color.h"

class ObjectiveFunction
{
public:
	ObjectiveFunction(const Eigen::MatrixXd& V, const Eigen::MatrixX3i& F);
	~ObjectiveFunction();
	virtual double value(Cuda::Array<double>& curr_x, const bool update) = 0;
	virtual void gradient(Cuda::Array<double>& X, const bool update) = 0;
	
	double_3 getN(const Cuda::Array<double>& X, const int fi);
	double_3 getC(const Cuda::Array<double>& X, const int fi);
	double getR(const Cuda::Array<double>& X, const int fi);
	double_3 getV(const Cuda::Array<double>& X, const int vi);

	//Finite Differences check point
	void FDGradient(const Cuda::Array<double>& X, Cuda::Array<double>& grad);
    void checkGradient(const Eigen::VectorXd& X);
    
	//weight for each objective function
	float w = 0;
	Eigen::VectorXd Efi;
	Cuda::Array<double> grad;
	double energy_value, gradient_norm;
	std::string name;
	Cuda::indices mesh_indices;
	Eigen::MatrixX3i restShapeF;
	Eigen::MatrixX3d restShapeV;
};
