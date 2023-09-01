#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Eigen/Sparse>
#include <Engine/MeshEdit/BoundaryCircle.h>
#include <Engine/MeshEdit/BoundaryBase.h>
#include <Engine/MeshEdit/BoundarySquare.h>

namespace Ubpa {
	class TriMesh;
	class MinSurf;

	class Paramaterize : public HeapObj {
	public:
		Paramaterize(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<Paramaterize> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<Paramaterize>(triMesh);
		}
	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);
		// kernel part of the algorithm
		void Paramaterization();
		bool Run();

		std::vector<pointf2>Get_texcoord();

	public:
		enum BoundaryType
		{
			kCircle,
			kSquare
		}boundary_type;

		enum BarycentricType
		{
			kUniform,
			kCotangent
		}barycentric_type;


		void set_method(BarycentricType barycentric);
		void set_boundary(BoundaryType boundary);

	private:
		
		void init_boundary();
		void set_boundary();
		void builder_uniform();
		void builder_cotangent();
		void builder_selector();


		void Solve();

	private:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };

	private:
		double Distance(V* v1, V* v2);
		double Cosine(V* v0, V* v1, V* v2);
		double Cotangent(V* v0, V* v1, V* v2);

	private:
		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		Ptr<BoundaryBase> boundary_base_ptr;
		Eigen::SparseMatrix<double> L_mat;
		Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
		std::vector<int> boundary_index;
		std::vector<pointf2> boundary_list;
		std::vector<pointf2>texcoords;
		int v_size ;

	};
}
