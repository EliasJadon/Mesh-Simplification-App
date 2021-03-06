#include "ObjectiveFunctions/Basic.h"

using namespace ObjectiveFunctions;

Basic::Basic(const Eigen::MatrixXd& V, const Eigen::MatrixX3i& F) {
	restShapeV = V;
	restShapeF = F;
	
	int numV = restShapeV.rows();
	int numF = restShapeF.rows();
	int numH = OptimizationUtils::getNumberOfHinges(F);

	Cuda::initIndices(mesh_indices, numF, numV, numH);
	Cuda::AllocateMemory(grad, mesh_indices.total_variables);
	for (int i = 0; i < grad.size; i++) {
		grad.host_arr[i] = 0;
	}
	Efi.resize(F.rows());
	Efi.setZero();
	w = 0;
	energy_value = 0;
	gradient_norm = 0;
	name = "Basic";
	std::cout << "\t" << name << " constructor" << std::endl;
}

Basic::~Basic() {
	Cuda::FreeMemory(grad);
	std::cout << "\t" << name << " destructor" << std::endl;
}

double_3 Basic::getN(const Cuda::Array<double>& X, const int fi) {
	return double_3(
		X.host_arr[fi + mesh_indices.startNx],
		X.host_arr[fi + mesh_indices.startNy],
		X.host_arr[fi + mesh_indices.startNz]
	);
}

double_3 Basic::getC(const Cuda::Array<double>& X, const int fi) {
	return double_3(
		X.host_arr[fi + mesh_indices.startCx],
		X.host_arr[fi + mesh_indices.startCy],
		X.host_arr[fi + mesh_indices.startCz]
	);
}

double_3 Basic::getA(const Cuda::Array<double>& X, const int fi) {
	return double_3(
		X.host_arr[fi + mesh_indices.startAx],
		X.host_arr[fi + mesh_indices.startAy],
		X.host_arr[fi + mesh_indices.startAz]
	);
}

double Basic::getR(const Cuda::Array<double>& X, const int fi) {
	return X.host_arr[fi + mesh_indices.startR];
}

double_3 Basic::getV(const Cuda::Array<double>& X, const int vi) {
	return double_3(
		X.host_arr[vi + mesh_indices.startVx],
		X.host_arr[vi + mesh_indices.startVy],
		X.host_arr[vi + mesh_indices.startVz]
	);
}

void Basic::FDGradient(const Cuda::Array<double>& X, Cuda::Array<double>& grad)
{
	Cuda::Array<double> Xd;
	Cuda::AllocateMemory(grad, X.size);
	Cuda::AllocateMemory(Xd, X.size);
	for (int i = 0; i < X.size; i++) {
		Xd.host_arr[i] = X.host_arr[i];
	}
	const double dX = 1e-4;
	double f_P, f_M;

    //this is a very slow method that evaluates the gradient of the objective function through FD...
	for (int i = 0; i < X.size; i++) {
        Xd.host_arr[i] = X.host_arr[i] + dX;
		f_P = value(Xd,false);

        Xd.host_arr[i] = X.host_arr[i] - dX;
		f_M = value(Xd,false);

        //now reset the ith param value
        Xd.host_arr[i] = X.host_arr[i];
        grad.host_arr[i] = (f_P - f_M) / (2 * dX);
    }
}

void Basic::checkGradient(const Eigen::VectorXd& X)
{
	Cuda::Array<double> XX;
	Cuda::AllocateMemory(XX, X.size());
	for (int i = 0; i < XX.size; i++) {
		XX.host_arr[i] = X(i);
	}

	gradient(XX, false);
	Cuda::Array<double>& Analytic_gradient = this->grad;
	Cuda::Array<double> FD_gradient;
	FDGradient(XX, FD_gradient);
	assert(FD_gradient.size == Analytic_gradient.size && "The size of analytic gradient & FD gradient must be equal!");
	double tol = 1e-4;
	double eps = 1e-10;
	

	double Analytic_gradient_norm = 0;
	double FD_gradient_norm = 0;
	for (int i = 0; i < XX.size; i++) {
		Analytic_gradient_norm += pow(Analytic_gradient.host_arr[i], 2);
		FD_gradient_norm += pow(FD_gradient.host_arr[i], 2);
	}
	
	std::cout << "\n\n" << name << ": g.norm() = " << Analytic_gradient_norm << "(analytic) , " << FD_gradient_norm << "(FD)" << std::endl;
	for (int i = 0; i < Analytic_gradient.size; i++) {
        double absErr = abs(FD_gradient.host_arr[i] - Analytic_gradient.host_arr[i]);
        double relError = 2 * absErr / (eps + Analytic_gradient.host_arr[i] + FD_gradient.host_arr[i]);
        if (relError > tol && absErr > 1e-5) {
			std::cout << name << "\t" << i << ":\tAnalytic val: " <<
				Analytic_gradient.host_arr[i] << ", FD val: " << FD_gradient.host_arr[i] <<
				". Error: " << absErr << "(" << relError * 100 << "%%)\n";
        }
    }
}
