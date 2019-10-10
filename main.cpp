#include "../Externals/Include/Include.h"

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define SHADOW_MAP_SIZE 4096


#define _CRT_SECURE_NO_WARNINGS
GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;
const GLfloat Pi = 3.1415926536f;
GLfloat tick;

using namespace glm;
using namespace std;

vec3 temp = vec3();
mat4 view;
mat4 projection;
GLint um4p;
GLint um4mv;
const aiScene *scene;
//const aiScene *scene2;

///*
static float c = Pi / 180.0f;
static int du = 90, oldmy = -1, oldmx = -1;
static float r = 10.0f, h = 4.0f;
vec3 eye = vec3(0.0f, 0.0f, 0.0f);
vec3 center = vec3(r*cos(-c * du), 5 * h, r*sin(-c * du)) + eye;
vec3 camera_move;
//*/
GLuint program;

//skybox
GLuint skybox_prog;
GLuint tex_envmap;
GLuint skybox_vao;

//vec3 eye = vec3(0.0f, 0.0f, 0.0f);
struct
{
	struct
	{
		GLint inv_vp_matrix;
		GLint eye;
	} skybox;
} uniforms;
//sk

//shadow
GLuint  shadow_tex;
GLint   shadow_mv;
GLint   light_matrix;
GLuint  shadow_prog;
GLint   mvp;
GLint Shadow_Loc;
GLuint light_Loc;
int shadow_index = -1;

struct
{
	GLuint fbo;
	GLuint depthMap;
	GLuint rbo;
} shadowBuffer;

struct
{
	int width;
	int height;
} viewportSize;
//sh

GLuint			program2;
GLuint          window_vao;
GLuint			vertex_shader;
GLuint			fragment_shader;
GLuint			window_buffer;

// FBO parameter
GLuint			FBO;
GLuint			depthRBO;
GLuint			FBODataTexture;

//tinyobj
typedef struct
{
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object
	GLuint vboTex;		// vertex buffer object of texture
	GLuint ebo;

	GLuint p_normal;
	int materialId;
	int indexCount;
	GLuint m_texture;
} Shapes;

Shapes m_shpae;

///*
//assimp
typedef struct Shape
{
	GLuint vao;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_texcoord;
	GLuint ibo;
	int drawCount;
	int materialID;
}Shape;

typedef struct Material
{
	GLuint diffuse_tex;
}Material;

vector<Shape> sp_shapes, sp2_shapes;
vector<Material> sp_materials, sp2_materials;
//*/
vector<const GLchar*> faces;
//---

void My_Reshape(int width, int height);
static const GLfloat window_positions[] =
{
	1.0f,-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,0.0f,
	-1.0f,1.0f,0.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};

char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void freeShaderSource(char** srcp)
{
	delete[] srcp[0];
	delete[] srcp;
}

// define a simple data structure for storing texture image raw data
typedef struct _TextureData
{
	_TextureData(void) :
		width(0),
		height(0),
		data(0)
	{
	}

	int width;
	int height;
	unsigned char* data;
} TextureData;

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
TextureData loadPNG(const char* const pngFilepath)
{
	TextureData texture;
	int components;

	// load the texture with stb image, force RGBA (4 components required)
	stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, 4);

	// is the image successfully loaded?
	if (data != NULL)
	{
		// copy the raw data
		size_t dataSize = texture.width * texture.height * 4 * sizeof(unsigned char);
		texture.data = new unsigned char[dataSize];
		memcpy(texture.data, data, dataSize);

		// mirror the image vertically to comply with OpenGL convention
		for (size_t i = 0; i < texture.width; ++i)
		{
			for (size_t j = 0; j < texture.height / 2; ++j)
			{
				for (size_t k = 0; k < 4; ++k)
				{
					size_t coord1 = (j * texture.width + i) * 4 + k;
					size_t coord2 = ((texture.height - j - 1) * texture.width + i) * 4 + k;
					std::swap(texture.data[coord1], texture.data[coord2]);
				}
			}
		}

		// release the loaded image
		stbi_image_free(data);
	}

	return texture;
}

///*
//assimp
void My_LoadModels()
{
	for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
		aiMaterial *material = scene->mMaterials[i];
		Material mtr;
		aiString texturePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS) {
			// load width, height and data from texturePath.C_Str();
			int width, height;
			TextureData TD = loadPNG(texturePath.C_Str());
			width = TD.width;
			height = TD.height;
			unsigned char *data = TD.data;

			glGenTextures(1, &mtr.diffuse_tex);
			glBindTexture(GL_TEXTURE_2D, mtr.diffuse_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

		}
		// save materialÅc
		sp_materials.push_back(mtr);
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh *mesh = scene->mMeshes[i];
		Shape shape;
		glGenVertexArrays(1, &shape.vao);
		glBindVertexArray(shape.vao);
		//create 3 vbos to hold data
		float *vertices = new float[mesh->mNumVertices * 3];
		float *texCoords = new float[mesh->mNumVertices * 2];
		float *normals = new float[mesh->mNumVertices * 3];
		for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
			vertices[v * 3] = mesh->mVertices[v][0];
			vertices[v * 3 + 1] = mesh->mVertices[v][1];
			vertices[v * 3 + 2] = mesh->mVertices[v][2];
			texCoords[v * 2] = mesh->mTextureCoords[0][v][0];
			texCoords[v * 2 + 1] = mesh->mTextureCoords[0][v][1];
			normals[v * 3] = mesh->mNormals[v][0];
			normals[v * 3 + 1] = mesh->mNormals[v][1];
			normals[v * 3 + 2] = mesh->mNormals[v][2];

		}

		glGenBuffers(1, &shape.vbo_position);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_position);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 3, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &shape.vbo_texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_texcoord);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 2, texCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &shape.vbo_normal);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_normal);
		glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(float) * 3, normals, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		delete[] vertices;
		delete[] texCoords;
		delete[] normals;

		// create 1 ibo to hold data
		unsigned int *indices = new unsigned int[mesh->mNumFaces * 3];
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
			indices[f * 3] = mesh->mFaces[f].mIndices[0];
			indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
			indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
		}

		glGenBuffers(1, &shape.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->mNumFaces * sizeof(unsigned int) * 3, indices, GL_STATIC_DRAW);

		delete[] indices;

		shape.materialID = mesh->mMaterialIndex;
		shape.drawCount = mesh->mNumFaces * 3;

		//save shape
		sp_shapes.push_back(shape);
	}
	aiReleaseImport(scene);
}
//*/

//tinyobj
void My_LoadModels2()
{

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;

	bool ret = tinyobj::LoadObj(shapes, materials, err, "Plane.obj");
	if (err.size() > 0)
	{
		printf("Load Models Fail! Please check the solution path");
		return;
	}

	printf("Load Models Success ! Shapes size %d Maerial size %d\n", shapes.size(), materials.size());

	for (int i = 0; i < shapes.size(); i++)
	{
		glGenVertexArrays(1, &m_shpae.vao);
		glBindVertexArray(m_shpae.vao);

		glGenBuffers(3, &m_shpae.vbo);
		glGenBuffers(1, &m_shpae.p_normal);
		glBindBuffer(GL_ARRAY_BUFFER, m_shpae.vbo);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() * sizeof(float) + shapes[i].mesh.normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, 0, shapes[i].mesh.positions.size() * sizeof(float), &shapes[i].mesh.positions[0]);
		glBufferSubData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() * sizeof(float), shapes[i].mesh.normals.size() * sizeof(float), &shapes[i].mesh.normals[0]);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)(shapes[i].mesh.positions.size() * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, m_shpae.p_normal);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.normals.size() * sizeof(float), shapes[i].mesh.normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_shpae.vboTex);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.texcoords.size() * sizeof(float), shapes[i].mesh.texcoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shpae.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[i].mesh.indices.size() * sizeof(unsigned int), shapes[i].mesh.indices.data(), GL_STATIC_DRAW);
		m_shpae.materialId = shapes[i].mesh.material_ids[0];
		m_shpae.indexCount = shapes[i].mesh.indices.size();


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}
	/*
	texture_data tdata = load_png("ladybug_diff.png");

	glGenTextures(1, &m_shpae.m_texture);
	glBindTexture(GL_TEXTURE_2D, m_shpae.m_texture);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	*/
}

void My_Init()
{
	//printf("%x", GL_DRAW_FRAMEBUFFER);
	//printf("%x", GL_FRAMEBUFFER);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(1.0f, 1.6f, 1.0f, 1.0f);

	///*
	//shadow
	GLuint shadow_vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint shadow_fs = glCreateShader(GL_FRAGMENT_SHADER);
	char** shadowVertexSource = loadShaderSource("quad.vs.glsl");
	char** shadowFragmentSource = loadShaderSource("quad.fs.glsl");
	glShaderSource(shadow_vs, 1, shadowVertexSource, NULL);
	glShaderSource(shadow_fs, 1, shadowFragmentSource, NULL);
	freeShaderSource(shadowVertexSource);
	freeShaderSource(shadowFragmentSource);
	glCompileShader(shadow_vs);
	glCompileShader(shadow_fs);
	glAttachShader(shadow_prog, shadow_vs);
	glAttachShader(shadow_prog, shadow_fs);
	shaderLog(shadow_vs);
	shaderLog(shadow_fs);
	glLinkProgram(shadow_prog);
	//glUseProgram(shadow_prog);

	mvp = glGetUniformLocation(shadow_prog, "mvp");

	//sh
	//*/

	//skybox
	skybox_prog = glCreateProgram();
	GLuint skybox_frag = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint skybox_ver = glCreateShader(GL_VERTEX_SHADER);
	char** skyboxVertexSource = loadShaderSource("skybox.vs.glsl");
	char** skyboxFragmentSource = loadShaderSource("skybox.fs.glsl");
	glShaderSource(skybox_ver, 1, skyboxVertexSource, NULL);
	glShaderSource(skybox_frag, 1, skyboxFragmentSource, NULL);
	freeShaderSource(skyboxVertexSource);
	freeShaderSource(skyboxFragmentSource);
	glCompileShader(skybox_ver);
	glCompileShader(skybox_frag);
	glAttachShader(skybox_prog, skybox_ver);
	glAttachShader(skybox_prog, skybox_frag);
	shaderLog(skybox_ver);
	shaderLog(skybox_frag);
	glLinkProgram(skybox_prog);
	glUseProgram(skybox_prog);

	uniforms.skybox.inv_vp_matrix = glGetUniformLocation(skybox_prog, "inv_vp_matrix");
	uniforms.skybox.eye = glGetUniformLocation(skybox_prog, "eye");

	faces.push_back("right.png");
	faces.push_back("left.png");
	faces.push_back("top.png");
	faces.push_back("bottom.png");
	faces.push_back("back.png");
	faces.push_back("front.png");

	TextureData envmap_data;
	//TextureData envmap_data = loadPNG("mountaincube.png");
	glGenTextures(1, &tex_envmap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);
	for (int i = 0; i < 6; ++i)
	{
		envmap_data = loadPNG(faces[i]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGBA,
			envmap_data.width,
			envmap_data.height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			envmap_data.data);
		//envmap_data.data + i * (envmap_data.width * (envmap_data.height /6) * sizeof(unsigned char) * 4));
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	delete[] envmap_data.data;

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glGenVertexArrays(1, &skybox_vao);
	//------------------

	scene = aiImportFile("nanosuit.obj", aiProcessPreset_TargetRealtime_MaxQuality);
	//scene2 = aiImportFile("nanosuit.obj", aiProcessPreset_TargetRealtime_MaxQuality);
	program = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	shaderLog(vertexShader);
	shaderLog(fragmentShader);
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	um4mv = glGetUniformLocation(program, "um4mv");
	um4p = glGetUniformLocation(program, "um4p");
	shadow_mv = glGetUniformLocation(program, "shadow_matrix");
	shadow_tex = glGetUniformLocation(program, "shadow_tex");
	Shadow_Loc = glGetUniformLocation(program, "shadow_index");
	light_Loc = glGetUniformLocation(program, "light_pos");
	glUseProgram(program);

	My_LoadModels();
	My_LoadModels2();

	///*
	//shadow fbo
	glGenFramebuffers(1, &shadowBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);

	glGenTextures(1, &shadowBuffer.depthMap);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBuffer.depthMap, 0);

	/*
	//add
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//add
	*/
	//sh fbo
	//*/

	/*
	program2 = glCreateProgram();

	char** FB_vertexShaderSource = loadShaderSource("FB_vertex.vs.glsl");
	char** FB_fragmentShaderSource = loadShaderSource("FB_fragment.fs.glsl");
	GLuint vs2 = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs2, 1, FB_vertexShaderSource, NULL);
	freeShaderSource(FB_vertexShaderSource);
	glCompileShader(vs2);
	shaderLog(vs2);
	GLuint fs2 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs2, 1, FB_fragmentShaderSource, NULL);
	freeShaderSource(FB_fragmentShaderSource);
	glCompileShader(fs2);
	shaderLog(fs2);
	glAttachShader(program2, vs2);
	glAttachShader(program2, fs2);
	glLinkProgram(program2);

	glGenVertexArrays(1, &window_vao);
	glBindVertexArray(window_vao);

	glGenBuffers(1, &window_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, window_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_positions), window_positions, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenFramebuffers(1, &FBO);

	My_Reshape(1440, 900);
	//////////////////////////////////////////////////////////////////////////
	/////////Create RBO and Render Texture in Reshape Function////////////////
	//////////////////////////////////////////////////////////////////////////
	*/
}

void My_Display()
{
	//object
	mat4 Identy_Init(1.0);
	mat4 mv_matrix = translate(Identy_Init, temp);
	mv_matrix = translate(mv_matrix, vec3(-10.0f, -13.0f, -8.0f));
	mv_matrix = scale(mv_matrix, vec3(0.5f, 0.35f, 0.5f));

	mat4 scale_bias_matrix =
		translate(mat4(), vec3(0.5f, 0.5f, 0.5f)) *
		scale(mat4(), vec3(0.5f, 0.5f, 0.5f));
	mat4 obj_matrix = translate(Identy_Init, temp);
	obj_matrix = mv_matrix;
	mat4 quad_matrix = translate(Identy_Init, temp);
	quad_matrix = translate(quad_matrix, vec3(-10.0f, -13.0f, 3.0f));
	quad_matrix = scale(quad_matrix, vec3(2.2f, 2.2f, 2.2f));
	quad_matrix = rotate(quad_matrix, 90.f, vec3(0.0f, 1.0f, 0.0f));
	///*
	
	// ----- Begin Shadow Map Pass -----
	const float shadow_range = 15.0f;
	mat4 light_proj_matrix = ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, 0.0f, 100.0f);
	mat4 light_view_matrix = lookAt(vec3(-31.75, 26.05, -97.72), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;

	//mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;
	mat4 shadow_sbpv_matrix = light_vp_matrix;

	glUseProgram(shadow_prog);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	shadow_index = 0;
	glUniform1i(Shadow_Loc, shadow_index);
	glUniformMatrix4fv(mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * obj_matrix));
	for (unsigned int i = 0; i < sp_shapes.size(); ++i) {
		glBindVertexArray(sp_shapes[i].vao);
		int materialID = sp_shapes[i].materialID;
		glBindTexture(GL_TEXTURE_2D, sp_materials[materialID].diffuse_tex);

		glDrawElements(GL_TRIANGLES, sp_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	shadow_index = 1;
	glUniform1i(Shadow_Loc, shadow_index);
	glUniformMatrix4fv(mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * quad_matrix));
	glBindVertexArray(m_shpae.vao);
	glDrawElements(GL_TRIANGLES, m_shpae.indexCount, GL_UNSIGNED_INT, 0);

	glDisable(GL_POLYGON_OFFSET_FILL);
	// ----- End Shadow Map Pass -----
	//*/

	// (1) Bind the framebuffer object correctly
	// (2) Draw the buffer with color
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// ----- Begin Blinn-Phong Shading Pass -----
	//*****************add
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//**********************

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//*****************add
	glViewport(0, 0, viewportSize.width, viewportSize.height);
	//**********************
	static const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const GLfloat one = 1.0f;

	// TODO :
	// (1) Clear the color buffer (GL_COLOR) with the color of white
	// (2) Clear the depth buffer (GL_DEPTH) with value one 
//	glClearBufferfv(GL_COLOR, 0, white);
//	glClearBufferfv(GL_DEPTH, 0, &one);


	///*
	//skybox
	//tick = glutGet(GLUT_ELAPSED_TIME)*5.0;
	//eye = vec3(0.0f, 0.0f, 0.0f) + camera_move;
	//center = vec3(r*cos(c*du), 5 * h, r*sin(c*du)) + camera_move;
	mat4 view_matrix = lookAt(eye, vec3(-1.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 inv_vp_matrix = inverse(projection * view_matrix);

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);

	glUseProgram(skybox_prog);
	glBindVertexArray(skybox_vao);

	glUniformMatrix4fv(uniforms.skybox.inv_vp_matrix, 1, GL_FALSE, &inv_vp_matrix[0][0]);
	glUniform3fv(uniforms.skybox.eye, 1, &eye[0]);

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	//*/
	glUseProgram(program);
	
	///*
	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glUniform1i(shadow_tex, 0);

	mat4 shadow_matrix = shadow_sbpv_matrix * obj_matrix;
	//*/
	shadow_index = 0;
	glUniform1i(Shadow_Loc, shadow_index);
	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * obj_matrix));
	glUniformMatrix4fv(shadow_mv, 1, GL_FALSE, value_ptr(shadow_matrix));
	//glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	///*
	//assimp
	for (unsigned int i = 0; i < sp_shapes.size(); ++i) {
		glBindVertexArray(sp_shapes[i].vao);
		int materialID = sp_shapes[i].materialID;
		glBindTexture(GL_TEXTURE_2D, sp_materials[materialID].diffuse_tex);

		glDrawElements(GL_TRIANGLES, sp_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	//*/
	shadow_index = 1;
	shadow_matrix = shadow_sbpv_matrix * quad_matrix;
	glUniform1i(Shadow_Loc, shadow_index);
	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * quad_matrix));
	glUniformMatrix4fv(shadow_mv, 1, GL_FALSE, value_ptr(shadow_matrix));
	glBindVertexArray(m_shpae.vao);
	glDrawElements(GL_TRIANGLES, m_shpae.indexCount, GL_UNSIGNED_INT, 0);

	/*
   // Re-bind the framebuffer and clear it
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, FBODataTexture);

   // (1) Bind the vao we want to render
   // (2) Use the correct shader program
   glBindVertexArray(window_vao);
   glUseProgram(program2);

   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   */

	glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	viewportSize.width = width;
	viewportSize.height = height;
	glViewport(0, 0, width, height);
	float viewportAspect = (float)width / (float)height;
	projection = perspective(radians(80.0f), viewportAspect, 0.1f, 1000.0f);
	view = lookAt(eye, vec3(-1.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	/*
	// If the windows is reshaped, we need to reset some settings of framebuffer
	glDeleteRenderbuffers(1, &depthRBO);
	glDeleteTextures(1, &FBODataTexture);
	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);

	// (1) Generate a texture for FBO
	// (2) Bind it so that we can specify the format of the textrue
	glGenTextures(1, &FBODataTexture);
	glBindTexture(GL_TEXTURE_2D, FBODataTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// (1) Bind the framebuffer with first parameter "GL_DRAW_FRAMEBUFFER"
	// (2) Attach a renderbuffer object to a framebuffer object
	// (3) Attach a texture image to a framebuffer object
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBODataTexture, 0);
	*/

}

void My_Timer(int val)
{
	glutPostRedisplay();
	timer_cnt++;
	if (timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
	}
}

void My_Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		oldmx = x, oldmy = y;
		printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
	}
	else if (state == GLUT_UP)
	{
		printf("Mouse %d is released at (%d, %d)\n", button, x, y);
	}
}
///*
void My_Mouse_Drag(int x, int y)
{
	//printf("%d\n",du);  
	du += x - oldmx;
	h += 0.03f*(oldmy - y);
	oldmx = x, oldmy = y;
}
//*/
void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	///*
	if (key == 'w') {
		camera_move += (center - eye) / 2.0f;
	}
	else if (key == 's') {
		camera_move += (eye - center) / 2.0f;
	}
	else if (key == 'd') {
		camera_move += cross((center - eye), vec3(0.0f, 1.0f, 0.0f)) / 2.0f;
	}
	else if (key == 'a') {
		camera_move += cross(vec3(0.0f, 1.0f, 0.0f), (center - eye)) / 2.0f;
	}
	else if (key == 'z') {
		camera_move += vec3(0.0f, 2.5f, 0.0f);
	}
	else if (key == 'x') {
		camera_move += vec3(0.0f, -2.5f, 0.0f);
	}
	//*/

}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);

		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch (id)
	{
	case MENU_TIMER_START:
		if (!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}


int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1440, 900);
	glutCreateWindow("AS4_Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	//int shader = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	//glutAddSubMenu("Shader", shader);

	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);
	/*
	glutSetMenu(shader);
	glutAddMenuEntry("NoShader", Shader_None);
	glutAddMenuEntry("Abstraction", Shader_Abstraction);
	glutAddMenuEntry("LaplacianFilter", Shader_LaplacianFilter);
	glutAddMenuEntry("SharpnessFilter", Shader_SharpnessFilter);
	glutAddMenuEntry("Pixelation", Shader_Pixelation);
	glutAddMenuEntry("Fish-eyedistortion", Shader_Fish_eyedistortion);
	glutAddMenuEntry("Red-BlueStereo", Shader_Red_BlueStereo);
	glutAddMenuEntry("Bloom_Effect", Shader_Bloom_Effect);
	glutAddMenuEntry("halftoning", Shader_halftoning);
	glutAddMenuEntry("NightVision", Shader_NightVision);
	glutAddMenuEntry("Frosted_Glass", Shader_Frosted_Glass);
	glutAddMenuEntry("Swirl", Shader_Swirl);
	*/

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutMotionFunc(My_Mouse_Drag);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0);

	// Enter main event loop.
	glutMainLoop();

	return 0;
}
