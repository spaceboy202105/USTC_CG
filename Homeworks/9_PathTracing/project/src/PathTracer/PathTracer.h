#pragma once

#include <UScene/UScene.h>

#include <vector>

namespace Ubpa {
	class PathTracer {
	public:
		void init_probs();
		void init_alias_table();
		PathTracer(const Scene* scene, const SObj* cam_obj, Image* img, size_t spp);
		float PDF_based_alias(const vecf3& dir, const EnvLight* env_light) const;
		std::tuple<rgbf, vecf3, float> sample_based_alias(const normalf& n, const EnvLight* env_light)const;
		void Run();

	private:
		rgbf Shade(const IntersectorClosest::Rst& intersection, const vecf3& wo, bool last_bounce_specular = false) const;
		std::vector<int> alias_table;
		std::vector<float> prob_table;
		std::vector<float> probs;
		struct SampleLightResult {
			rgbf L{ 0.f }; // light radiance
			float pd{ 0.f }; // probability density
			normalf n{ 0.f }; // normalize normal
			pointf3 x{ 0.f }; // position on light
			bool is_infinity{ false }; // infinity distance
		};
		SampleLightResult SampleLight(const IntersectorClosest::Rst& intersection, const vecf3& wo, const Cmpt::Light* light, const Cmpt::L2W* l2w, const Cmpt::SObjPtr* ptr) const;

		// wi (normalized), pd (probability density)
		static std::tuple<vecf3, float> SampleBRDF(const IntersectorClosest::Rst& intersection, const vecf3& wo);
		static rgbf BRDF(IntersectorClosest::Rst intersection, const vecf3& wi, const vecf3& wo);

		const Scene* const scene;
		const EnvLight* env_light{ nullptr };
		Image* const img;

		BVH bvh;

		const size_t spp;
		const Cmpt::Camera* const cam;
		const Cmpt::Camera::CoordinateSystem ccs;
	};
}