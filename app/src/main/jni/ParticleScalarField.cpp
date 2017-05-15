#include "ShaderCode.h"

GLuint particle_scalar_field_program_id                    		      = 0;

GLuint particle_scalar_field_vert_shader_id                 		  = 0;
GLuint particle_scalar_field_frag_shader_id                 		  = 0;

GLuint particle_scalar_field_buffer_id                        		  = 0;

GLuint particle_scalar_field_transform_feedback_object_id   		  = 0;
const GLchar* particle_scalar_field_varying_name[]          		  = {
                                                            		        "field_value"
                                                            		    };

GLuint particle_scalar_field_texture_object_id                     	   = 0;

GLuint        particle_scalar_field_uniform_cells_per_axis_id          = 0;
const GLchar* particle_scalar_field_uniform_cells_per_axis_name        = "cells_per_axis";
GLuint        particle_scalar_field_uniform_samples_per_axis_id        = 0;
const GLchar* particle_scalar_field_uniform_samples_per_axis_name      = "samples_per_axis";
GLuint        particle_scalar_field_uniform_n_particles_id             = 0;
const GLchar* particle_scalar_field_uniform_n_particles_name           = "n_particles";

GLuint        particle_scalar_field_uniform_particle_radius_id         = 0;
const GLchar* particle_scalar_field_uniform_particle_radius_name       = "particle_radius";

GLuint        particle_scalar_field_uniform_sampler_space_arr_id       = 0;
const GLchar* particle_scalar_field_uniform_sampler_space_arr_name     = "space_arr";
GLuint        particle_scalar_field_uniform_sampler_index_arr_id       = 0;
const GLchar* particle_scalar_field_uniform_sampler_index_arr_name     = "index_arr";
GLuint        particle_scalar_field_uniform_sampler_position_arr_id    = 0;
const GLchar* particle_scalar_field_uniform_sampler_position_arr_name  = "position_arr";


const GLchar* particle_scalar_field_vert_shader = GLSL(
    
    
    precision highp isampler2D; /**< Specify high precision for isampler2D type. */
    precision highp isampler3D; /**< Specify high precision for isampler3D type. */
    precision highp sampler2D;  /**< Specify high precision for sampler2D type. */
    precision highp sampler3D;  /**< Specify high precision for sampler3D type. */


    uniform float particle_radius;
    uniform int cells_per_axis;
    uniform int samples_per_axis;
    uniform int n_particles;
    
    uniform isampler3D space_arr;
    uniform isampler2D index_arr;
    uniform sampler2D position_arr;

    out vec4 field_value;
    

    float kernel(float dist){
        if(dist > 1.0 || dist < 0.0){
            return 0.0;
        }
        if(dist < 0.5){
            return (1.0 - 4.0*dist*dist*dist);
        }
        dist = 1.0 - dist;
        return 4.0*dist*dist*dist;
    }

    float kernel_D1(float dist){
    	if(dist > 1.0 || dist < 0.0){
            return 0.0;
        }

        if(dist < 0.5){
            return - 12.0*dist*dist;
        }

        dist = 1.0 - dist;
        return - 12.0*dist*dist;
    }

    float kernel_D2(float dist){
    	if(dist > 1.0 || dist < 0.0){
            return 0.0;
        }

        if(dist < 0.5){
            return - 24.0*dist;
        }

        dist = 1.0 - dist;
        return 24.0*dist;
    }

    vec4 compute_field(vec3 effectee, vec3 effector){
        float norm_dist = distance(effector,effectee)/particle_radius;
        vec3 r = normalize(effector - effectee);
        return vec4(24.0*kernel(norm_dist),r*kernel_D1(norm_dist));
    }

    int nearest_int(float f){
        int i = int(f);
        if(f-float(i) < 0.5){
            return i;
        }
        return i+1;
    }
    
    ivec3 get_nearest_cell(vec3 position){
        return ivec3(nearest_int(position.x*float(cells_per_axis)), 
                     nearest_int(position.y*float(cells_per_axis)), 
                     nearest_int(position.z*float(cells_per_axis)));
    }

    vec2 decode_particle_index(int index){
        int n_k_particles = n_particles / 1024;
        return vec2(float(index%1024)/1023.0,float(index/1024)/float(n_k_particles - 1));
    }

    int get_cell_index(ivec3 cell){
        int index = cell.z;
        index *= cells_per_axis;
        index += cell.y;
        index *= cells_per_axis;
        index += cell.x;

        return index;
    }

    ivec3 decode_cell_index(int index){
        ivec3 cell;
        cell.x = index%cells_per_axis;
        index /= cells_per_axis;
        cell.y = index%cells_per_axis;
        index /= cells_per_axis;
        cell.z = index;
        return cell;
    }

    int get_start_index(ivec3 cell){
        return textureLod(space_arr,vec3(cell)/float(cells_per_axis - 1),0.0).r;
    }

    int get_end_index(ivec3 cell){
        int cells_in_3d = cells_per_axis * cells_per_axis * cells_per_axis;
        int index = get_cell_index(cell);
        if(index == cells_in_3d - 1){
            return n_particles;
        }
        ivec3 new_cell = decode_cell_index(index + 1);
        return textureLod(space_arr,vec3(new_cell)/float(cells_per_axis - 1),0.0).r;
    }

    bool cell_in_space(ivec3 cell){
        return (cell.x >= 0 && cell.x < cells_per_axis &&
                cell.y >= 0 && cell.y < cells_per_axis &&
                cell.z >= 0 && cell.z < cells_per_axis);
    }

    vec3 get_sample_position(int ID){
    	int index = ID;
    	vec3 pos;
    	pos.x = float(index % samples_per_axis)/float(samples_per_axis-1);
    	index /= samples_per_axis;
    	pos.y = float(index % samples_per_axis)/float(samples_per_axis-1);
    	index /= samples_per_axis;
    	pos.z = float(index)/float(samples_per_axis-1);
    	return pos;
    }

    void main(){

        const ivec3 offsets[8] = ivec3[8]
        (
            ivec3( 0,  0,  0),
            ivec3(-1,  0,  0),
            ivec3(-1,  0, -1),
            ivec3( 0,  0, -1),
            ivec3( 0, -1,  0),
            ivec3(-1, -1,  0),
            ivec3(-1, -1, -1),
            ivec3( 0, -1, -1)
        );
        vec3 sample_position = get_sample_position(gl_VertexID);
        ivec3 nearest_cell = get_nearest_cell(sample_position);

        vec4 value = vec4(0.0);
        for(int i=0; i!= 8 ; i++){
            ivec3 cell = nearest_cell + offsets[i];
            if(!cell_in_space(cell)){
                continue;
            }
            int start_index = get_start_index(cell);
            int end_index = get_end_index(cell);

            for(int index = start_index; index != end_index; index++){
                int extended_index = textureLod(index_arr,decode_particle_index(index),0.0).r;
                vec3 particle_position = textureLod(position_arr,decode_particle_index(extended_index),0.0).rgb;
                value += compute_field(sample_position, particle_position);
            }
        }
        field_value = value;
    }
);

const GLchar* particle_scalar_field_frag_shader = GLSL(
    void main()
    {
    }
);


void SetUpParticleScalarField(){

    GL_CHECK(glGenBuffers(1, &particle_scalar_field_buffer_id));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, particle_scalar_field_buffer_id));
    GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 4*samples_in_3d_space*sizeof(GLfloat), NULL, GL_STREAM_READ));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));

    Shader::processShader(&particle_scalar_field_vert_shader_id, particle_scalar_field_vert_shader, GL_VERTEX_SHADER);
    Shader::processShader(&particle_scalar_field_frag_shader_id, particle_scalar_field_frag_shader, GL_FRAGMENT_SHADER);
    
    particle_scalar_field_program_id = GL_CHECK(glCreateProgram());

    GL_CHECK(glAttachShader(particle_scalar_field_program_id, particle_scalar_field_vert_shader_id));
    GL_CHECK(glAttachShader(particle_scalar_field_program_id, particle_scalar_field_frag_shader_id));
    
    GL_CHECK(glTransformFeedbackVaryings(particle_scalar_field_program_id, 1, particle_scalar_field_varying_name, GL_SEPARATE_ATTRIBS));

    GL_CHECK(glLinkProgram(particle_scalar_field_program_id));

    
    particle_scalar_field_uniform_n_particles_id           = GL_CHECK(glGetUniformLocation(particle_scalar_field_program_id, particle_scalar_field_uniform_n_particles_name         ));
    particle_scalar_field_uniform_cells_per_axis_id        = GL_CHECK(glGetUniformLocation(particle_scalar_field_program_id, particle_scalar_field_uniform_cells_per_axis_name      ));
    particle_scalar_field_uniform_samples_per_axis_id      = GL_CHECK(glGetUniformLocation(particle_scalar_field_program_id, particle_scalar_field_uniform_samples_per_axis_name    ));
    particle_scalar_field_uniform_particle_radius_id       = GL_CHECK(glGetUniformLocation(particle_scalar_field_program_id, particle_scalar_field_uniform_particle_radius_name     ));
    particle_scalar_field_uniform_sampler_space_arr_id     = GL_CHECK(glGetUniformLocation(particle_scalar_field_program_id, particle_scalar_field_uniform_sampler_space_arr_name   ));
    particle_scalar_field_uniform_sampler_index_arr_id     = GL_CHECK(glGetUniformLocation(particle_scalar_field_program_id, particle_scalar_field_uniform_sampler_index_arr_name   ));
    particle_scalar_field_uniform_sampler_position_arr_id  = GL_CHECK(glGetUniformLocation(particle_scalar_field_program_id, particle_scalar_field_uniform_sampler_position_arr_name));
    
    GL_CHECK(glUseProgram(particle_scalar_field_program_id));

    GL_CHECK(glUniform1i(particle_scalar_field_uniform_n_particles_id          ,n_particles              ));
    GL_CHECK(glUniform1i(particle_scalar_field_uniform_cells_per_axis_id       ,space_cells_per_axis     ));
    GL_CHECK(glUniform1i(particle_scalar_field_uniform_samples_per_axis_id     ,samples_per_axis         ));
    GL_CHECK(glUniform1f(particle_scalar_field_uniform_particle_radius_id      ,particle_radius          ));
    GL_CHECK(glUniform1i(particle_scalar_field_uniform_sampler_space_arr_id    ,9                        ));
    GL_CHECK(glUniform1i(particle_scalar_field_uniform_sampler_index_arr_id    ,8                        ));
    GL_CHECK(glUniform1i(particle_scalar_field_uniform_sampler_position_arr_id ,5                        ));

    GL_CHECK(glGenTransformFeedbacks(1, &particle_scalar_field_transform_feedback_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, particle_scalar_field_transform_feedback_object_id));

    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particle_scalar_field_buffer_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));


    GL_CHECK(glGenTextures(1, &particle_scalar_field_texture_object_id));

    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_3D, particle_scalar_field_texture_object_id));
    GL_CHECK(glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, samples_per_axis, samples_per_axis, samples_per_axis));

    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL,  0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE));
}


// int first = 1;
// void printBuuff(){
//     if (!first)
//         return;
//     GLfloat* test = new GLfloat[4*samples_in_3d_space];

//     GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, particle_scalar_field_buffer_id));
//     GL_CHECK(glGetBufferSubData(GL_ARRAY_BUFFER, 0, 4*samples_in_3d_space*sizeof(GLfloat), test));

//     for(int i = 0; i!=4*samples_in_3d_space; i++){
//         printf("%f, ",test[i]);
//     }
//     printf("\n");

//     delete[] test;
//     first = 0;
// }



void UpdateParticleScalarField(){

    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, particle_scalar_field_transform_feedback_object_id));

    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    {
        GL_CHECK(glUseProgram(particle_scalar_field_program_id));

        GL_CHECK(glBeginTransformFeedback(GL_POINTS));
        {
            GL_CHECK(glDrawArrays(GL_POINTS, 0, samples_in_3d_space));
        }
        GL_CHECK(glEndTransformFeedback());
    }
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));

    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindBuffer   (GL_PIXEL_UNPACK_BUFFER, particle_scalar_field_buffer_id));
    GL_CHECK(glTexSubImage3D(GL_TEXTURE_3D,
                             0,
                             0,
                             0,
                             0,
                             samples_per_axis,
                             samples_per_axis,
                             samples_per_axis,
                             GL_RGBA,
                             GL_FLOAT,
                             NULL 
                            ));

    // printBuuff();
    
}
