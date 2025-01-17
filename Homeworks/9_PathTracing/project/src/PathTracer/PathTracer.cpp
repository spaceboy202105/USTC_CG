#include "PathTracer.h"

#include <UBL/Image.h>

#include <iostream>
#include <queue>

#include <thread>

namespace Ubpa
{
	struct EnvLight;

	namespace Cmpt
	{
		class Light;
		class L2W;
		class Camera;
	}

	class Image;
	class SObj;
	class Scene;
}

using namespace Ubpa;
using namespace std;

void PathTracer::init_probs()
{
	int h = env_light->texture->img.get()->height,
		w = env_light->texture->img.get()->width;
	probs.resize(w * h);
	float sum_probs = 0.0;
	for (int i = 0; i < h; ++i)
	{
		for (int j = 0; j < w; ++j)
		{
			probs[j + w * i] = env_light->texture->img.get()->At(j, i).to_rgb().illumination();
			sum_probs += probs[j + w * i];
		}
	}
	for (float& prob : probs)
	{
		prob /= sum_probs;
	}
}

void PathTracer::init_alias_table()
{
	int n = probs.size();
	prob_table = vector<float>(n), alias_table = vector<int>(n);
	queue<int> smaller, larger;
	for (int i = 0; i < n; ++i) {
		prob_table[i] = probs[i] * n;
		if (prob_table[i] < 1.0f)
			smaller.push(i);
		else
			larger.push(i);
	}
	while (!smaller.empty() && !larger.empty()) {
		int small = smaller.front();
		smaller.pop();
		int large = larger.front();
		larger.pop();
		alias_table[small] = large;
		prob_table[large] = (prob_table[large] + prob_table[small]) - 1.0f;
		if (prob_table[large] < 1.0f)
			smaller.push(large);
		else
			larger.push(large);
	}
}

PathTracer::PathTracer(const Scene* scene, const SObj* cam_obj, Image* img, size_t spp)
	: scene{ scene },
	bvh{ const_cast<Scene*>(scene) },
	img{ img },
	cam{ cam_obj->Get<Cmpt::Camera>() },
	ccs{ cam->GenCoordinateSystem(cam_obj->Get<Cmpt::L2W>()->value) },
	spp{ spp }
{
	IntersectorVisibility::Instance();
	IntersectorClosest::Instance();

	scene->Each([this](const Cmpt::Light* light) ->bool {
		if (!vtable_is<EnvLight>(light->light.get()))
			return true; // continue

		env_light = static_cast<const EnvLight*>(light->light.get());
		return false; // stop
		});

	// TODO: preprocess env_light here
	init_probs();
	init_alias_table();
}

float PathTracer::PDF_based_alias(const vecf3& dir, const EnvLight* env_light)const
{
	vecf3 wi = dir.normalize();
	int w = env_light->texture->img.get()->width,
		h = env_light->texture->img.get()->height;
	float phi = atan2(-wi[0], -wi[1]) + PI<float>;
	float theta = acos(wi[1]);

	float u = phi / (2.f * PI<float>),
		v = 1 - (theta / PI<float>);
	pointf2 texcoord = pointf2(u, v);
	//get the central int coord
	int x = round(w * u - 0.5),
		y = round(h * v - 0.5);
	int idxx = x + w * y;
	if (idxx >= prob_table.size())idxx = prob_table.size() - 1;
	float P_img = prob_table[idxx];
	float env_pd = P_img / (2.f * PI<float>*PI<float>*sin(theta));
	return env_pd;
}

std::tuple<rgbf, vecf3, float> PathTracer::sample_based_alias(const normalf& n, const EnvLight* env_light)const
{
	int w = env_light->texture->img.get()->width,
		h = env_light->texture->img.get()->height;
	float rnf = (rand01<float>() * (w * h));
	int rni = floor(rnf);
	float rnf01 = rnf - rni;
	if (rni >= prob_table.size())rni = prob_table.size() - 1;
	int idx = rnf01 < prob_table[rni] ? rni : alias_table[rni];
	int x = idx % w;
	int y = (idx - x) / w;
	float u = (x + rand01<float>()) / w;
	float v = (y + rand01<float>()) / h;
	float theta = (1.f - v) * PI<float>;
	float phi = 2.f * u * PI<float>;
	vecf3 wi(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));
	float P_img = prob_table[x + w * y];
	float env_pd = P_img / (2.f * PI<float>*PI<float>*sin(theta));
	return { env_light->Radiance(wi),wi.normalize(),env_pd };
}

void PathTracer::Run() {
	img->SetAll(0.f);
	auto start = std::chrono::high_resolution_clock::now();

	// #ifdef NDEBUG
	const size_t core_num = std::thread::hardware_concurrency();
	auto work = [this, core_num](size_t id) {
		for (size_t j = id; j < img->height; j += core_num) {
			for (size_t i = 0; i < img->width; i++) {
				for (size_t k = 0; k < spp; k++) {
					float u = (i + rand01<float>() - 0.5f) / img->width;
					float v = (j + rand01<float>() - 0.5f) / img->height;
					rayf3 r = cam->GenRay(u, v, ccs);
					rgbf Lo;
					do { Lo = Shade(IntersectorClosest::Instance().Visit(&bvh, r), -r.dir, true); } while (Lo.has_nan());
					img->At<rgbf>(i, j) += Lo / float(spp);
				}
			}
			float progress = (j + 1) / float(img->height);
			cout << progress << endl;
		}
	};
	vector<thread> workers;
	for (size_t i = 0; i < core_num; i++)
		workers.emplace_back(work, i);
	for (auto& worker : workers)
		worker.join();
	// #else

		// for (size_t j = 0; j < img->height; j++) {
		// 	for (size_t i = 0; i < img->width; i++) {
		// 		for (size_t k = 0; k < spp; k++) {
		// 			float u = (i + rand01<float>() - 0.5f) / img->width;
		// 			float v = (j + rand01<float>() - 0.5f) / img->height;
		// 			rayf3 r = cam->GenRay(u, v, ccs);
		// 			rgbf Lo;
		// 			do { Lo = Shade(IntersectorClosest::Instance().Visit(&bvh, r), -r.dir, true); } while (Lo.has_nan());
		// 			img->At<rgbf>(i, j) += Lo / static_cast<float>(spp);
		// 		}
		// 	}
		// 	float progress = (j + 1) / float(img->height);
		// 	cout << progress << endl;
		// }
	// #endif
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Execution time: " << duration.count() << "ms" << std::endl;
}

rgbf PathTracer::Shade(const IntersectorClosest::Rst& intersection, const vecf3& wo, bool last_bounce_specular) const {
	// TODO: HW9 - Trace
	// [ Tips ]
	// - EnvLight::Radiance(<direction>), <direction> is pointing to environment light
	// - AreaLight::Radiance(<uv>)
	// - rayf3: point, dir, tmin, **tmax**
	// - IntersectorVisibility::Instance().Visit(&bvh, <rayf3>)
	//   - tmin = EPSILON<float>
	//   - tmax = distance to light - EPSILON<float>
	// - IntersectorCloest::Instance().Visit(&bvh, <rayf3>)
	//   - tmin as default (EPSILON<float>)
	//   - tmax as default (FLT_MAX)
	//
	 // struct IntersectorClosest::Rst {
		//  bool IsIntersected() const noexcept { return sobj != nullptr; }
		//  const SObj* sobj{ nullptr }; // intersection sobj
		//  pointf3 pos; // intersection point's position
		//  pointf2 uv; // texcoord
	 //   normalf n; // normal, normalized
		//  vecf3 tangent; // perpendicular to normal, normalized
	 // };

	constexpr rgbf error_color = rgbf{ 1.f,0.f,1.f };
	constexpr rgbf todo_color = rgbf{ 0.f,1.f,0.f };
	constexpr rgbf zero_color = rgbf{ 0.f,0.f,0.f };

	if (!intersection.IsIntersected()) {
		if (last_bounce_specular && env_light != nullptr) {
			// TODO: environment light

			return env_light->Radiance(-wo);
		}
		else
			return zero_color;
	}

	if (!intersection.sobj->Get<Cmpt::Material>()) {
		auto light = intersection.sobj->Get<Cmpt::Light>();
		if (!light) return error_color;

		if (last_bounce_specular) { // avoid double-count
			auto area_light = dynamic_cast<const AreaLight*>(light->light.get());
			if (!area_light) return error_color;

			// TODO: area light
			return area_light->Radiance(intersection.uv);
		}
		else
			return zero_color;
	}

	rgbf L_dir{ 0.f };
	rgbf L_indir{ 0.f };

	scene->Each([=, &L_dir](const Cmpt::Light* light, const Cmpt::L2W* l2w, const Cmpt::SObjPtr* ptr) {
		// TODO: L_dir += ...
		// - use PathTracer::BRDF to get BRDF value
		SampleLightResult sample_light_rst = SampleLight(intersection, wo, light, l2w, ptr);
		if (sample_light_rst.pd <= 0)
			return;
		if (sample_light_rst.is_infinity) {
			// TODO: L_dir of environment light
			// - only use SampleLightResult::L, n, pd
			// - SampleLightResult::x is useless

			vecf3 wi = -sample_light_rst.n.cast_to<vecf3>();
			float cos_theta = intersection.n.cast_to<vecf3>().dot(wi);
			rgbf f_r = PathTracer::BRDF(intersection, wi.normalize(), wo.normalize());
			rayf3 env_ray(intersection.pos, wi, EPSILON<float>);
			bool visibility = IntersectorVisibility::Instance().Visit(&bvh, env_ray);
			L_dir += sample_light_rst.L * f_r * visibility * abs(cos_theta) / sample_light_rst.pd;
		}
		else {
			// TODO: L_dir of area light
			//intersection.pos: x
			//intersection.n: n(x)
			//sample_light_rst.n: n(y)
			//sample_light_rst.x: y
			// - Intersectors::visibility.Visit(&bvh, <rayf3>)
			//   - tmin = EPSILON<float>
			//   - tmax = distance to light - EPSILON<float>

			vecf3 wi = sample_light_rst.x - intersection.pos;
			//vecf3 wi = intersection.pos - sample_light_rst.x;
			float cos_yx = wi.dot(sample_light_rst.n.cast_to<vecf3>() / (wi.norm()));
			float cos_xy = (-wi).dot(intersection.n.cast_to<vecf3>() / (wi.norm()));
			rgbf f_r = PathTracer::BRDF(intersection, wi.normalize(), wo.normalize());
			rayf3 light_ray(sample_light_rst.x, -wi, EPSILON<float>, 1 - EPSILON<float>);
			bool visibility = IntersectorVisibility::Instance().Visit(&bvh, light_ray);
			float geo_item = abs(cos_xy * cos_yx) / wi.norm2();
			if (cos_yx < 0)//avoid under light
				L_dir += sample_light_rst.L * f_r * geo_item * visibility / sample_light_rst.pd;
		}
		});

	// TODO: Russian Roulette
	// - rand01<float>() : random in [0, 1)
	if (rand01<float>() > 0.9)
		return zero_color;
	// TODO: recursion
	// - use PathTracer::SampleBRDF to get wi and pd (probability density)
	// wi may be **under** the surface
	// - use PathTracer::BRDF to get BRDF value

	std::tuple<vecf3, float> t_ = PathTracer::SampleBRDF(intersection, wo);
	auto wi = std::get<0>(t_);
	auto pd = std::get<1>(t_);
	if ((pd == 0.0))L_indir = zero_color;
	else
	{
		rgbf f_r = PathTracer::BRDF(intersection, wi.normalize(), wo.normalize());
		rayf3 indir_ray(intersection.pos, wi, EPSILON<float>);
		float cos_theta = intersection.n.cast_to<vecf3>().dot(wi) / wi.norm();

		L_indir = Shade(IntersectorClosest::Instance().Visit(&bvh, indir_ray), -wi, false) * f_r * abs(cos_theta) / pd;
	}

	// TODO: combine L_dir and L_indir
	if (isnan(L_indir[0]) && isnan(L_indir[1]) && isnan(L_indir[2]))
		L_indir = zero_color;
	return L_dir + L_indir;
}

PathTracer::SampleLightResult PathTracer::SampleLight(const IntersectorClosest::Rst& intersection, const vecf3& wo, const Cmpt::Light* light, const Cmpt::L2W* l2w, const Cmpt::SObjPtr* ptr) const {
	PathTracer::SampleLightResult rst;

	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return rst; // invalid
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return rst; // not support

	if (wo.dot(intersection.n.cast_to<vecf3>()) < 0)
		return rst;

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);
	//          roughness    0     0.5     1
	// metalness----------------------------
	//     0    |           0.5    0.38    0
	//    0.5   |           0.75   0.56    0
	//     1    |            1     0.75    0
	float p_mat = (1 + metalness) / 2 * (1 - stdBRDF::Alpha(roughness)); // 0 - 1

	auto w2l = l2w->value->inverse();

	float pd_mat, pd_light; // dwi / dA
	vecf3 wi;
	vecf3 light_wi; // wi in light space

	// multi-importance sampling, MIS

	if (vtable_is<AreaLight>(light->light.get())) {
		// [1] area light

		auto area_light = static_cast<const AreaLight*>(light->light.get());
		auto geo = ptr->value->Get<Cmpt::Geometry>();
		if (!geo) return rst; // invalid
		if (!vtable_is<Square>(geo->primitive.get())) return rst; // not support

		rst.n = (l2w->value * normalf{ 0,1,0 }).normalize();
		auto light_p = w2l * intersection.pos; // intersection point's position in light space
		scalef3 world_s = l2w->WorldScale();
		float area = world_s[0] * world_s[1] * Square::area;

		if (rand01<float>() < p_mat) {
			// [1.1] sample material

			// pd_mat : dwi
			tie(wi, pd_mat) = SampleBRDF(intersection, wo);
			light_wi = (w2l * wi).normalize(); // wi in light space

			auto light_r = rayf3{ light_p, light_wi, -std::numeric_limits<float>::max() }; // ray in light space
			auto [isIntersected, t, xz] = light_r.intersect_std_square();
			if (isIntersected) {
				pointf3 p_on_light = pointf3{ xz[0], 0.f, xz[1] };

				pd_light = 1 / area;

				rst.x = l2w->value * p_on_light;
				rst.L = area_light->Radiance({ (xz[0] + 1) / 2, (1 - xz[1]) / 2 });

				// pd_mat : dw -> dA
				float dist2 = light_p.distance2(p_on_light);
				float cos_theta_l = (-light_wi)[1];
				pd_mat *= std::abs(cos_theta_l) / dist2;
			}
			else {
				pd_light = 0.f;
				rst.L = 0.f;
				rst.x = 0.f;
			}
		}
		else {
			// [1.2] sample area light

			auto Xi = uniform_in_square<float>(); // [0, 1] x [0, 1]
			pointf3 p_on_light{ 2 * Xi[0] - 1, 0, 2 * Xi[1] - 1 }; // light space
			vecf3 diff = p_on_light - light_p;
			float dist2 = diff.norm2();
			light_wi = diff / std::sqrt(dist2);
			wi = (l2w->value * light_wi).normalize();

			pd_light = 1.f / area;

			rst.L = area_light->Radiance(Xi.cast_to<pointf2>());
			rst.x = l2w->value * p_on_light;
			rst.n = l2w->UpInWorld().cast_to<normalf>();

			// pd_mat : dw
			matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
			matf3 world_to_surface = surface_to_world.inverse();
			svecf s_wo = (world_to_surface * wo).cast_to<svecf>();
			svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
			pd_mat = brdf->PDF(albedo, metalness, roughness, s_wi, s_wo);

			// pd_mat : dw -> dA
			float cos_theta_l = (-light_wi)[1];
			pd_mat *= std::abs(cos_theta_l) / dist2;
		}
	}
	else if (vtable_is<EnvLight>(light->light.get())) {
		// [2] env light
		auto env_light = static_cast<const EnvLight*>(light->light.get());
		auto light_n = (w2l * intersection.n).normalize(); // intersetion point's normal in light space

		rst.is_infinity = true;
		rst.x = std::numeric_limits<float>::max();

		if (rand01<float>() < p_mat) {
			tie(wi, pd_mat) = SampleBRDF(intersection, wo);
			light_wi = (w2l * wi).normalize();
			rst.L = env_light->Radiance(light_wi);
			// pd_light : dwi
			// pd_light = env_light->PDF(light_wi, light_n); // TODO: use your PDF
			pd_light = PDF_based_alias(light_wi, env_light);
		}
		else {
			// pd_light : dwi
			tie(rst.L, light_wi, pd_light) = sample_based_alias(light_n, env_light);// TODO: use your sampling method
			// tie(rst.L, light_wi, pd_light) = env_light->Sample(light_n);// TODO: use your sampling method
			wi = (l2w->value * light_wi).normalize();
			matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
			matf3 world_to_surface = surface_to_world.inverse();
			svecf s_wo = (world_to_surface * wo).cast_to<svecf>();
			svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
			pd_mat = brdf->PDF(albedo, metalness, roughness, s_wi, s_wo);
		}

		rst.n = -wi.cast_to<normalf>();
	}
	else
		return rst; // not support

	rst.pd = p_mat * pd_mat + (1 - p_mat) * pd_light;

	return rst;
}

std::tuple<vecf3, float> PathTracer::SampleBRDF(const IntersectorClosest::Rst& intersection, const vecf3& wo) {
	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return { vecf3{0.f}, 0.f };
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return { vecf3{0.f}, 0.f };

	matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
	matf3 world_to_surface = surface_to_world.inverse();
	svecf s_wo = (world_to_surface * wo).cast_to<svecf>();

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);

	auto [s_wi, pdf] = brdf->Sample(albedo, metalness, roughness, s_wo);
	if (pdf == 0.f)
		return { vecf3{0.f}, 0.f };

	vecf3 wi = surface_to_world * s_wi;

	return { wi,pdf };
}

rgbf PathTracer::BRDF(IntersectorClosest::Rst intersection, const vecf3& wi, const vecf3& wo) {
	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return rgbf{ 1.f,0.f,1.f };
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return rgbf{ 1.f,0.f,1.f };

	matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
	matf3 world_to_surface = surface_to_world.inverse();
	svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
	svecf s_wo = (world_to_surface * wo).cast_to<svecf>();

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);

	return brdf->BRDF(albedo, metalness, roughness, s_wi, s_wo);
}