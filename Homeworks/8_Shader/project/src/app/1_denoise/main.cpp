#include <UGL/UGL>
#include <UGM/UGM>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <UGM/Interfaces/IMatrix/IMatrix.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "../../tool/Camera.h"
#include "../../tool/SimpleLoader.h"
#include <ANN/ANN.h>
#include <iostream>
#include <unordered_map>
// #include <annoy/annoylib.h>
#include <flann/flann.hpp>
using namespace Ubpa;

// typedef  struct my_pair
// {
// 	int x, y;
// 	my_pair(int xx, int yy) :x(xx), y(yy) {}
// public:
// 	bool operator==(const my_pair& p)const
// 	{
// 		return this->x == p.x && this->y == p.y;
// 	}
// 	bool operator()(const my_pair& l, const my_pair& r)const
// 	{
// 		return l == r;
// 	}
// }ppair;
// static size_t myHash(const my_pair& self)noexcept
// {
// 	static std::hash<std::string>myhash;
// 	return myhash(self.x + "," + self.y);
// }
int hash(int a, int b, int size)
{
	return a + b * size;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
gl::Texture2D loadTexture(char const* path);
gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources);

// settings
unsigned int scr_width = 800;
unsigned int scr_height = 600;
float displacement_bias = 0.f;
float displacement_scale = 1.f;
float displacement_lambda = 0.2f;
bool have_denoise = false;

// camera
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "HW8 - denoise", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	gl::Enable(gl::Capability::DepthTest);

	// build and compile our shader zprogram
	// ------------------------------------
	gl::Shader vs(gl::ShaderType::VertexShader, "../data/shaders/p3t2n3_denoise.vert");
	gl::Shader fs(gl::ShaderType::FragmentShader, "../data/shaders/light.frag");
	gl::Program program(&vs, &fs);
	rgbf ambient{ 0.2f,0.2f,0.2f };
	program.SetTex("albedo_texture", 0);
	program.SetTex("displacementmap", 1);
	program.SetVecf3("point_light_pos", { 0,5,0 });
	program.SetVecf3("point_light_radiance", { 100,100,100 });
	program.SetVecf3("ambient_irradiance", ambient);
	program.SetFloat("roughness", 0.5f);
	program.SetFloat("metalness", 0.f);

	// load model
	// ------------------------------------------------------------------
	auto spot = SimpleLoader::LoadObj("../data/models/spot_triangulated_good.obj", true);
	// world space positions of our cubes
	pointf3 instancePositions[] = {
		pointf3(0.0f,  0.0f,  0.0f),
		pointf3(2.0f,  5.0f, -15.0f),
		pointf3(-1.5f, -2.2f, -2.5f),
		pointf3(-3.8f, -2.0f, -12.3f),
		pointf3(2.4f, -0.4f, -3.5f),
		pointf3(-1.7f,  3.0f, -7.5f),
		pointf3(1.3f, -2.0f, -2.5f),
		pointf3(1.5f,  2.0f, -2.5f),
		pointf3(1.5f,  0.2f, -1.5f),
		pointf3(-1.3f,  1.0f, -1.5f)
	};

	// load and create a texture
	// -------------------------
	gl::Texture2D spot_albedo = loadTexture("../data/textures/spot_albedo.png");

	gl::Texture2D displacementmap = genDisplacementmap(spot);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		gl::ClearColor({ ambient, 1.0f });
		gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit); // also clear the depth buffer now!

		program.SetVecf3("camera_pos", camera.Position);

		// bind textures on corresponding texture units
		program.Active(0, &spot_albedo);
		program.Active(1, &displacementmap);

		// pass projection matrix to shader (note that in this case it could change every frame)
		transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.f);
		program.SetMatf4("projection", projection);

		// camera/view transformation
		program.SetMatf4("view", camera.GetViewMatrix());
		program.SetFloat("displacement_bias", displacement_bias);
		program.SetFloat("displacement_scale", displacement_scale);
		program.SetFloat("displacement_lambda", displacement_lambda);
		program.SetBool("have_denoise", have_denoise);

		// render spots
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			float angle = 20.0f * i + 10.f * (float)glfwGetTime();
			transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
			program.SetMatf4("model", model);
			spot->va->Draw(&program);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	delete spot;

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Movement::UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		have_denoise = !have_denoise;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	gl::Viewport({ 0, 0 }, width, height);
	scr_width = width;
	scr_height = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);
		firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - lastX;
	float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);

	camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

gl::Texture2D loadTexture(char const* path)
{
	gl::Texture2D tex;
	tex.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat, gl::MinFilter::Linear, gl::MagFilter::Linear);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	gl::PixelDataFormat c2f[4] = {
		gl::PixelDataFormat::Red,
		gl::PixelDataFormat::Rg,
		gl::PixelDataFormat::Rgb,
		gl::PixelDataFormat::Rgba
	};
	gl::PixelDataInternalFormat c2if[4] = {
		gl::PixelDataInternalFormat::Red,
		gl::PixelDataInternalFormat::Rg,
		gl::PixelDataInternalFormat::Rgb,
		gl::PixelDataInternalFormat::Rgba
	};
	if (data)
	{
		tex.SetImage(0, c2if[nrChannels - 1], width, height, c2f[nrChannels - 1], gl::PixelDataType::UnsignedByte, data);
		tex.GenerateMipmap();
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return tex;
}

gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources) {
	float* displacementData = new float[1024 * 1024];

	// TODO: HW8 - 1_denoise | genDisplacementmap
	// 1. set displacementData with resources's positions, indices, normals, ...
	size_t Vsize = resources->positions.size();
	size_t indices_size = resources->indices.size();
	std::vector<pointf3> pos_list = resources->positions;
	std::vector<unsigned> indices = resources->indices;
	std::vector<std::vector<size_t>> graphics = std::vector<std::vector<size_t>>(Vsize, std::vector<size_t>(0));
	std::vector<normal<float>> normals = resources->normals;
	std::map<pointi2, float>coord_val;
	// 2. change global variable: displacement_bias, displacement_scale, displacement_lambda

	// ...
	// std::unordered_map<int, float>inv_dis;
	for (int i = 0; i < indices_size; i += 3)
	{
		size_t a = indices[i], b = indices[i + 1], c = indices[i + 2];
		graphics[a].push_back(b);
		graphics[b].push_back(a);
		graphics[a].push_back(c);
		graphics[c].push_back(a);
		graphics[c].push_back(b);
		graphics[b].push_back(c);
	}
	float max_val = -INFINITY;
	float min_val = INFINITY;
	for (int i = 0; i < Vsize; ++i)
	{
		vecf3 dealt_vec = vecf3(0, 0, 0);
		float dealt_dot = 0.0;
		for (unsigned long long j : graphics[i])
		{
			dealt_vec += (pos_list[i].cast_to<vecf3>() / graphics[i].size() - pos_list[j].cast_to<vecf3>() / graphics[i].size());
		}
		// dealt_vec /= ;
		dealt_dot = dealt_vec.dot(normals[i].cast_to<vecf3>());
		pointf2 texcoord = resources->texcoords[i];
		if (texcoord[0] >= 0 && texcoord[1] >= 0)
		{
			displacementData[long(texcoord[0] * 1024) * 1024 + long(texcoord[1] * 1024)] = dealt_dot;
		}
		coord_val[pointi2(int(texcoord[0] * 1024), int(texcoord[1] * 1024))] = dealt_dot;
		max_val = max_val < dealt_dot ? dealt_dot : max_val;
		min_val = min_val > dealt_dot ? dealt_dot : min_val;
	}
	displacement_scale = max_val - min_val;
	displacement_bias = min_val;
	displacement_lambda = 0.75;
	// flann::Matrix<float>dataset(new float[Vsize], Vsize, 1);
	// for (int i = 0; i < Vsize; ++i)
	// {
	// 	dataset[i][0] = dealt_dot_list[i];
	// }
	// flann::Index<flann::L2<float>>index(dataset, flann::KDTreeIndexParams());
	// index.buildIndex();

	// auto vbo_map = resources->name2vb;
	// ANNpoint ann_alloc_pt = annAllocPt(1024 * 1024, 2);

	// resources.
	ANNpointArray data_ptr = annAllocPts(1024 * 1024, 2);
	int cnt_idx = 0;
	int k_idx = 1;//k_nearest_neighbor
	for (auto val : coord_val)
	{
		data_ptr[cnt_idx][0] = val.first[0];
		data_ptr[cnt_idx][1] = val.first[1];
		cnt_idx++;
	}
	ANNkd_tree* kdtree = new ANNkd_tree(data_ptr, cnt_idx, 2);
	for (int i = 0; i < 1024; ++i)
	{
		for (int j = 0; j < 1024; ++j)
		{
			float interpolation_vals = 0;
			ANNpoint cur_point = annAllocPt(2);
			ANNidxArray idx = new ANNidx[k_idx];
			ANNdistArray dist = new ANNdist[k_idx];
			cur_point[0] = i;
			cur_point[1] = j;
			kdtree->annkSearch(cur_point, k_idx, idx, dist);
			for (int k = 0; k < k_idx; k++)
			{
				int x = data_ptr[idx[k]][0];
				int y = data_ptr[idx[k]][1];
				float tmp_val = coord_val[pointi2(x, y)];
				interpolation_vals += tmp_val;
			}
			displacementData[1024 * j + i] = (interpolation_vals / k_idx - displacement_bias) / displacement_scale;
			delete[] idx;
			delete[]dist;
		}
	}

	delete kdtree;

	gl::Texture2D displacementmap;
	displacementmap.SetImage(0, gl::PixelDataInternalFormat::Red,
		1024, 1024, gl::PixelDataFormat::Red, gl::PixelDataType::Float, displacementData);
	displacementmap.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
		gl::MinFilter::Linear, gl::MagFilter::Linear);
	stbi_uc* stbi_data = new stbi_uc[1024 * 1024];
	for (size_t i = 0; i < 1024 * 1024; i++)
		stbi_data[i] = static_cast<stbi_uc>(std::clamp(displacementData[i] * 255.f, 0.f, 255.f));
	stbi_write_png("../data/1_denoise_displacement_map.png", 1024, 1024, 1, stbi_data, 1024);

	return displacementmap;
}