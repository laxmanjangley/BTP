#ifndef SHADERCODE_H
#define SHADERCODE_H


#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
#include <vector>

#ifdef PCIMPL
#include <iostream>
#include <GL/glew.h>

#ifdef PCIMPL
#define LOGI(...) printf(__VA_ARGS__)
#define LOGE(...) fprintf(stderr,__VA_ARGS__)
#else
#define LOG_TAG "libNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#define GLSL(src) "#version 330 core\n" #src
#else
#include <jni.h>
#include <android/log.h>
#include "GLES3/gl31.h"
#include "GLES2/gl2ext.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#define GLSL(src) "#version 310 es\n" #src
#endif

#include "Timer.h"
#include "Matrix.h"
#include "Shader.h"
#include "TriangulationTable.h"

using namespace std;
using namespace MaliSDK;

extern GLfloat model_time;
extern GLfloat time_delta;
extern const GLuint tesselation_level;
extern GLfloat isosurface_level;
extern unsigned int window_width;
extern unsigned int window_height;

extern const GLuint samples_per_axis;
extern const GLuint samples_in_3d_space;
extern const GLuint cells_per_axis;
extern const GLuint cells_in_3d_space;

extern Timer timer;

extern const int n_spheres;
extern const int n_sphere_position_components;

extern const int n_k_particles;
extern const int n_particles;
extern const int space_cells_per_axis;
extern const int space_cells_in_3d_space;
extern const float effect_radius;
extern const float particle_radius;
extern const float wall_offset;

extern Matrix    mvp;

extern GLfloat*     position;

/* 1. Calculate sphere positions stage variable data. */

extern const GLchar* spheres_updater_vert_shader;
extern const GLchar* spheres_updater_frag_shader;
extern GLuint        spheres_updater_program_id;
extern GLuint        spheres_updater_vert_shader_id;
extern GLuint        spheres_updater_frag_shader_id;
extern GLuint        spheres_updater_sphere_positions_buffer_object_id;
extern GLuint        spheres_updater_transform_feedback_object_id;
extern const GLchar* spheres_updater_uniform_time_name;
extern GLuint        spheres_updater_uniform_time_id;
extern const GLchar* sphere_position_varying_name;
void SetUpSphereUpdater();
void DrawSphereUpdater();
void CleanUpSphereUpdater();


/* 2. Scalar field generation stage variable data. */

extern const GLchar* scalar_field_vert_shader;
extern const GLchar* scalar_field_frag_shader;
extern GLuint        scalar_field_program_id;
extern GLuint        scalar_field_vert_shader_id;
extern GLuint        scalar_field_frag_shader_id;
extern GLuint        scalar_field_buffer_object_id;
extern GLuint        scalar_field_transform_feedback_object_id;
extern const GLchar* scalar_field_uniform_samples_per_axis_name;
extern GLuint        scalar_field_uniform_samples_per_axis_id;
extern const GLchar* scalar_field_uniform_spheres_name;
extern GLuint        scalar_field_uniform_spheres_id;
extern const GLchar* scalar_field_value_varying_name;
extern GLuint        scalar_field_texture_object_id;
void SetUpScalarField();
void DrawScalarField();
void CleanUpScalarField();



/* 3. Marching Cubes cell-splitting stage variable data. */
extern const GLchar* marching_cubes_cells_vert_shader;
extern const GLchar* marching_cubes_cells_frag_shader;
extern GLuint        marching_cubes_cells_program_id;
extern GLuint        marching_cubes_cells_vert_shader_id;
extern GLuint        marching_cubes_cells_frag_shader_id;
extern const GLchar* marching_cubes_cells_uniform_cells_per_axis_name;
extern GLuint        marching_cubes_cells_uniform_cells_per_axis_id;
extern const GLchar* marching_cubes_cells_uniform_isolevel_name;
extern GLuint        marching_cubes_cells_uniform_isolevel_id;
extern const GLchar* marching_cubes_cells_uniform_scalar_field_sampler_name;
extern GLuint        marching_cubes_cells_uniform_scalar_field_sampler_id;
extern const GLchar* marching_cubes_cells_varying_name;
extern GLuint        marching_cubes_cells_transform_feedback_object_id;
extern GLuint        marching_cubes_cells_types_buffer_id;
extern GLuint        marching_cubes_cells_types_texture_object_id;
void SetUpMarchingCubesCells();
void DrawMarchingCubesCells();
void CleanUpMarchingCubesCells();



/* 4. Marching Cubes algorithm triangle generation and rendering stage variable data. */
extern const GLchar* marching_cubes_triangles_vert_shader;
extern const GLchar* marching_cubes_triangles_frag_shader;
extern GLuint        marching_cubes_triangles_program_id;
extern GLuint        marching_cubes_triangles_frag_shader_id;
extern GLuint        marching_cubes_triangles_vert_shader_id;
extern const GLchar* marching_cubes_triangles_uniform_samples_per_axis_name;
extern GLuint        marching_cubes_triangles_uniform_samples_per_axis_id;
extern const GLchar* marching_cubes_triangles_uniform_isolevel_name;
extern GLuint        marching_cubes_triangles_uniform_isolevel_id;
extern const GLchar* marching_cubes_triangles_uniform_time_name;
extern GLuint        marching_cubes_triangles_uniform_time_id;
extern const GLchar* marching_cubes_triangles_uniform_mvp_name;
extern GLuint        marching_cubes_triangles_uniform_mvp_id;
extern const GLchar* marching_cubes_triangles_uniform_cell_types_sampler_name;
extern GLuint        marching_cubes_triangles_uniform_cell_types_sampler_id;
extern const GLchar* marching_cubes_triangles_uniform_scalar_field_sampler_name;
extern GLuint        marching_cubes_triangles_uniform_scalar_field_sampler_id;
extern const GLchar* marching_cubes_triangles_uniform_sphere_positions_name;
extern GLuint        marching_cubes_triangles_uniform_sphere_positions_id;
extern const GLchar* marching_cubes_triangles_uniform_tri_table_sampler_name;
extern GLuint        marching_cubes_triangles_uniform_tri_table_sampler_id;
extern GLuint        marching_cubes_triangles_lookup_table_texture_id;
extern GLuint        marching_cubes_triangles_vao_id;
void SetUpMarchingCubesTriangles();
void DrawMarchingCubesTriangles();
void CleanUpMarchingCubesTriangles();


/*5. Marching cubes using geometry shaders*/

extern const GLchar* marching_cubes_geo_shader;
extern const GLchar* marching_cubes_vert_shader;
extern const GLchar* marching_cubes_frag_shader;
extern GLuint        marching_cubes_program_id;
extern GLuint        marching_cubes_geo_shader_id;
extern GLuint        marching_cubes_vert_shader_id;
extern GLuint        marching_cubes_frag_shader_id;
extern const GLchar* marching_cubes_uniform_time_name;
extern GLuint        marching_cubes_uniform_time_id;
extern const GLchar* marching_cubes_uniform_samples_per_axis_name;
extern GLuint        marching_cubes_uniform_samples_per_axis_id;
extern const GLchar* marching_cubes_uniform_iso_level_name;
extern GLuint        marching_cubes_uniform_iso_level_id;
extern const GLchar* marching_cubes_uniform_mvp_name;
extern GLuint        marching_cubes_uniform_mvp_id;
extern const GLchar* marching_cubes_uniform_scalar_field_sampler_name;
extern GLuint        marching_cubes_uniform_scalar_field_sampler_id;
extern const GLchar* marching_cubes_uniform_tri_table_sampler_name;
extern GLuint        marching_cubes_uniform_tri_table_sampler_id;
extern GLuint        marching_cubes_lookup_table_texture_id;
extern GLuint        marching_cubes_vao_id;
void SetUpMarchingCubes();
void DrawMarchingCubes();
void CleanUpMarchingCubes();

/* Draw particles as points*/

extern const GLchar* draw_points_vert_shader;
extern const GLchar* draw_points_frag_shader;
extern GLuint        draw_points_program_id;
extern GLuint        draw_points_frag_shader_id;
extern GLuint        draw_points_vert_shader_id;
extern const GLchar* draw_points_uniform_mvp_name;
extern GLuint        draw_points_uniform_mvp_id;
extern GLuint        draw_points_vao_id;
void SetUpDrawPoints();
void DrawPoints();
void DrawScalarFieldPoints();

/* Particle Updater */

extern GLfloat*      position;
extern vector<int>*  cells;
extern GLint*        ordererd_index;
extern GLint*        space_distribution_index;
extern const GLchar* particle_updater_pressure_update_vert_shader;
extern const GLchar* particle_updater_overall_update_vert_shader;
extern const GLchar* particle_updater_dummy_frag_shader;
extern GLuint        particle_updater_pressure_update_program_id;
extern GLuint        particle_updater_overall_update_program_id;
extern GLuint        particle_updater_pressure_update_vert_shader_id;
extern GLuint        particle_updater_overall_update_vert_shader_id;
extern GLuint        particle_updater_dummy_frag_shader_id;
extern GLuint        particle_updater_position_buffer_id;
extern GLuint        particle_updater_velocity_buffer_id;
extern GLuint        particle_updater_pressure_buffer_id;
extern GLuint        particle_updater_pressure_update_transform_feedback_object_id;
extern const GLchar* particle_updater_pressure_update_varying_name[];
extern GLuint        particle_updater_overall_update_transform_feedback_object_id;
extern const GLchar* particle_updater_overall_update_varying_name[];
extern GLuint        particle_updater_position_texture_object_id;
extern GLuint        particle_updater_velocity_texture_object_id;
extern GLuint        particle_updater_pressure_texture_object_id;
extern GLuint        particle_updater_spacial_index_array_texture_object_id;
extern GLuint        particle_updater_ordered_index_array_texture_object_id;


void SetUpParticleUpdater();
void UpdateParticles();


/*Sort particles*/

extern GLuint        particle_sort_program_id;
extern GLuint        particle_sort_vert_shader_id;
extern GLuint        particle_sort_frag_shader_id;

extern GLuint        particle_sort_new_index_buffer_id;

extern GLuint        particle_sort_uniform_cells_per_axis_id;
extern const GLchar* particle_sort_uniform_cells_per_axis_name;
extern GLuint        particle_sort_uniform_n_particles_id;
extern const GLchar* particle_sort_uniform_n_particles_name;
extern GLuint        particle_sort_uniform_mask1_id;
extern const GLchar* particle_sort_uniform_mask1_name;
extern GLuint        particle_sort_uniform_mask2_id;
extern const GLchar* particle_sort_uniform_mask2_name;
extern GLuint        particle_sort_uniform_sampler_index_arr_id;
extern const GLchar* particle_sort_uniform_sampler_index_arr_name;
extern GLuint        particle_sort_uniform_sampler_position_arr_id;
extern const GLchar* particle_sort_uniform_sampler_position_arr_name;

extern GLuint        particle_sort_transform_feedback_object_id;
extern const GLchar* particle_sort_varying_names[];
extern GLuint        particle_sort_new_index_texture_object_id;

void SetUpParticleSort();
void SortParticles();


/*find start index*/
extern GLuint        find_start_program_id;
extern GLuint        find_start_vert_shader_id;
extern GLuint        find_start_frag_shader_id;

extern GLuint        find_start_index_buffer_id;

extern GLuint        find_start_uniform_cells_per_axis_id;
extern const GLchar* find_start_uniform_cells_per_axis_name;
extern GLuint        find_start_uniform_n_particles_id;
extern const GLchar* find_start_uniform_n_particles_name;
extern GLuint        find_start_uniform_sampler_index_arr_id;
extern const GLchar* find_start_uniform_sampler_index_arr_name;
extern GLuint        find_start_uniform_sampler_position_arr_id;
extern const GLchar* find_start_uniform_sampler_position_arr_name;

extern GLuint        find_start_transform_feedback_object_id;
extern const GLchar* find_start_varying_names[];
extern GLuint        find_start_index_texture_object_id;

void SetUpFindStartIndex();
void FindStartIndex();


/*Particle Scalar Field*/

extern GLuint        particle_scalar_field_program_id;
extern GLuint        particle_scalar_field_vert_shader_id;
extern GLuint        particle_scalar_field_frag_shader_id;
extern GLuint        particle_scalar_field_buffer_id;
extern GLuint        particle_scalar_field_transform_feedback_object_id;
extern const GLchar* particle_scalar_field_varying_name[];
extern GLuint        particle_scalar_field_texture_object_id;
extern GLuint        particle_scalar_field_uniform_cells_per_axis_id;
extern const GLchar* particle_scalar_field_uniform_cells_per_axis_name;
extern GLuint        particle_scalar_field_uniform_samples_per_axis_id;
extern const GLchar* particle_scalar_field_uniform_samples_per_axis_name;
extern GLuint        particle_scalar_field_uniform_n_particles_id;
extern const GLchar* particle_scalar_field_uniform_n_particles_name;
extern GLuint        particle_scalar_field_uniform_particle_radius_id;
extern const GLchar* particle_scalar_field_uniform_particle_radius_name;
extern GLuint        particle_scalar_field_uniform_sampler_space_arr_id;
extern const GLchar* particle_scalar_field_uniform_sampler_space_arr_name;
extern GLuint        particle_scalar_field_uniform_sampler_index_arr_id;
extern const GLchar* particle_scalar_field_uniform_sampler_index_arr_name;
extern GLuint        particle_scalar_field_uniform_sampler_position_arr_id;
extern const GLchar* particle_scalar_field_uniform_sampler_position_arr_name;

void SetUpParticleScalarField();
void UpdateParticleScalarField();

// profiling
extern double sort_time;
extern double find_time;
extern double update_time;
extern double field_time;
extern double mc_time;
extern int frame_count;



#endif //SHADERCODE_H