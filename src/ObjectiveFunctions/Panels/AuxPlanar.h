#pragma once
#include "ObjectiveFunctions/Panels/AuxBasic.h"

namespace ObjectiveFunctions {
	namespace Panels {
		class AuxPlanar : public AuxBasic
		{
		public:
			double w1 = 1, w2 = 100, w3 = 100;
			AuxPlanar(const Eigen::MatrixXd& V, const Eigen::MatrixX3i& F, const Cuda::PenaltyFunction type);
			~AuxPlanar();
			virtual double value(Cuda::Array<double>& curr_x, const bool update) override;
			virtual void gradient(Cuda::Array<double>& X, const bool update) override;
		};
	};
};

