#include "ShaderCode.h"

GLuint        find_start_program_id                          = 0;
GLuint        find_start_vert_shader_id                      = 0;
GLuint        find_start_frag_shader_id                      = 0;

GLuint        find_start_index_buffer_id                 	 = 0;

GLuint        find_start_uniform_cells_per_axis_id           = 0;
const GLchar* find_start_uniform_cells_per_axis_name         = "cells_per_axis";
GLuint        find_start_uniform_n_particles_id              = 0;
const GLchar* find_start_uniform_n_particles_name            = "n_particles";
GLuint        find_start_uniform_sampler_index_arr_id        = 0;
const GLchar* find_start_uniform_sampler_index_arr_name      = "index_arr";
GLuint        find_start_uniform_sampler_position_arr_id     = 0;
const GLchar* find_start_uniform_sampler_position_arr_name   = "position_arr";

GLuint        find_start_transform_feedback_object_id        = 0;
const GLchar* find_start_varying_names[]                     = { "start_index" };
GLuint        find_start_index_texture_object_id         	 = 0;


const GLchar* find_start_vert_shader = GLSL(

    precision highp isampler2D; /**< Specify high precision for isampler2D type. */
    precision highp isampler3D; /**< Specify high precision for isampler3D type. */
    precision highp sampler2D;  /**< Specify high precision for sampler2D type. */
    precision highp sampler3D;  /**< Specify high precision for sampler3D type. */

	uniform int cells_per_axis;
    uniform int n_particles;

    uniform isampler2D index_arr;
    uniform sampler2D position_arr;

    flat out int start_index;

    vec2 decode_particle_index(int index){
        int n_k_particles = n_particles / 1024;
        return vec2(float(index%1024)/1023.0,float(index/1024)/float(n_k_particles - 1));
    }

    int cell_index(int extended_index){

        vec3 position = textureLod(position_arr, decode_particle_index(extended_index), 0.0).rgb;
        
        int index = int(position.z*float(cells_per_axis));
        index *= cells_per_axis;
        index += int(position.y*float(cells_per_axis));
        index *= cells_per_axis;
        index += int(position.x*float(cells_per_axis));
        return index;

    }

    void main(){
    	int first_cell_index = cell_index(textureLod(index_arr, decode_particle_index(0)              , 0.0).r);
    	int last_cell_index  = cell_index(textureLod(index_arr, decode_particle_index(n_particles - 1), 0.0).r);
    	int my_cell_index = gl_VertexID;
    	

    	if(my_cell_index <= first_cell_index){
    		start_index = 0;
    	}
    	if(my_cell_index > last_cell_index){
			start_index = n_particles;
		}
		if(my_cell_index > first_cell_index 
		  && my_cell_index <= last_cell_index){
		  	
	    	int high = n_particles - 1;
	    	int low = 0;
	    	int mid;
	    	while(true){
	    		mid = (high + low) / 2;
	    		int mid_extended_index = textureLod(index_arr, decode_particle_index(mid), 0.0).r;
	    		
	    		int mid_cell_index = cell_index(mid_extended_index);
	    		if(mid == low){
	    			if(mid_cell_index == my_cell_index){
	    				start_index = mid;
	    			}
	 				else{
	 					start_index = high;
	 				}
	 				break;
	    		}
	    		if(mid_cell_index >= my_cell_index){
	    			high = mid;
	    		}
	    		else{
	    			low = mid;
	    		}
	    	}
	    }
    }
    
);

const GLchar* find_start_frag_shader = GLSL(

	void main(){

	}
);

void SetUpFindStartIndex(){
	GL_CHECK(glGenBuffers(1, &find_start_index_buffer_id));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, find_start_index_buffer_id));
    GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, space_cells_in_3d_space*sizeof(GLint), NULL, GL_STREAM_READ));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));

    find_start_program_id = GL_CHECK(glCreateProgram());

    Shader::processShader(&find_start_vert_shader_id, find_start_vert_shader, GL_VERTEX_SHADER);
    Shader::processShader(&find_start_frag_shader_id, find_start_frag_shader, GL_FRAGMENT_SHADER);

    GL_CHECK(glAttachShader(find_start_program_id, find_start_vert_shader_id));
    GL_CHECK(glAttachShader(find_start_program_id, find_start_frag_shader_id));
    
    GL_CHECK(glTransformFeedbackVaryings(find_start_program_id, 1, find_start_varying_names, GL_SEPARATE_ATTRIBS));

    GL_CHECK(glLinkProgram(find_start_program_id));
    
    find_start_uniform_cells_per_axis_id         = GL_CHECK(glGetUniformLocation(find_start_program_id, find_start_uniform_cells_per_axis_name       ));
    find_start_uniform_n_particles_id            = GL_CHECK(glGetUniformLocation(find_start_program_id, find_start_uniform_n_particles_name          ));
    find_start_uniform_sampler_index_arr_id      = GL_CHECK(glGetUniformLocation(find_start_program_id, find_start_uniform_sampler_index_arr_name    ));
    find_start_uniform_sampler_position_arr_id   = GL_CHECK(glGetUniformLocation(find_start_program_id, find_start_uniform_sampler_position_arr_name ));
    
    GL_CHECK(glUseProgram(find_start_program_id));

    GL_CHECK(glUniform1i(find_start_uniform_cells_per_axis_id        ,space_cells_per_axis     ));
    GL_CHECK(glUniform1i(find_start_uniform_n_particles_id           ,n_particles              ));
    GL_CHECK(glUniform1i(find_start_uniform_sampler_index_arr_id     ,8                        ));
    GL_CHECK(glUniform1i(find_start_uniform_sampler_position_arr_id  ,5                        ));

    GL_CHECK(glGenTransformFeedbacks(1, &find_start_transform_feedback_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, find_start_transform_feedback_object_id));

    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, find_start_index_buffer_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    GL_CHECK(glGenTextures(1, &find_start_index_texture_object_id));

    GL_CHECK(glActiveTexture(GL_TEXTURE9));
    GL_CHECK(glBindTexture(GL_TEXTURE_3D, find_start_index_texture_object_id));
    GL_CHECK(glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32I, space_cells_per_axis, space_cells_per_axis, space_cells_per_axis));

    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL,  0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE));

}

void FindStartIndex(){
	GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, find_start_transform_feedback_object_id));

    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    {
        GL_CHECK(glUseProgram(find_start_program_id));

        GL_CHECK(glBeginTransformFeedback(GL_POINTS));
        {
            GL_CHECK(glDrawArrays(GL_POINTS, 0, space_cells_in_3d_space));
        }
        GL_CHECK(glEndTransformFeedback());
    }
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));

    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    GL_CHECK(glActiveTexture(GL_TEXTURE9));
    GL_CHECK(glBindBuffer   (GL_PIXEL_UNPACK_BUFFER, find_start_index_buffer_id));
    GL_CHECK(glTexSubImage3D(GL_TEXTURE_3D,
                             0,
                             0,
                             0,
                             0,
                             space_cells_per_axis,
                             space_cells_per_axis,
                             space_cells_per_axis,
                             GL_RED_INTEGER,
                             GL_INT,
                             NULL
                            ));
}




// int cell_index(int i){
        
//         int index = int(position[3*i+2]*float(space_cells_per_axis));
//         index *= space_cells_per_axis;
//         index += int(position[3*i+1]*float(space_cells_per_axis));
//         index *= space_cells_per_axis;
//         index += int(position[3*i]*float(space_cells_per_axis));
//         // printf("%f, %f, %f : ",position[3*i]*space_cells_per_axis,position[3*i+1]*space_cells_per_axis,position[3*i+2]*space_cells_per_axis);
//         return index;
// }

// void TestFindStart(){
// 	UpdateParticles();
// 	GLint* test;
// 	test = new GLint[n_particles];
// 	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, particle_sort_new_index_buffer_id));
// 	GL_CHECK(glGetBufferSubData(GL_ARRAY_BUFFER, 0, n_particles*sizeof(GLint), test));
// 	GLint* test_;
// 	test_ = new GLint[space_cells_in_3d_space];
// 	GLint* test_t;
// 	test_t = new GLint[space_cells_in_3d_space];
// 	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, find_start_index_buffer_id));
// 	GL_CHECK(glGetBufferSubData(GL_ARRAY_BUFFER, 0, space_cells_in_3d_space*sizeof(GLint), test_));
// 	for(int i=0; i!= space_cells_in_3d_space;i++){
// 		test_t[i] = -1;
// 	}
// 	int p = -1;
// 	for(int i=0;i!=n_particles;i++){
// 		int c = cell_index(test[i]);
// 		if(c!=p){
// 			test_t[c] = i;
// 			p = c;
// 		}
// 	}
// 	p = 0;
// 	for(int i = 0; i!=space_cells_in_3d_space;i++){
// 		printf("(%i, %i), ", test_[i],test_t[i]);
// 	}
// 	// bool ok = true;
// 	// for(int i=0 ; i!=space_cells_in_3d_space; i++){
// 	// 	int st = test_[i];
// 	// 	if(cell_index(test[st])<i){
// 	// 		ok = false;
// 	// 		printf("%i, ", i);
// 	// 	}
// 	// 	else{
// 	// 		if(st!=0){
// 	// 			if(cell_index(test[st-1]) >= i){
// 	// 				ok = false;
// 	// 				printf("%i, ", i);
// 	// 			}
// 	// 		}
// 	// 	}
// 	// }
// 	// printf("\n%i\n", ok);
// }