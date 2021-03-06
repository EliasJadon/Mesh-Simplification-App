#pragma once
#include "ObjectiveFunctions/Total.h"
#include <atomic>
#include <shared_mutex>
#include <igl/flip_avoiding_line_search.h>
#include <Eigen/SparseCholesky>
#include <fstream>

namespace NumericalOptimizations {
	class Basic
	{
	public:
		Cuda::Array<double> X, p, curr_x, v_adam, s_adam;
		Basic(
			const int solverID,
			const Eigen::MatrixXd& V,
			const Eigen::MatrixXi& F,
			const OptimizationUtils::LineSearch linesearch_type,
			const Cuda::OptimizerType optimizer_type);
		void run();
		void run_new();
		void RunSymmetricDirichletGradient();
		void run_one_iteration();
		void run_one_iteration_new();
		void external_stop();
		bool external_is_running();
		void external_get_data(
			Eigen::MatrixXd& X,
			Eigen::MatrixXd& center,
			Eigen::VectorXd& radius,
			Eigen::MatrixXd& norm,
			Eigen::MatrixXd& cylinder_dir);
		void init(
			const Eigen::MatrixXd& V0,
			const Eigen::MatrixXd& norm0,
			const Eigen::MatrixXd& center0,
			const Eigen::MatrixXd& radius0,
			const Eigen::MatrixXd& cylinder_dir0);
		void upload_x(const Eigen::VectorXd& X0);

		std::shared_ptr<ObjectiveFunctions::Total> totalObjective;
		
		std::atomic_bool isGradientNeeded = { false };

		Cuda::indices mesh_indices;
		Cuda::OptimizerType Optimizer_type;
		double timer_curr = 0, timer_sum = 0, timer_avg = 0;

		OptimizationUtils::LineSearch lineSearch_type;
		double constantStep_LineSearch;
		inline int getNumiter() {
			return this->numIteration;
		}
		void update_lambda();
		bool isAutoLambdaRunning = true;
		bool isUpdateLambdaWhenConverge = false;
		int autoLambda_from = 100, autoLambda_count = 40, autoLambda_jump = 70;
		double init_step_size = 1;
		unsigned int linesearch_numiterations = 0;
		unsigned int linesearch_StopCounter = 0;
	private:
		double currentEnergy;
		unsigned int numIteration = 0;
		int solverID;
		void linesearch();
		void value_linesearch();
		void gradNorm_linesearch();
		void constant_linesearch();
		double step_size;

		std::atomic_bool halt = { false };
		std::atomic_bool is_running = { false };
	};
};