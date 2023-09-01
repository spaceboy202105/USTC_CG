#include <Engine/MeshEdit/Simulate.h>

#include <Eigen/Sparse>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

void Simulate::Clear() {
	this->positions.clear();
	this->velocity.clear();
}

bool Simulate::Init() {
	// Clear();

	original_length_list.resize(edgelist.size() / 2);
	mass = 1;
	I_mat.setZero();
	I_mat.setIdentity();
	//	Init extra force
	fext.resize(positions.size());

	this->velocity.resize(positions.size());
	for (int i = 0; i < positions.size(); i++)
	{
		fext[i] = vecf3(0, -gravity, 0);
		for (int j = 0; j < 3; j++)
		{
			this->velocity[i][j] = 0;
		}
	}

	for (size_t i = 0; i < edgelist.size() / 2; i++)
	{
		pointf3 p1 = positions[edgelist[i * 2]];
		pointf3 p2 = positions[edgelist[i * 2 + 1]];
		original_length_list[i] = pointf3::distance(p1, p2);
	}
	//init_mat_optimize();
	//cout << "original_length_list:" << endl << original_length_list[0] << endl << original_length_list[1] << endl << original_length_list[2] << endl<<endl;
	init_mat_optimize();
	return true;
}

bool Simulate::Run() {
	//cout << "positions:" << endl << positions[0] << endl << positions[1] << endl << positions[2] << endl << endl;
	//cout << "velocity:" << endl << velocity[0] << endl << velocity[1] << endl << velocity[2] << endl << endl;
	SimulateOnce();

	//cout << "------------------------------------------------------------------------" << endl << endl;
	// half-edge structure -> triangle mesh

	return true;
}

void Ubpa::Simulate::ClearFix()
{
	fixed_id.clear();
	fixed_pos.clear();
	Init();
}

void Ubpa::Simulate::AddFix()
{
	for (int i = 0; i < fixed_id.size(); i++)
	{
		if (fixed_id[i] == select_index)
		{
			return;
		}
	}
	fixed_id.push_back(select_index);
	fixed_pos.push_back(positions[select_index]);
	Init();
}

void Ubpa::Simulate::SetLeftFix()
{
	//固定网格x坐标最小点
	fixed_id.clear();
	fixed_pos.clear();
	double x = 100000;
	for (int i = 0; i < positions.size(); i++)
	{
		x = std::min(x, (double)positions[i][0]);
	}

	for (int i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][0] - x) < 1e-5)
		{
			fixed_id.push_back(i);
			fixed_pos.push_back(positions[i]);
		}
	}

	Init();
}

void Simulate::SetRightFix()
{
	//固定网格x坐标最大点
	fixed_id.clear();
	fixed_pos.clear();
	double x = -100000;
	for (int i = 0; i < positions.size(); i++)
	{
		x = std::max(x, (double)positions[i][0]);
	}

	for (int i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][0] - x) < 1e-5)
		{
			fixed_id.push_back(i);
			fixed_pos.push_back(positions[i]);
		}
	}

	Init();
}

void Ubpa::Simulate::SetUpFix()
{
	//固定网格y坐标最大点
	fixed_id.clear();
	fixed_pos.clear();
	double y = -100000;
	for (int i = 0; i < positions.size(); i++)
	{
		y = std::max(y, (double)positions[i][1]);
	}

	for (int i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][1] - y) < 1e-5)
		{
			fixed_id.push_back(i);
			fixed_pos.push_back(positions[i]);
			cout << i << " " << positions[i] << endl;
		}
	}

	Init();
}

void Ubpa::Simulate::SetDownFix()
{
	//固定网格y坐标最小点
	fixed_id.clear();
	fixed_pos.clear();
	double y = 100000;
	for (int i = 0; i < positions.size(); i++)
	{
		y = std::min(y, (double)positions[i][1]);
	}

	for (int i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][1] - y) < 1e-5)
		{
			fixed_id.push_back(i);
			fixed_pos.push_back(positions[i]);
		}
	}

	Init();
}

void Simulate::SimulateOnce()
{
	// init_iteration();
	Init_b_optim();
	x_eps = 10000;
	// size_t count_ = 0;
	while (abs(x_eps) > 1e-3)
	{
		// count_++;
		// iteration_once();
		iteration_once_optimize();
	}

	velocity_update();
}

void Simulate::step_update()
{
	vector<Matrix3f> gradiant_f_int;
	vector<Vector3f> g_list;
	std::vector<vecf3> fint_list;//internal force list

	gradiant_f_int.resize(positions.size());
	g_list.resize(positions.size());
	fint_list.resize(positions.size());

	for (int i = 0; i < positions.size(); i++)
	{
		gradiant_f_int[i].setZero();
		g_list[i].setZero();
		fint_list[i] = vecf3(0, 0, 0);
	}

	for (size_t i = 0; i < edgelist.size() / 2; i++)//for each spring
	{
		size_t index_i = edgelist[i * 2];
		size_t index_j = edgelist[i * 2 + 1];

		pointf3 pi = positions[index_i];
		pointf3 pj = positions[index_j];

		vecf3 rij = pi - pj,
			rji = pj - pi;//vector to describe spring

		float L0 = original_length_list[i];

		vecf3 f_int = stiff * (rij.norm() - L0) / rij.norm() * rji;
		fint_list[index_i] += f_int;//one plus
		fint_list[index_j] -= f_int;//another minus

		Vector3f r_vec;

		r_vec << rij[0], rij[1], rij[2];
		//
		I_mat.resize(3, 3);
		I_mat << 1, 0, 0,
			0, 1, 0,
			0, 0, 1;
		Matrix3f f1_x1_part_mat = stiff * (L0 / rij.norm() - 1) * I_mat - stiff * L0 / pow(rij.norm(), 3) * r_vec * r_vec.transpose();
		gradiant_f_int[index_i] += f1_x1_part_mat;
		gradiant_f_int[index_j] -= f1_x1_part_mat;
	}

	step_list.clear();
	for (int i = 0; i < positions.size(); i++)
	{
		pointf3 x = positions[i];
		Matrix3f gradiant_g;
		Vector3f g;
		Vector3f step;
		vecf3 g_vec;

		g_vec = mass * (x - y_list[i]) - h * h * fint_list[i];
		g << g_vec[0], g_vec[1], g_vec[2];

		gradiant_g.setZero();
		//gradiant_g(x)=g`(x)
		gradiant_g = mass * I_mat - h * h * gradiant_f_int[i];

		step = gradiant_g.inverse() * g;

		step_list.push_back(step);
	}
}

void Simulate::init_y()
{
	y_list.resize(positions.size());
	for (size_t i = 0; i < y_list.size(); i++)
	{
		vecf3 v(velocity[i][0], velocity[i][1], velocity[i][2]);
		y_list[i] = positions[i] + h * v + h * h * fext[i] / mass;
		velocity[i] = pointf3(positions[i][0], positions[i][1], positions[i][2]);
	}
	for (size_t i = 0; i < fixed_id.size(); i++)//fixed point never change
	{
		y_list[fixed_id[i]] = fixed_pos[i];
	}
}

void Simulate::init_iteration()
{
	init_y();

	for (size_t i = 0; i < positions.size(); i++)
	{
		positions[i] = y_list[i];
	}
}

void Simulate::iteration_once()
{
	step_update();
	x_eps = 0;
	for (int i = 0; i < positions.size(); i++)
	{
		Vector3f x_vec;
		x_vec << positions[i][0], positions[i][1], positions[i][2];
		x_vec -= step_list[i];
		x_eps += abs(step_list[i][0]) + abs(step_list[i][1]) + abs(step_list[i][2]);
		positions[i] = pointf3(x_vec[0], x_vec[1], x_vec[2]);
	}
	x_eps /= positions.size();
	for (size_t i = 0; i < fixed_pos.size(); i++)
	{
		positions[fixed_id[i]] = fixed_pos[i];
	}
}

void Simulate::velocity_update()
{
	for (size_t i = 0; i < velocity.size(); i++)
	{
		vecf3 new_pos(positions[i][0], positions[i][1], positions[i][2]);
		vecf3 old_pos(velocity[i][0], velocity[i][1], velocity[i][2]);
		vecf3 v = (new_pos - old_pos) / h;
		velocity[i] = pointf3(v[0], v[1], v[2]);
	}
	for (size_t i = 0; i < fixed_id.size(); i++)
	{
		velocity[fixed_id[i]] = pointf3(0, 0, 0);
	}
}

// Eigen::MatrixXf Simulate::Kronecker_product(Eigen::MatrixXf A, Eigen::MatrixXf B)
// {
// 	Eigen::MatrixXf result(A.rows() * B.rows(), A.cols() * B.cols());
// 	result.setZero();
// 	for (size_t A_row = 0; A_row < A.rows(); A_row++)
// 	{
// 		for (size_t A_col = 0; A_col < A.cols(); A_col++)
// 		{
// 			for (size_t B_row = 0; B_row < B.rows(); B_row++)
// 			{
// 				for (size_t B_col = 0; B_col < B.cols(); B_col++)
// 				{
// 					result(A_row * B.rows() + B_row, A_col * B.cols() + B_col) = A(A_row, A_col) * B(B_row, B_col);
// 				}
// 			}
// 		}
// 	}
// 	return result;
// }

void Simulate::removeRow(Eigen::MatrixXf& matrix, unsigned int rowToRemove)
{
	unsigned int numRows = matrix.rows() - 1;
	unsigned int numCols = matrix.cols();

	if (rowToRemove < numRows)
		matrix.block(rowToRemove, 0, numRows - rowToRemove, numCols) = matrix.block(rowToRemove + 1, 0, numRows - rowToRemove, numCols);

	matrix.conservativeResize(numRows, numCols);
}
/////////////////////////////////////Optimize Method/////////////////////////////////////////////////////

void Simulate::init_mat_optimize()
{
	vector<VectorXf> A_mat_list;
	vector<VectorXf> S_mat_list;
	MatrixXf identity_mat(3, 3);
	identity_mat.setIdentity();

	size_t m = positions.size();
	size_t s = edgelist.size() / 2;

	S_mat_list.resize(s);
	A_mat_list.resize(s);
	L_mat.resize(m, m);
	J_mat.resize(m, s);

	L_mat.setZero();
	J_mat.setZero();

	for (size_t i = 0; i < s; i++)
	{
		A_mat_list[i].resize(m);
		A_mat_list[i].setZero();
		S_mat_list[i].resize(s);
		S_mat_list[i].setZero();

		A_mat_list[i](edgelist[i * 2]) = 1;
		A_mat_list[i](edgelist[i * 2 + 1]) = -1;
		S_mat_list[i](i) = 1;
	}

	for (size_t i = 0; i < s; i++)
	{
		L_mat += A_mat_list[i] * A_mat_list[i].transpose() * stiff;
		J_mat += A_mat_list[i] * S_mat_list[i].transpose() * stiff;
	}

	MatrixXf K_mat(m, m);
	K_mat.setIdentity();

	for (size_t i = 0; i < fixed_id.size(); i++)
	{
		removeRow(K_mat, fixed_id[i] - i);
		//considered that index -1 when you delete 1 row
	}

	identity_mat.resize(L_mat.rows(), L_mat.cols());
	identity_mat.setIdentity();

	A_optim_mat = mass * identity_mat + h * h * L_mat;
	solver.compute((K_mat * A_optim_mat * K_mat.transpose()).sparseView().pruned());
}

void Simulate::Init_b_optim()
{
	init_y();
	y_mat.resize(L_mat.rows(), 3);
	for (size_t i = 0; i < L_mat.rows(); i++)
	{
		y_mat(i, 0) = y_list[i][0];
		y_mat(i, 1) = y_list[i][1];
		y_mat(i, 2) = y_list[i][2];
	}
	b_optim_mat = mass * y_mat;
}

void Simulate::update_d()
{
	d_vec.resize(edgelist.size() / 2, 3);
	d_vec.setZero();

	for (size_t i = 0; i < edgelist.size() / 2; i++)
	{
		pointf3 pi = positions[edgelist[i * 2]];
		pointf3 pj = positions[edgelist[i * 2 + 1]];
		vecf3 d;
		d = original_length_list[i] * (pi - pj) / (pi - pj).norm();
		d_vec(i, 0) = d[0];
		d_vec(i, 1) = d[1];
		d_vec(i, 2) = d[2];
	}
}

void Simulate::update_x()
{
	MatrixXf B;
	MatrixXf K_mat(positions.size(), positions.size());
	K_mat.setIdentity();

	for (size_t i = 0; i < fixed_id.size(); i++)
	{
		removeRow(K_mat, fixed_id[i] - i);
	}

	MatrixXf X(positions.size(), 3);
	for (size_t i = 0; i < positions.size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			X(i, j) = positions[i][j];
		}
	}

	MatrixXf bb = X - K_mat.transpose() * K_mat * X;
	B = K_mat * (h * h * J_mat * d_vec + mass * y_mat - A_optim_mat * bb);
	//1.get xf
	X = solver.solve(B);
	//2.get x
	X = K_mat.transpose() * X + bb;
	x_eps = 0;
	for (size_t i = 0; i < positions.size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			x_eps += abs(X(i, j) - positions[i][j]);
		}
		positions[i] = pointf3(X(i, 0), X(i, 1), X(i, 2));
	}
	x_eps /= positions.size();
}

void Simulate::iteration_once_optimize()
{
	update_d();

	update_x();
}