#pragma once
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Basic/HeapObj.h>

namespace Ubpa {
	class TriMesh;

	class BoundaryBase
	{
	public:
		BoundaryBase() {};
		virtual ~BoundaryBase() {};

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

	public:
		virtual void set_boundary(std::vector<int> boundary_index, std::vector<pointf2>& boundary_list) {};
	};
}