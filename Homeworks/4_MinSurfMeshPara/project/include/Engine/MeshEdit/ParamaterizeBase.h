#pragma once
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Basic/HeapObj.h>
#include<Eigen/Sparse>
#include<Eigen/Dense>
#include <Engine/MeshEdit//Paramaterize.h>

namespace Ubpa {
	class TriMesh;
	class MinSurf;

	typedef std::map<size_t, int, std::greater<>> index_map;
	typedef std::map<size_t, double, std::greater<>> coef_map;
	//typedef std::map<size_t, pointf2, std::greater<>> pt_map;

	// mesh boundary == 1
	class ParamaterizeBase
	{
	public:
		ParamaterizeBase(Ptr<TriMesh> triMesh);
		void set_iteration(float t);
		virtual ~ParamaterizeBase();

	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);
		bool Run();

	protected:

		void param_init();
		void L_mat_init();
		void R_mat_init();
		void solve();
		size_t get_opposite_index(size_t i, size_t j);
		void update();
		//void set_fixed_num(std::vector<size_t> t);


	protected:
		virtual void  Parameterization()=0;
		virtual void Local_Phase() {};
		
	public:
		enum ParaMethod
		{
			kASAP,
			kARAP
		};

	protected:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };

	protected:
		Ptr<TriMesh> triMesh;
		Ptr<HEMesh<V>> heMesh;
		std::vector<pointf2>texcoords;

		size_t nP, nV;
		std::vector<std::vector<pointf2>> vertices_list;//the points after 3d to 2d
		std::vector<index_map> vertices_index_triangle;
		std::vector<std::vector<size_t>> vertices_index_hemesh;
		std::vector<std::vector<double>> cotangent_list;
		std::vector<std::vector<double>> delta_x_list;
		std::vector<std::vector<double>> delta_y_list;
		Eigen::SparseMatrix<double> L_mat;
		int iteration_times;
		std::vector<size_t> fixed_num;

	protected:
		std::vector<pointf2> u_list;
		std::vector<std::vector<pointf2>> delta_u_list;
		std::vector<Eigen::Matrix2d> L_mat_list;
		Eigen::MatrixXd R_mat;
		Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
		Eigen::MatrixXd U;

	public:
		Ptr<Paramaterize>paramaterize;
	};
}