#include <Engine/MeshEdit/ParameterizeASAP.h>

using namespace Ubpa;
using namespace std;

ParameterizeASAP::ParameterizeASAP(Ptr<TriMesh> triMesh)
	:ParamaterizeBase::ParamaterizeBase(triMesh)
{
}

ParameterizeASAP::~ParameterizeASAP()
{
	Clear();
}

void ParameterizeASAP::Parameterization()
{
	// vector<size_t> vec_num;
	// vec_num.push_back(nV - 1);
	// set_fixed_num(vec_num);
	param_init();
	Local_Phase();
	L_mat_init();
	for (size_t t = 0; t < iteration_times; t++)
	{
		R_mat_init();
		solve();
		update();
	}
}

void ParameterizeASAP::Local_Phase()//get Lt
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
		Eigen::MatrixXd U = svd.matrixU();
		Eigen::MatrixXd V = svd.matrixV();
		double sigma = (svd.singularValues()(0) + svd.singularValues()(1)) / 2;
		Lt = sigma * U * V.transpose();
		L_mat_list.push_back(Lt);
	}
}