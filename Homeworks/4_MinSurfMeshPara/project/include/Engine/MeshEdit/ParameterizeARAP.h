#pragma once
#include <Engine/MeshEdit/ParamaterizeBase.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace Ubpa
{
	class TriMesh;
	
	class ParameterizeARAP :
		public ParamaterizeBase
	{
	public:
		ParameterizeARAP(Ptr<TriMesh> triMesh);
		~ParameterizeARAP();

	private:
		void Parameterization();
		void Local_Phase();



	};


}