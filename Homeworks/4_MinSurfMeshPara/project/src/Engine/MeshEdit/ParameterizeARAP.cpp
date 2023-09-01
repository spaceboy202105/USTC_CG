#include <Engine/MeshEdit/ParameterizeARAP.h>

using namespace Ubpa;
using namespace std;

ParameterizeARAP::ParameterizeARAP(Ptr<TriMesh> triMesh)
	:ParamaterizeBase::ParamaterizeBase(triMesh)
{
}

ParameterizeARAP::~ParameterizeARAP()
{
	Clear();
}

void ParameterizeARAP::Parameterization()
{
	param_init();
	Local_Phase();
	L_mat_init();
	for (size_t t = 0; t < iteration_times; t++)
	{
		//solve: LU=R
		R_mat_init();
		solve();
		update();

		cout << t + 1 << " iteration complete" << endl;
	}
}

void ParameterizeARAP::Local_Phase()//get Lt
{
	Eigen::Matrix2d Lt, Jt;

	for (size_t t = 0; t < nP; t++)
	{
		Jt.setZero();
		for (int i = 0; i < 3; i++)
		{
			Eigen::Vector2d u_vec, x_vec;
			x_vec << delta_x_list[t][i], delta_y_list[t][i];
			u_vec << delta_u_list[t][i][0], delta_u_list[t][i][1];
			Jt += cotangent_list[t][i] * u_vec * x_vec.transpose();
		}
		Eigen::JacobiSVD<Eigen::MatrixXd> svd(Jt, Eigen::ComputeThinU | Eigen::ComputeThinV);
		Lt = svd.matrixU() * svd.matrixV().transpose();
		L_mat_list.push_back(Lt);
	}
}