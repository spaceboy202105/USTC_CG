#include <Engine/MeshEdit/BoundaryCircle.h>
using namespace Ubpa;
BoundaryCircle::BoundaryCircle()
{
}

BoundaryCircle::~BoundaryCircle()
{
}

void BoundaryCircle::set_boundary(std::vector<int> boundary_index, std::vector<pointf2>& boundary_list)
{
	size_t nB = boundary_index.size();
	double theta = 2 * PI<double> / (double)nB;
	for (int i = 0; i < nB; i++)
	{
		boundary_list.push_back(pointf2(0.5 * cos(i * theta) + 0.5, 0.5 * sin(i * theta) + 0.5));
	}
}
