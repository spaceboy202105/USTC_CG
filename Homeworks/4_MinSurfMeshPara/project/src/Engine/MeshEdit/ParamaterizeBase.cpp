# include <Engine/MeshEdit/ParamaterizeBase.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Engine/MeshEdit/Paramaterize.h>

using namespace Ubpa;
using namespace std;

ParamaterizeBase::ParamaterizeBase(Ptr<TriMesh> triMesh)
{
	heMesh = make_shared<HEMesh<V>>();
	fixed_num.push_back(0);
	iteration_times = 1;
	Init(triMesh);
}

ParamaterizeBase::~ParamaterizeBase()
{
}

void ParamaterizeBase::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool ParamaterizeBase::Init(Ptr<TriMesh> triMesh)
{
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::Paramaterize::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	// init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {//cannot use our methods
		printf("ERROR::Paramaterize::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}

	this->triMesh = triMesh;
	return true;
}

void ParamaterizeBase::set_iteration(float t)
{
	iteration_times = (size_t)t;
}

bool ParamaterizeBase::Run() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Parameterization();

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());
	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);
	triMesh->Update(texcoords);
	return true;
}

void ParamaterizeBase::param_init()
{
	nP = heMesh->NumPolygons();
	nV = heMesh->NumVertices();
	for (size_t t = 0; t < nP; t++)//for each triangle
	{
		P* polygon = heMesh->Polygons()[t];
		vector<pointf2> p_list;
		vector<double> cot_list;
		vector<double> delta_x;
		vector<double> delta_y;
		double cosine[3] = { 0 };
		//x_vec
		vecf3 vec0 = polygon->BoundaryVertice()[0]->pos;
		vecf3 vec1 = polygon->BoundaryVertice()[1]->pos;
		vecf3 vec2 = polygon->BoundaryVertice()[2]->pos;
		//cout << vec0 << " " << vec1 << " " << vec2 << endl;
		index_map temp_map;
		vector<size_t> temp_vec;
		for (size_t i = 0; i < 3; i++)
		{
			size_t index = heMesh->Index(polygon->BoundaryVertice()[i]);
			temp_map.insert(index_map::value_type(index, i));
			temp_vec.push_back(index);
		}
		//build mapper index 2 triangle and hemesh
		vertices_index_triangle.push_back(temp_map);
		vertices_index_hemesh.push_back(temp_vec);
		//edge vector to get cosine
		cosine[0] = vecf3::cos_theta(vec1 - vec0, vec2 - vec0);
		cosine[1] = vecf3::cos_theta(vec0 - vec1, vec2 - vec1);
		cosine[2] = vecf3::cos_theta(vec0 - vec2, vec1 - vec2);

		//3D point to 2D point
		p_list.push_back(pointf2(0, 0));
		p_list.push_back(pointf2((vec1 - vec0).norm(), 0));//
		p_list.push_back(pointf2((vec2 - vec0).norm() * cosine[0], (vec2 - vec0).norm() * sqrt(1 - cosine[0] * cosine[0])));

		for (int i = 0; i < 3; i++)
		{
			delta_x.push_back((double)p_list[i % 3][0] - (double)p_list[(i + 1) % 3][0]);
			delta_y.push_back((double)p_list[i % 3][1] - (double)p_list[(i + 1) % 3][1]);
			cot_list.push_back(cosine[i] / sqrt(1 - cosine[i] * cosine[i]));
			//if (t == 0)
			//{
			//	fixed_pt.insert(pt_map::value_type(heMesh->Index(polygon->BoundaryVertice()[i]), p_list[i]));
			//}
		}
		delta_x_list.push_back(delta_x);
		delta_y_list.push_back(delta_y);
		vertices_list.push_back(p_list);
		cotangent_list.push_back(cot_list);
	}
	paramaterize->Paramaterization();//tutte paramaterization
	u_list = paramaterize->Get_texcoord();//init uv

	for (size_t t = 0; t < nP; t++)
	{
		vector<pointf2> delta_u;
		for (size_t i = 0; i < 3; i++)
		{
			pointf2 p1, p2;
			p1 = u_list[vertices_index_hemesh[t][i]];
			p2 = u_list[vertices_index_hemesh[t][(i + 1) % 3]];
			delta_u.push_back(pointf2(p1[0] - p2[0], p1[1] - p2[1]));
		}
		delta_u_list.push_back(delta_u);
	}
}

void ParamaterizeBase::R_mat_init()
{
	Eigen::Vector2d x;
	Eigen::Vector2d temp_vec;
	R_mat.resize(nV, 2);
	R_mat.setZero();

	for (size_t i = 0; i < nV; i++)
	{
		x.setZero();
		temp_vec.setZero();
		V* vi = heMesh->Vertices()[i];
		auto edge_ = vi->HalfEdge();
		do
		{
			V* vj = edge_->End();
			size_t tij, tji;
			double cotij = 0, cotji = 0;
			size_t j = heMesh->Index(vj);

			P* pij = edge_->Polygon();
			P* pji = edge_->Pair()->Polygon();

			if (pij != nullptr)
			{
				tij = heMesh->Index(pij);
				cotij = cotangent_list[tij][get_opposite_index(vertices_index_triangle[tij][i], vertices_index_triangle[tij][j])];

				pointf2 xi = vertices_list[tij][vertices_index_triangle[tij][i]];
				pointf2 xj = vertices_list[tij][vertices_index_triangle[tij][j]];
				x << xi[0] - xj[0], xi[1] - xj[1];
				temp_vec += cotij * L_mat_list[tij] * x;
			}
			if (pji != nullptr)
			{
				tji = heMesh->Index(pji);
				cotji = cotangent_list[tji][get_opposite_index(vertices_index_triangle[tji][i], vertices_index_triangle[tji][j])];
				pointf2 xi = vertices_list[tji][vertices_index_triangle[tji][i]];
				pointf2 xj = vertices_list[tji][vertices_index_triangle[tji][j]];
				x << xi[0] - xj[0], xi[1] - xj[1];
				temp_vec += cotji * L_mat_list[tji] * x;
			}
			R_mat(i, 0) = temp_vec(0);
			R_mat(i, 1) = temp_vec(1);
			edge_ = edge_->Pair()->Next();
		} while (edge_ != vi->HalfEdge());
	}

	for (size_t i = 0; i < fixed_num.size(); i++)
	{
		R_mat(fixed_num[i], 0) = u_list[fixed_num[i]][0];
		R_mat(fixed_num[i], 1) = u_list[fixed_num[i]][1];
	}
}

void ParamaterizeBase::solve()
{
	U.resize(nV, 2);
	U.setZero();

	U = solver.solve(R_mat);//LU=R
}

void ParamaterizeBase::L_mat_init()
{
	/*
	 * [cot(ij)+cot(ji)]*(ui-uj) can build a laplace matrix
	 *
	 */
	vector<Eigen::Triplet<double>> L_coef;
	L_mat.resize(nV, nV);

	for (size_t i = 0; i < nV; i++)
	{
		double u_temp = 0;
		V* vi = heMesh->Vertices()[i];
		auto edges = vi->HalfEdge();

		do//for each
		{
			V* vj = edges->End();//get vert
			size_t tij, tji;
			double cotij = 0, cotji = 0;
			size_t j = heMesh->Index(vj);

			P* pij = edges->Polygon();//get the triangle which the edge belong to (in half mesh vision)
			P* pji = edges->Pair()->Polygon();

			if (pij != nullptr)
			{
				tij = heMesh->Index(pij);//i,j oppsite edge => get the
				cotij = cotangent_list[tij][get_opposite_index(vertices_index_triangle[tij][i], vertices_index_triangle[tij][j])];
			}
			if (pji != nullptr)
			{
				tji = heMesh->Index(pji);
				cotji = cotangent_list[tji][get_opposite_index(vertices_index_triangle[tji][i], vertices_index_triangle[tji][j])];
			}
			//
			u_temp += cotij + cotji;
			L_coef.push_back(Eigen::Triplet<double>(i, j, -cotij - cotji));
			edges = edges->Pair()->Next();
		} while (edges != vi->HalfEdge());
		bool flag = true;
		for (size_t k : fixed_num)
		{
			if (k == i)
			{
				flag = false;
				break;
			}
		}
		if (flag)
			L_coef.push_back(Eigen::Triplet<double>(i, i, u_temp));
	}

	L_mat.setFromTriplets(L_coef.begin(), L_coef.end());

	for (size_t i = 0; i < fixed_num.size(); i++)
		L_mat.insert(fixed_num[i], fixed_num[i]) = 1.0;

	L_mat.makeCompressed();
	solver.compute(L_mat);
}

size_t ParamaterizeBase::get_opposite_index(size_t i, size_t j)//for point index get the
{
	return 3 - i - j;
}

void ParamaterizeBase::update()
{
	texcoords.clear();

	double Umax = -INFINITY, Umin = INFINITY, Vmax = -INFINITY, Vmin = INFINITY;

	for (size_t i = 0; i < nV; i++)
	{
		Umax = max(Umax, U(i, 0));
		Umin = min(Umin, U(i, 0));
		Vmax = max(Vmax, U(i, 1));
		Vmin = min(Vmin, U(i, 1));
	}

	for (size_t i = 0; i < nV; i++)
	{
		texcoords.push_back(pointf2((U(i, 0) - Umin) / (Umax - Umin), (U(i, 1) - Vmin) / (Vmax - Vmin)));

		// if (display_status)
		// {
			//normalize the
		heMesh->Vertices()[i]->pos.at(0) = (U(i, 0) - Umin) / (Umax - Umin);
		heMesh->Vertices()[i]->pos.at(1) = (U(i, 1) - Vmin) / (Vmax - Vmin);//normalize
		heMesh->Vertices()[i]->pos.at(2) = 0;//3d to 2d
	// }
	}
	//update u
	u_list.clear();
	u_list = texcoords;
	for (size_t t = 0; t < nP; t++)
	{
		vector<pointf2> delta_u;
		for (size_t i = 0; i < 3; i++)
		{
			pointf2 p1, p2;
			p1 = u_list[vertices_index_hemesh[t][i]];
			p2 = u_list[vertices_index_hemesh[t][(i + 1) % 3]];
			delta_u.push_back(pointf2(p1[0] - p2[0], p1[1] - p2[1]));
		}
		delta_u_list.push_back(delta_u);
	}
}

//void ParamaterizeBase::set_fixed_num(vector<size_t> t)
//{
//	for (size_t i = 0; i < t.size(); i++)
//	{
//		if (t[i] != 0)
//			t.push_back(i);
//	}
//}