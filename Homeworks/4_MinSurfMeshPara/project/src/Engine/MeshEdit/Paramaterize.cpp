#include <Engine/MeshEdit/Paramaterize.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

using namespace Ubpa;

using namespace std;

Paramaterize::Paramaterize(Ptr<TriMesh> triMesh) 
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

void Paramaterize::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool Paramaterize::Init(Ptr<TriMesh> triMesh) {
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

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
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

bool Paramaterize::Run() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Paramaterization();

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


void Paramaterize::Paramaterization()
{
	init_boundary();
	set_boundary();
	builder_selector();
	Solve();
}

double Paramaterize::Distance(V* v1, V* v2)
{
	
	return sqrt((v1->pos.at(0) - v2->pos.at(0)) * (v1->pos.at(0) - v2->pos.at(0)) + (v1->pos.at(1) - v2->pos.at(1)) * (v1->pos.at(1) - v2->pos.at(1)) +
		(v1->pos.at(2) - v2->pos.at(2)) * (v1->pos.at(2) - v2->pos.at(2)));
}

double Paramaterize::Cosine(V* v0, V* v1, V* v2)
{
	double edge1 = Distance(v0, v1);
	double edge2 = Distance(v1, v2);
	double edge3 = Distance(v0, v2);
	return (edge1 * edge1 + edge2 * edge2 - edge3 * edge3) / (2 * edge1 * edge2);
}

double Paramaterize::Cotangent(V* v0, V* v1, V* v2)
{
	double cosine = Cosine(v0, v1, v2);
	return cosine / sqrt(1 - cosine * cosine);
}


void Paramaterize::init_boundary()
{
	size_t nB = heMesh->Boundaries()[0].size();
	for (int i = 0; i < nB; i++)
	{
		boundary_index.push_back(heMesh->Index(heMesh->Boundaries()[0][i]->Origin()));
	}
}

// void Paramaterize::set_boundary_square()
// {
// 	boundary_type = kSquare;
// }
//
// void Paramaterize::Set_Boundary_Circle()
// {
// 	boundary_type = kCircle;
// }

void Paramaterize::set_boundary()
{
	if (boundary_type == kSquare)
	{
		boundary_base_ptr = make_shared<BoundarySquare>();
	}
	else if (boundary_type == kCircle)
	{
		boundary_base_ptr = make_shared<BoundaryCircle>();
	}
	boundary_base_ptr->set_boundary(boundary_index, boundary_list);
}

// void Paramaterize::Set_Uniform_Method()
// {
// 	barycentric_type = kUniform;
// }
//
// void Paramaterize::Set_Cotangent_Method()
// {
// 	barycentric_type = kCotangent;
// }

void Paramaterize::set_boundary(BoundaryType boundary)
{
	boundary_type = boundary;
}

void Paramaterize::set_method(BarycentricType barycentric)
{
	barycentric_type = barycentric;
}

void Paramaterize::builder_uniform()
{

	size_t v_size = heMesh->NumVertices();
	L_mat.resize(v_size, v_size);
	L_mat.setZero();
	//	generate Laplace Matrix
	vector<Eigen::Triplet<double>> Lij;

	for (size_t i = 0; i < v_size; i++)
	{
		V* v1 = heMesh->Vertices()[i];
		Lij.push_back(Eigen::Triplet<double>(i, i, 1));
		if (!v1->IsBoundary())
		{
			double connect_num = v1->AdjVertices().size();
			for (size_t j = 0; j < connect_num; j++)
			{
				Lij.push_back(Eigen::Triplet<double>(i, heMesh->Index(v1->AdjVertices()[j]), -1 / connect_num));
			}
		}
	}


	L_mat.setFromTriplets(Lij.begin(), Lij.end());

}

void Paramaterize::builder_cotangent()
{
	size_t v_size = heMesh->NumVertices();
	vector<Eigen::Triplet<double>> L_coef;
	L_mat.resize(v_size, v_size);
	L_mat.setZero();
	for (size_t i = 0; i < v_size; i++)
	{
		V* v = heMesh->Vertices()[i];
		L_coef.push_back(Eigen::Triplet<double>(i, i, 1));

		if (!v->IsBoundary())
		{
			int last = 0, next = 0;
			vector<size_t> adj_idx;
			vector<double> weight_list;
			double weight_sum=0;
			int sz = v->AdjVertices().size();
			for (int j = 0; j < sz; j++)
			{

				V* v1 = v->AdjVertices()[(j-1+sz)%sz];
				V* v2 = v->AdjVertices()[(j+sz)%sz];
				V* v3 = v->AdjVertices()[(j+1+sz)%sz];

				weight_list.push_back(Cotangent(v, v1, v2) + Cotangent(v, v3, v2));
				adj_idx.push_back(heMesh->Index(v2));
				weight_sum += weight_list.back();
			}
			for (size_t j = 0; j < adj_idx.size(); j++)
			{
				L_coef.push_back(Eigen::Triplet<double>(i, adj_idx[j], -weight_list[j]/weight_sum));
			}
		}		
	}


	L_mat.setFromTriplets(L_coef.begin(), L_coef.end());
}

void Paramaterize::builder_selector()
{
	v_size = heMesh->NumVertices();
	if (barycentric_type == kCotangent)
		builder_cotangent();
	else if (barycentric_type == kUniform)
		builder_uniform();
}


void Paramaterize::Solve()
{
	Eigen::VectorXd bx(v_size), by(v_size);
	Eigen::VectorXd vec_x(v_size), vec_y(v_size);

	bx.setZero(); by.setZero();
	
	for (int i = 0; i < boundary_index.size(); i++)
	{
		V* v = heMesh->Vertices()[boundary_index[i]];
		bx[boundary_index[i]] = boundary_list[i][0];
		by[boundary_index[i]] = boundary_list[i][1];
	}
	solver.compute(L_mat);
	if (solver.info() != Eigen::Success)
	{
		throw std::exception("Compute Matrix Is Error!");
		return;
	}
	vec_x = solver.solve(bx);
	vec_y = solver.solve(by);

	for (int i = 0; i < v_size; i++)
	{
		// if (display_status == kon)
		// {
			heMesh->Vertices()[i]->pos.at(0) = vec_x(i);
			heMesh->Vertices()[i]->pos.at(1) = vec_y(i);
			heMesh->Vertices()[i]->pos.at(2) = 0;
		// }
		texcoords.push_back(pointf2(vec_x(i), vec_y(i)));
	}
}

std::vector<pointf2>Paramaterize::Get_texcoord()
{
	return texcoords;
}