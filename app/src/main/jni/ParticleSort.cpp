#include "ShaderCode.h"

GLuint        particle_sort_program_id                          = 0;
GLuint        particle_sort_vert_shader_id                      = 0;
GLuint        particle_sort_frag_shader_id                      = 0;

GLuint        particle_sort_new_index_buffer_id                 = 0;

GLuint        particle_sort_uniform_cells_per_axis_id           = 0;
const GLchar* particle_sort_uniform_cells_per_axis_name         = "cells_per_axis";
GLuint        particle_sort_uniform_n_particles_id              = 0;
const GLchar* particle_sort_uniform_n_particles_name            = "n_particles";
GLuint        particle_sort_uniform_mask1_id                    = 0;
const GLchar* particle_sort_uniform_mask1_name                  = "mask1";
GLuint        particle_sort_uniform_mask2_id                    = 0;
const GLchar* particle_sort_uniform_mask2_name                  = "mask2";
GLuint        particle_sort_uniform_sampler_index_arr_id        = 0;
const GLchar* particle_sort_uniform_sampler_index_arr_name      = "index_arr";
GLuint        particle_sort_uniform_sampler_position_arr_id     = 0;
const GLchar* particle_sort_uniform_sampler_position_arr_name   = "position_arr";

GLuint        particle_sort_transform_feedback_object_id        = 0;
const GLchar* particle_sort_varying_names[]                     = { "new_index" };
GLuint        particle_sort_new_index_texture_object_id         = 0;



const GLchar* particle_sort_vert_shader = GLSL(

    precision highp isampler2D; /**< Specify high precision for isampler2D type. */
    precision highp isampler3D; /**< Specify high precision for isampler3D type. */
    precision highp sampler2D;  /**< Specify high precision for sampler2D type. */
    precision highp sampler3D;  /**< Specify high precision for sampler3D type. */


    uniform int cells_per_axis;
    uniform int n_particles;
    
    uniform int mask1;
    uniform int mask2;

    uniform isampler2D index_arr;
    uniform sampler2D position_arr;

    flat out int new_index;

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
 
        int my_index                = gl_VertexID;
        int other_index             = my_index^mask2;
        int my_extended_index       = textureLod(index_arr, decode_particle_index(my_index), 0.0).r;
        int other_extended_index    = textureLod(index_arr, decode_particle_index(other_index), 0.0).r;
        int order                   = (mask1 << 1) & my_index;
        int my_cell_index           = cell_index(my_extended_index);
        int other_cell_index        = cell_index(other_extended_index);
        int grt_index;
        if(my_cell_index > other_cell_index || 
          (my_cell_index == other_cell_index && my_extended_index > other_extended_index)){
            grt_index = my_extended_index;
        }
        else{
            grt_index = other_extended_index;
        }               
        int lst_index = (grt_index == other_extended_index)? my_extended_index : other_extended_index;

        if(order == 0){
            if(my_index > other_index){
                new_index = grt_index;
            }
            else{
                new_index = lst_index;
            }
        }
        else{
            if(my_index > other_index){
                new_index = lst_index;
            }
            else{
                new_index = grt_index;
            }
        }
    }
);

const GLchar* particle_sort_frag_shader = GLSL(

    void main(){

    }

);

void SetUpParticleSort(){

    GLint* index_init = new GLint[n_particles];
    for(int i=0; i!=n_particles; i++){
        index_init[i] = i;
    }


    GL_CHECK(glGenBuffers(1, &particle_sort_new_index_buffer_id));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, particle_sort_new_index_buffer_id));
    GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, n_particles*sizeof(GLint), index_init, GL_STREAM_READ));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));

    delete[] index_init;

    particle_sort_program_id = GL_CHECK(glCreateProgram());

    Shader::processShader(&particle_sort_vert_shader_id, particle_sort_vert_shader, GL_VERTEX_SHADER);
    Shader::processShader(&particle_sort_frag_shader_id, particle_sort_frag_shader, GL_FRAGMENT_SHADER);

    GL_CHECK(glAttachShader(particle_sort_program_id, particle_sort_vert_shader_id));
    GL_CHECK(glAttachShader(particle_sort_program_id, particle_sort_frag_shader_id));
    
    GL_CHECK(glTransformFeedbackVaryings(particle_sort_program_id, 1, particle_sort_varying_names, GL_SEPARATE_ATTRIBS));

    GL_CHECK(glLinkProgram(particle_sort_program_id));
    
    particle_sort_uniform_cells_per_axis_id         = GL_CHECK(glGetUniformLocation(particle_sort_program_id, particle_sort_uniform_cells_per_axis_name       ));
    particle_sort_uniform_n_particles_id            = GL_CHECK(glGetUniformLocation(particle_sort_program_id, particle_sort_uniform_n_particles_name          ));
    particle_sort_uniform_mask1_id                  = GL_CHECK(glGetUniformLocation(particle_sort_program_id, particle_sort_uniform_mask1_name                ));
    particle_sort_uniform_mask2_id                  = GL_CHECK(glGetUniformLocation(particle_sort_program_id, particle_sort_uniform_mask2_name                ));
    particle_sort_uniform_sampler_index_arr_id      = GL_CHECK(glGetUniformLocation(particle_sort_program_id, particle_sort_uniform_sampler_index_arr_name    ));
    particle_sort_uniform_sampler_position_arr_id   = GL_CHECK(glGetUniformLocation(particle_sort_program_id, particle_sort_uniform_sampler_position_arr_name ));
    
    GL_CHECK(glUseProgram(particle_sort_program_id));

    GL_CHECK(glUniform1i(particle_sort_uniform_cells_per_axis_id        ,space_cells_per_axis     ));
    GL_CHECK(glUniform1i(particle_sort_uniform_n_particles_id           ,n_particles              ));
    GL_CHECK(glUniform1i(particle_sort_uniform_sampler_index_arr_id     ,8                        ));
    GL_CHECK(glUniform1i(particle_sort_uniform_sampler_position_arr_id  ,5                        ));

    GL_CHECK(glGenTransformFeedbacks(1, &particle_sort_transform_feedback_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, particle_sort_transform_feedback_object_id));

    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particle_sort_new_index_buffer_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    GL_CHECK(glGenTextures(1, &particle_sort_new_index_texture_object_id));

    GL_CHECK(glActiveTexture(GL_TEXTURE8));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, particle_sort_new_index_texture_object_id));
    GL_CHECK(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, 1024, n_k_particles         ));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
}


void SortParticles(){

    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, particle_sort_transform_feedback_object_id));
    GL_CHECK(glUseProgram(particle_sort_program_id));
    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    {   
        int mask1 = 1;
        int mask2 = 1;
        while(mask1 < n_particles){
            mask2 = mask1;
            while(mask2){
                GL_CHECK(glUniform1i(particle_sort_uniform_mask1_id, mask1));
                GL_CHECK(glUniform1i(particle_sort_uniform_mask2_id, mask2));
                GL_CHECK(glActiveTexture(GL_TEXTURE8));
                GL_CHECK(glBindBuffer   (GL_PIXEL_UNPACK_BUFFER, particle_sort_new_index_buffer_id));
                GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,
                                         0,
                                         0,
                                         0,
                                         1024,
                                         n_k_particles,
                                         GL_RED_INTEGER,
                                         GL_INT,
                                         NULL
                                        ));
                GL_CHECK(glBeginTransformFeedback(GL_POINTS));
                {
                    GL_CHECK(glDrawArrays(GL_POINTS, 0, n_particles));
                }
                GL_CHECK(glEndTransformFeedback());
                mask2 = mask2>>1;
            }
            mask1 = mask1<<1;
        }
    }
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    GL_CHECK(glActiveTexture(GL_TEXTURE8));
    GL_CHECK(glBindBuffer   (GL_PIXEL_UNPACK_BUFFER, particle_sort_new_index_buffer_id));
    GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,
                             0,
                             0,
                             0,
                             1024,
                             n_k_particles,
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


// void TestSort(){
//     // SortParticles();
//     // GLint* test;
//     // GLint* isThere;
//     // test = new GLint[n_particles];
//     // isThere = new GLint[n_particles];
//     // GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, particle_sort_new_index_buffer_id));
//     // GL_CHECK(glGetBufferSubData(GL_ARRAY_BUFFER, 0, n_particles*sizeof(GLint), test));
//     // for(int i=0;i!=n_particles; i++){
//     //     isThere[test[i]] = 121;
//     //     // printf("%i\n", cell_index(test[i]));
//     //     // printf("%i\n", cell_index(i));
//     // }
//     // for(int i=0;i!=n_particles;i++){
//     //     if(isThere[i]!=121){
//     //         printf("%i, ", i);
//     //     }
//     //     // printf("%i, ",cell_index(test[i]));
//     // }
//     // printf("\n");
// }