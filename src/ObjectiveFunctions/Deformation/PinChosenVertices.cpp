#include "ObjectiveFunctions/Deformation/PinChosenVertices.h"

using namespace ObjectiveFunctions::Deformation;

PinChosenVertices::PinChosenVertices(const Eigen::MatrixXd& V, const Eigen::MatrixX3i& F)
	: ObjectiveFunctions::Basic{ V,F }
{
    name = "Pin Chosen Vertices";
	w = 500;
	Constraints_Position.resize(V.rows(), 3);
	std::cout << "\t" << name << " constructor" << std::endl;
}

PinChosenVertices::~PinChosenVertices()
{
	std::cout << "\t" << name << " destructor" << std::endl;
}

void PinChosenVertices::insertConstraint(const int new_vertex, const Eigen::MatrixX3d& V)
{
	m_value.lock();
	m_gradient.lock();
	Constraints_indices.insert(new_vertex);
	Constraints_Position.row(new_vertex) = V.row(new_vertex);
	m_gradient.unlock();
	m_value.unlock();
}

void PinChosenVertices::translateConstraint(const int vertex, const Eigen::RowVector3d& translation)
{
	m_value.lock();
	m_gradient.lock();
	Constraints_Position.row(vertex) += translation;
	m_gradient.unlock();
	m_value.unlock();
}

void PinChosenVertices::eraseConstraint(const int vertex)
{
	m_value.lock();
	m_gradient.lock();
	Constraints_indices.erase(vertex);
	m_gradient.unlock();
	m_value.unlock();
}

void PinChosenVertices::clearConstraints()
{
	m_value.lock();
	m_gradient.lock();
	Constraints_indices.clear();
	m_gradient.unlock();
	m_value.unlock();
}

std::set<int> PinChosenVertices::getConstraintsIndices() {
	return Constraints_indices;
}

double PinChosenVertices::value(Cuda::Array<double>& curr_x, const bool update)
{
	m_value.lock();
	double value = 0;
	for (int v_index : Constraints_indices) {
		double_3 Vi = getV(curr_x, v_index);
		value += squared_norm(sub(Vi, Constraints_Position.row(v_index)));
	}
	m_value.unlock();

	if (update)
		energy_value = value;
	return value;
}

void PinChosenVertices::gradient(Cuda::Array<double>& X, const bool update)
{
	for (int i = 0; i < grad.size; i++)
		grad.host_arr[i] = 0;

	m_gradient.lock();
	for (int v_index : Constraints_indices) {
		grad.host_arr[v_index + mesh_indices.startVx] = 2 * (X.host_arr[v_index + mesh_indices.startVx] - Constraints_Position(v_index, 0));
		grad.host_arr[v_index + mesh_indices.startVy] = 2 * (X.host_arr[v_index + mesh_indices.startVy] - Constraints_Position(v_index, 1));
		grad.host_arr[v_index + mesh_indices.startVz] = 2 * (X.host_arr[v_index + mesh_indices.startVz] - Constraints_Position(v_index, 2));
	}
	m_gradient.unlock();

	if (update) {
		gradient_norm = 0;
		for (int i = 0; i < grad.size; i++)
			gradient_norm += pow(grad.host_arr[i], 2);
	}
}