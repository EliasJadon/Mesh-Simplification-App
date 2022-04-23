#include "GUIExtensions/MeshSimplificationData.h"

using namespace GUIExtensions;

MeshSimplificationData::MeshSimplificationData(
	const int CoreID,
	const int meshID,
	igl::opengl::glfw::Viewer* viewer)
{
	this->CoreID = CoreID;
	this->ModelID = meshID;
	showFacesNorm = showSphereEdges = showNormEdges = showTriangleCenters = showSphereCenters = false;
}

std::vector<int> MeshSimplificationData::GlobNeighSphereCenters(
	const int fi, 
	const float distance) 
{
	std::vector<int> Neighbors; Neighbors.clear();
	for (int i = 0; i < C.rows(); i++)
		if (((C.row(fi) - C.row(i)).norm() + abs(R(fi) - R(i))) < distance)
			Neighbors.push_back(i);
	return Neighbors;
}

std::vector<int> MeshSimplificationData::FaceNeigh(const Eigen::Vector3d center, const float distance) 
{
	std::vector<int> Neighbors; Neighbors.clear();
	for (int i = 0; i < center_of_faces.rows(); i++) {
		double x = center(0) - center_of_faces(i, 0);
		double y = center(1) - center_of_faces(i, 1);
		double z = center(2) - center_of_faces(i, 2);
		if ((pow(x, 2) + pow(y, 2) + pow(z, 2)) < pow(distance,2))
			Neighbors.push_back(i);
	}
	return Neighbors;
}

std::vector<int> MeshSimplificationData::GlobNeighNorms(const int fi, const float distance) 
{
	std::vector<int> Neighbors; Neighbors.clear();
	for (int i = 0; i < N.rows(); i++)
		if ((N.row(fi) - N.row(i)).squaredNorm() < distance)
			Neighbors.push_back(i);
	return Neighbors;
}

std::vector<int> MeshSimplificationData::getNeigh(
	const app_utils::Neighbor_Type type, 
	const Eigen::MatrixXi& F, 
	const int fi, 
	const float distance) 
{
	std::vector<int> neigh;
	if (type == app_utils::Neighbor_Type::CURR_FACE)
		return { fi };
	if (type == app_utils::Neighbor_Type::GLOBAL_NORMALS)
		return GlobNeighNorms(fi, distance);
	if (type == app_utils::Neighbor_Type::GLOBAL_SPHERE)
		return GlobNeighSphereCenters(fi, distance);
	if (type == app_utils::Neighbor_Type::LOCAL_NORMALS)
		neigh = GlobNeighNorms(fi, distance);
	else if (type == app_utils::Neighbor_Type::LOCAL_SPHERE)
		neigh = GlobNeighSphereCenters(fi, distance);
	
	//pick only adjanced faces in order to get local faces
	std::vector<int> result; result.push_back(fi);
	std::vector<std::vector<std::vector<int>>> TT;
	igl::triangle_triangle_adjacency(F, TT);
	int prevSize;
	do {
		prevSize = result.size();
		result = vectorsIntersection(adjSetOfTriangles(F, result, TT), neigh);
	} while (prevSize != result.size());
	return result;
}

std::vector<int> MeshSimplificationData::adjSetOfTriangles(
	const Eigen::MatrixXi& F, 
	const std::vector<int> selected, 
	std::vector<std::vector<std::vector<int>>> TT) 
{
	std::vector<int> adj = selected;
	for (int selectedFace : selected) {
		for (std::vector<int> _ : TT[selectedFace]) {
			for (int fi : _) {
				if (std::find(adj.begin(), adj.end(), fi) == adj.end())
					adj.push_back(fi);
			}
		}
	}
	return adj;
}

std::vector<int> MeshSimplificationData::vectorsIntersection(
	const std::vector<int>& A, 
	const std::vector<int>& B) 
{
	std::vector<int> intersection;
	for (int fi : A) {
		if (std::find(B.begin(), B.end(), fi) != B.end())
			intersection.push_back(fi);
	}
	return intersection;
}

void MeshSimplificationData::initFaceColors(
	const int numF,
	const Eigen::Vector3f center_sphere_color,
	const Eigen::Vector3f center_vertex_color,
	const Eigen::Vector3f centers_sphere_edge_color,
	const Eigen::Vector3f centers_norm_edge_color,
	const Eigen::Vector3f face_norm_color)
{
	color_per_face.resize(numF, 3);
	color_per_sphere_center.resize(numF, 3);
	color_per_vertex_center.resize(numF, 3);
	color_per_face_norm.resize(numF, 3);
	color_per_sphere_edge.resize(numF, 3);
	color_per_norm_edge.resize(numF, 3);
	for (int fi = 0; fi < numF; fi++) {
		color_per_sphere_center.row(fi) = center_sphere_color.cast<double>();
		color_per_vertex_center.row(fi) = center_vertex_color.cast<double>();
		color_per_face_norm.row(fi) = face_norm_color.cast<double>();
		color_per_sphere_edge.row(fi) = centers_sphere_edge_color.cast<double>();
		color_per_norm_edge.row(fi) = centers_norm_edge_color.cast<double>();
	}
}

void MeshSimplificationData::setFaceColors(const int fi, const Eigen::Vector3d color) {
	color_per_face.row(fi) = color;
	color_per_sphere_center.row(fi) = color;
	color_per_vertex_center.row(fi) = color;
	color_per_face_norm.row(fi) = color;
	color_per_sphere_edge.row(fi) = color;
	color_per_norm_edge.row(fi) = color;
}

void MeshSimplificationData::shiftFaceColors(
	const int fi, 
	const double alpha,
	const Eigen::Vector3f model_color,
	const Eigen::Vector3f color) 
{
	double w = std::min<double>(std::max<double>(alpha, 0), 1);
	auto averaged = color.cast<double>() * w + model_color.cast<double>() * (1 - w);
	color_per_face.row(fi) = averaged;
	color_per_sphere_center.row(fi) = averaged;
	color_per_vertex_center.row(fi) = averaged;
	color_per_face_norm.row(fi) = averaged;
	color_per_sphere_edge.row(fi) = averaged;
	color_per_norm_edge.row(fi) = averaged;
}

void MeshSimplificationData::initMinimizers(
	const Eigen::MatrixXd& V, const Eigen::MatrixXi& F,
	const OptimizationUtils::InitSphereAuxVariables& typeAuxVar,
	const int distance_from, const int distance_to,
	const double minus_normals_radius_length)
{
	Eigen::MatrixX3d normals;
	igl::per_face_normals((Eigen::MatrixX3d)V, (Eigen::MatrixX3i)F, normals);

	Eigen::MatrixXd center0;
	Eigen::VectorXd Radius0;

	if (typeAuxVar == OptimizationUtils::InitSphereAuxVariables::SPHERE_FIT)
		OptimizationUtils::Least_Squares_Sphere_Fit(distance_from, distance_to, V, F, center0, Radius0);
	else if (typeAuxVar == OptimizationUtils::InitSphereAuxVariables::MODEL_CENTER_POINT)
		OptimizationUtils::center_of_mesh(V, F, center0, Radius0);
	else if (typeAuxVar == OptimizationUtils::InitSphereAuxVariables::MINUS_NORMALS) {
		this->center_of_faces = OptimizationUtils::center_per_triangle(V, F);
		Radius0.resize(F.rows());
		center0.resize(F.rows(), 3);
		Radius0.setConstant(minus_normals_radius_length);
		for (int i = 0; i < center0.rows(); i++)
			center0.row(i) = this->center_of_faces.row(i) - Radius0(i) * normals.row(i);
	}
	
	Eigen::MatrixXd cylinder_dir0(F.rows(), 3);
	for (int fi = 0; fi < F.rows(); fi++) {
		const int v0 = F(fi, 0);
		const int v1 = F(fi, 1);
		const int v2 = F(fi, 2);
		cylinder_dir0.row(fi) = (V.row(v1) - V.row(v0)).normalized();
	}
	
	this->center_of_faces = OptimizationUtils::center_per_triangle(V, F);
	this->C = center0;
	this->R = Radius0;
	this->N = normals;
	this->A = cylinder_dir0;

	minimizer->init(totalObjective, V, F, normals, center0, Radius0, cylinder_dir0);
}

Eigen::MatrixX4d MeshSimplificationData::getValues(const app_utils::Face_Colors face_coloring_Type) {
	Eigen::MatrixX4d values(N.rows(), 4);
	for (int fi = 0; fi < N.rows(); fi++) {
		values.row(fi) = Eigen::Vector4d(N(fi, 0), N(fi, 1), N(fi, 2), 0);
		if (face_coloring_Type == app_utils::Face_Colors::SPHERES_CLUSTERING)
			values.row(fi) = Eigen::Vector4d(C(fi, 0), C(fi, 1), C(fi, 2), R[fi]);
	}
	return values;
}

