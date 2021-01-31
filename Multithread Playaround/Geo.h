#include <tuple>
#include <vector>
#include "Definer.h"

//Geo

namespace Geo {
	using geoModel = std::tuple<std::vector<vec3>, std::vector<uint>>;

	static const float icoF = (1 + pow(5, 0.5)) / 2;
	static std::unordered_map<uint, geoModel> icoCache;

	static const vec3 ICOSAHEDRONVERT[] = {
		vec3(-1, icoF, 0), vec3(1, icoF, 0), vec3(-1, -icoF, 0), vec3(1, -icoF, 0),
		vec3(0, -1, icoF), vec3(0, 1, icoF), vec3(0, -1, -icoF), vec3(0, 1, -icoF),
		vec3(icoF, 0, -1), vec3(icoF, 0, 1), vec3(-icoF, 0, -1), vec3(-icoF, 0, 1)
	};
	static const uint ICOSAHEDRONIND[] = {
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		11, 10, 2, 5, 11, 4, 1, 5, 9, 7, 1, 8, 10, 7, 6,
		3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		9, 8, 1, 4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7
	};

	geoModel genIcoSphere(float size, vec3 centre, uint order);
}