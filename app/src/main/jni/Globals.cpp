#include "ShaderCode.h"

GLfloat      model_time        = 0.0f;  /**< Time (in seconds), increased each rendering iteration.                                         */
GLfloat      time_delta        = 0.0f;
const GLuint tesselation_level = 25;    /**< Level of details you would like to split model into. Please use values from th range [8..256]. */
GLfloat      isosurface_level  = 12.0f; /**< Scalar field's isosurface level.                                                               */
unsigned int window_width      = 256;   /**< Window width resolution (pixels).                                                              */
unsigned int window_height     = 256;   /**< Window height resolution (pixels).                                                             */

/* Marching Cubes algorithm-specific constants. */
const GLuint samples_per_axis      = tesselation_level;                                      /**< Amount of samples we break scalar space into (per each axis). */
const GLuint samples_in_3d_space   = samples_per_axis * samples_per_axis * samples_per_axis; /**< Amount of samples in 3D space. */
const GLuint cells_per_axis        = samples_per_axis - 1;                                   /**< Amount of cells per each axis. */
const GLuint cells_in_3d_space     = cells_per_axis * cells_per_axis * cells_per_axis;       /**< Amount of cells in 3D space. */

/** Instance of a timer to measure time moments. */
Timer timer;

const int n_spheres = 3;
const int n_sphere_position_components = 4;

const int n_k_particles             = 4;  // * 1024
const int n_particles 				= n_k_particles * 1024;
const int space_cells_per_axis		= 16;
const int space_cells_in_3d_space   = space_cells_per_axis * space_cells_per_axis * space_cells_per_axis;
const float effect_radius           = 1.0/float(space_cells_per_axis); //max as per shader implementation
const float particle_radius         = effect_radius;
const float wall_offset             = 2 * effect_radius;

Matrix        mvp;