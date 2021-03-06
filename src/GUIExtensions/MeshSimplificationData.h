#pragma once
#include "Utils/GUI.h"
#include "NumericalOptimizations/InitAuxVar.h"

namespace GUIExtensions {
	class MeshSimplificationData {
	public:
		Eigen::MatrixXd center_of_faces, C, N, A;
		Eigen::VectorXd R;
		std::shared_ptr<NumericalOptimizations::Basic> minimizer;

		Clustering_Colors clustering_colors;
		std::vector<std::vector<int>> clustering_faces_indices;
		std::vector<std::pair<int, int>> SelfIntersection_pairs, flippedFaces_pairs;
		Eigen::MatrixX3d clustering_faces_colors;
		
		float prev_camera_zoom;
		Eigen::Vector3f prev_camera_translation;
		Eigen::Quaternionf prev_trackball_angle;
		Eigen::MatrixXd color_per_face, color_per_sphere_center, color_per_vertex_center;
		Eigen::MatrixXd color_per_face_norm, color_per_sphere_edge, color_per_norm_edge;
		int ModelID, CoreID;
		ImVec2 screen_position, screen_size, results_window_position, outputs_window_position;
		bool showSphereEdges, showCylinderDir, showNormEdges, showTriangleCenters, showSphereCenters, showFacesNorm;

		MeshSimplificationData(
			const Eigen::MatrixXd& V,
			const Eigen::MatrixXi& F,
			const int CoreID,
			const int meshID,
			igl::opengl::glfw::Viewer* viewer);
		~MeshSimplificationData() = default;
		std::vector<int> GlobNeighSphereCenters(const int fi, const float distance);
		std::vector<int> GlobNeighCylinders(const int fi, const float distance);
		std::vector<int> FaceNeigh(const Eigen::Vector3d center, const float distance);
		std::vector<int> GlobNeighNorms(const int fi, const float distance);
		std::vector<int> getNeigh(const app_utils::Neighbor_Type type, const Eigen::MatrixXi& F, const int fi, const float distance);
		std::vector<int> adjSetOfTriangles(const Eigen::MatrixXi& F, const std::vector<int> selected, std::vector<std::vector<std::vector<int>>> TT);
		std::vector<int> vectorsIntersection(const std::vector<int>& A, const std::vector<int>& B);
		Eigen::MatrixX4d getValues(const app_utils::Face_Colors face_coloring_Type);
		void initFaceColors(
			const int numF,
			const Eigen::Vector3f center_sphere_color,
			const Eigen::Vector3f center_vertex_color,
			const Eigen::Vector3f centers_sphere_edge_color,
			const Eigen::Vector3f centers_norm_edge_color,
			const Eigen::Vector3f face_norm_color);
		void setFaceColors(const int fi, const Eigen::Vector3d color);
		void shiftFaceColors(const int fi, const double alpha, const Eigen::Vector3f model_color, const Eigen::Vector3f color);
		void initMinimizers(
			const Eigen::MatrixXd& V,
			const Eigen::MatrixXi& F,
			const NumericalOptimizations::InitAuxVar::type& init_aux_var_type,
			const int NeighLevel,
			const double manual_radius_value,
			const Eigen::RowVector3d manual_cylinder_dir,
			const Eigen::RowVector3d helper_vector_dir,
			const Eigen::MatrixXd& manual_A,
			const Eigen::VectorXd& manual_R);
	};
};
