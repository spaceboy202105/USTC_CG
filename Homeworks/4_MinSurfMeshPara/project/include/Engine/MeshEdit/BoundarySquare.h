#pragma once
#include<Engine/MeshEdit/BoundaryBase.h>
namespace Ubpa {
	class BoundarySquare :
		public BoundaryBase
	{
	public:
		BoundarySquare();
		~BoundarySquare();
	public:
		void set_boundary(std::vector<int> boundary_index, std::vector<pointf2>& boundary_list);
	};
}