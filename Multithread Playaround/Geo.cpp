#include "Geo.h"

Geo::geoModel Geo::genIcoSphere(float size, vec3 centre, uint order) {
	auto it = icoCache.find(order);
	geoModel* geo;
	if (it == icoCache.end()) {
		std::vector<vec3> baseVert{ std::begin(ICOSAHEDRONVERT), std::end(ICOSAHEDRONVERT) };
		std::vector<uint> baseInd{ std::begin(ICOSAHEDRONIND), std::end(ICOSAHEDRONIND) };
		for (uint i = 1; i <= order; i++) {
			std::vector<uint> newInd{};
			newInd.reserve(baseInd.size() * 4);
			baseVert.reserve(10 * pow(4, i) + 2);
			std::unordered_map<uint, uint> midPoint{};

			for (uint j = 0; j < baseInd.size(); j += 3) {
				const uint& p0 = baseInd[j];
				const uint& p1 = baseInd[j + 1];
				const uint& p2 = baseInd[j + 2];

				const auto getMidPoint = [&midPoint, &baseVert](const uint a, const uint b) {
					// Cantor's pairing function
					const uint& key = ((a + b) * (a + b + 1) / 2) + std::min(a, b);
					auto it = midPoint.find(key);
					if (it != midPoint.end()) {
						return it->second;
					}
					uint i = baseVert.size();
					midPoint.insert_or_assign(key, i);
					baseVert.push_back((baseVert[a] + baseVert[b]) / 2.f);
					return i;
				};
				//    0
				// m01 m20
				//1  m12  2
				const uint m01 = getMidPoint(p0, p1);
				const uint m12 = getMidPoint(p1, p2);
				const uint m20 = getMidPoint(p2, p0);
				newInd.insert(newInd.end(), {
					p0, m01, m20,
					p1, m12, m01,
					p2, m20, m12,
					m01,m12, m20 }
				);
			}
			baseInd = std::move(newInd);
		}
		for (auto& point : baseVert) {
			point = glm::normalize(point);
		}
		geo = new std::tuple(baseVert, baseInd);
		icoCache.insert_or_assign(order, *geo);
	}
	else {
		geo = &it->second;
	}

	if (size == 1.f && centre == vec3(0.f)) {
		return std::tuple(std::vector<vec3>{std::get<0>(*geo)},
			std::vector<uint>{std::get<1>(*geo)});
	}
	else {
		std::vector<vec3> verts{};
		verts.reserve(std::get<0>(*geo).size());
		for (uint i = 0; i < std::get<0>(*geo).size(); i++) {
			verts[i] = std::get<0>(*geo)[i] * size + centre;
		}
		return std::tuple(verts, std::vector<uint>{std::get<1>(*geo)});
	}
}