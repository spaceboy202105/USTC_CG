#pragma once
#include <Engine/MeshEdit/ParamaterizeBase.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

// #include "ParamaterizeBase.h"
#include "Basic/Ptr.h"

namespace Ubpa
{
	class TriMesh;

	class ParameterizeASAP :
		public ParamaterizeBase
	{
	public:
		ParameterizeASAP(Ptr<TriMesh> triMesh);
		~ParameterizeASAP();

	private:
		void Parameterization();
		void Local_Phase();

	};


}
