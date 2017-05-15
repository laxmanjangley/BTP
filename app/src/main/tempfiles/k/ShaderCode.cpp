#include "ShaderCode.h"

string LoadShaderCodeFromFile(string path){
	std::string shaderCode;
	std::ifstream shaderStream(path.c_str(), std::ios::in);
	if(shaderStream.is_open()){
		std::string Line = "";
		while(getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}
	return shaderCode;
}


string spheres_updater_vert_shader;
string spheres_updater_frag_shader;
string scalar_field_vert_shader;
string scalar_field_frag_shader;
string marching_cubes_cells_vert_shader;
string marching_cubes_cells_frag_shader;
string marching_cubes_triangles_vert_shader;
string marching_cubes_triangles_frag_shader;


void LoadAllShaders(){

	spheres_updater_vert_shader          = LoadShaderCodeFromFile("shaders/spheres_updater_vert_shader.glsl");
	spheres_updater_frag_shader          = LoadShaderCodeFromFile("shaders/spheres_updater_frag_shader.glsl");
	scalar_field_vert_shader             = LoadShaderCodeFromFile("shaders/scalar_field_vert_shader.glsl");
	scalar_field_frag_shader             = LoadShaderCodeFromFile("shaders/scalar_field_frag_shader.glsl");
	marching_cubes_cells_vert_shader     = LoadShaderCodeFromFile("shaders/marching_cubes_cells_vert_shader.glsl");
	marching_cubes_cells_frag_shader     = LoadShaderCodeFromFile("shaders/marching_cubes_cells_frag_shader.glsl");
	marching_cubes_triangles_vert_shader = LoadShaderCodeFromFile("shaders/marching_cubes_triangles_vert_shader.glsl");
	marching_cubes_triangles_frag_shader = LoadShaderCodeFromFile("shaders/marching_cubes_triangles_frag_shader.glsl");

}