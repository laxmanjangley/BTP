#include "Draw.h"

extern string spheres_updater_vert_shader;
extern string spheres_updater_frag_shader;
extern string scalar_field_vert_shader;
extern string scalar_field_frag_shader;
extern string marching_cubes_cells_vert_shader;
extern string marching_cubes_cells_frag_shader;
extern string marching_cubes_triangles_vert_shader;
extern string marching_cubes_triangles_frag_shader;

/* General metaballs example properties. */
GLfloat      model_time        = 0.0f;  /**< Time (in seconds), increased each rendering iteration.                                         */
const GLuint tesselation_level = 32;    /**< Level of details you would like to split model into. Please use values from th range [8..256]. */
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

/** Amount of spheres defining scalar field. This value should be synchronized between all files. */
const int n_spheres = 3;

/** Amount of components in sphere position varying. */
const int n_sphere_position_components = 4;

/** Matrix that transforms vertices from model space to perspective projected world space. */
Matrix        mvp;


/* 1. Calculate sphere positions stage variable data. */
/** Program object id for sphere update stage. */
GLuint        spheres_updater_program_id                                 = 0;
/** Vertex shader id for sphere update stage. */
GLuint        spheres_updater_vert_shader_id                             = 0;
/** Fragment shader id for sphere update stage. */
GLuint        spheres_updater_frag_shader_id                             = 0;

/** Buffer object id to store calculated sphere positions. */
GLuint        spheres_updater_sphere_positions_buffer_object_id          = 0;

/** Id of transform feedback object to keep sphere update stage buffer bindings. */
GLuint        spheres_updater_transform_feedback_object_id               = 0;

/** Name of time uniform for sphere positions update stage. */
const GLchar* spheres_updater_uniform_time_name                          = "time";
/** Location of time uniform for sphere positions update stage. */
GLuint        spheres_updater_uniform_time_id                            = 0;

/** Sphere position output variable's name. */
const GLchar* sphere_position_varying_name                               = "sphere_position";


/* 2. Scalar field generation stage variable data. */
/** Program object id for scalar field generator stage. */
GLuint        scalar_field_program_id                                    = 0;
/** Vertex shader id for scalar field generator stage. */
GLuint        scalar_field_vert_shader_id                                = 0;
/** Fragment shader id for scalar field generator stage. */
GLuint        scalar_field_frag_shader_id                                = 0;

/** Buffer object id to store calculated values of scalar field. */
GLuint        scalar_field_buffer_object_id                              = 0;

/** Id of transform feedback object to keep scalar field buffer binding. */
GLuint        scalar_field_transform_feedback_object_id                  = 0;

/** Name of samples_per_axis uniform. */
const GLchar* scalar_field_uniform_samples_per_axis_name                 = "samples_per_axis";
/** Location of samples_per_axis uniform. */
GLuint        scalar_field_uniform_samples_per_axis_id                   = 0;

/** Name of uniform block storing sphere data. */
const GLchar* scalar_field_uniform_spheres_name                          = "spheres_uniform_block";
/** Index of uniform block storing sphere data. */
GLuint        scalar_field_uniform_spheres_id                            = 0;

/** Scalar_field_value output variable's name. */
const GLchar* scalar_field_value_varying_name                            = "scalar_field_value";

/** Id of a 3D texture object storing scalar field data. */
GLuint        scalar_field_texture_object_id                             = 0;


/* 3. Marching Cubes cell-splitting stage variable data. */
/** Program object id for cell splitting stage. */
GLuint        marching_cubes_cells_program_id                            = 0;
/** Vertex shader id for cell splitting stage. */
GLuint        marching_cubes_cells_vert_shader_id                        = 0;
/** Fragment shader id for cell splitting stage. */
GLuint        marching_cubes_cells_frag_shader_id                        = 0;

/** Name of cells_per_axis uniform. */
const GLchar* marching_cubes_cells_uniform_cells_per_axis_name           = "cells_per_axis";
/** Location of cells_per_axis uniform. */
GLuint        marching_cubes_cells_uniform_cells_per_axis_id             = 0;

/** Name of iso_level uniform. */
const GLchar* marching_cubes_cells_uniform_isolevel_name                 = "iso_level";
/** Location of iso_level uniform. */
GLuint        marching_cubes_cells_uniform_isolevel_id                   = 0;

/** Name of scalar_field uniform. */
const GLchar* marching_cubes_cells_uniform_scalar_field_sampler_name     = "scalar_field";
/** Location of scalar_field uniform. */
GLuint        marching_cubes_cells_uniform_scalar_field_sampler_id       = 0;

/** Cell_type_index output variable's name. */
const GLchar* marching_cubes_cells_varying_name                          = "cell_type_index";

/** Id of transform feedback object to keep cell types buffer binding. */
GLuint        marching_cubes_cells_transform_feedback_object_id          = 0;

/** Id of a buffer object to hold result cell type data. */
GLuint        marching_cubes_cells_types_buffer_id                       = 0;

/** Id of a texture object to hold result cell type data. */
GLuint        marching_cubes_cells_types_texture_object_id               = 0;


/* 4. Marching Cubes algorithm triangle generation and rendering stage variable data. */
/** Program object id for marching cubes algorthim's for rendering stage. */
GLuint        marching_cubes_triangles_program_id                        = 0;
/** Vertex shader id for marching cubes algorthim's for rendering stage. */
GLuint        marching_cubes_triangles_frag_shader_id                    = 0;
/** Fragment shader id for marching cubes algorthim's for rendering stage. */
GLuint        marching_cubes_triangles_vert_shader_id                    = 0;

/** Name of samples_per_axis uniform. */
const GLchar* marching_cubes_triangles_uniform_samples_per_axis_name     = "samples_per_axis";
/** Location of samples_per_axis uniform. */
GLuint        marching_cubes_triangles_uniform_samples_per_axis_id       = 0;

/** Name of iso_level uniform. */
const GLchar* marching_cubes_triangles_uniform_isolevel_name             = "iso_level";
/** Location of iso_level uniform. */
GLuint        marching_cubes_triangles_uniform_isolevel_id               = 0;

/** Name of time uniform. */
const GLchar* marching_cubes_triangles_uniform_time_name                 = "time";
/** Location of time uniform. */
GLuint        marching_cubes_triangles_uniform_time_id                   = 0;

/** Name of mvp uniform. */
const GLchar* marching_cubes_triangles_uniform_mvp_name                  = "mvp";
/** Location of mvp uniform. */
GLuint        marching_cubes_triangles_uniform_mvp_id                    = 0;

/** Name of cell_types uniform. */
const GLchar* marching_cubes_triangles_uniform_cell_types_sampler_name   = "cell_types";
/** Location of cell_types uniform. */
GLuint        marching_cubes_triangles_uniform_cell_types_sampler_id     = 0;

/** Name of scalar_field uniform. */
const GLchar* marching_cubes_triangles_uniform_scalar_field_sampler_name = "scalar_field";
/** Location of scalar_field uniform. */
GLuint        marching_cubes_triangles_uniform_scalar_field_sampler_id   = 0;

/** Name of sphere_positions_uniform_block uniform block. */
const GLchar* marching_cubes_triangles_uniform_sphere_positions_name     = "sphere_positions_uniform_block";
/** Index of sphere_positions_uniform_block uniform block. */
GLuint        marching_cubes_triangles_uniform_sphere_positions_id       = 0;

/** Name of tri_table uniform. */
const GLchar* marching_cubes_triangles_uniform_tri_table_sampler_name    = "tri_table";
/** Location of tri_table uniform. */
GLuint        marching_cubes_triangles_uniform_tri_table_sampler_id      = 0;

/** Id of a texture object to hold triangle look-up table data. */
GLuint        marching_cubes_triangles_lookup_table_texture_id           = 0;

/** Id of vertex array object. */
GLuint        marching_cubes_triangles_vao_id                            = 0;


/** Calculates combined model view and projection matrix.
 *
 *  @param mvp combined mvp matrix
 */
void calc_mvp(Matrix& mvp)
{
    /* Define projection properties. */
    float degreesToRadiansCoefficient = atanf(1) / 45;                            /* Coefficient to recalculate degrees to radians.      */
    float frustum_fovy                = 45.0f;                                    /* 45 degrees field of view in the y direction.        */
    float frustum_aspect              = (float)window_width/(float)window_height; /* Aspect ratio.                                       */
    float frustum_z_near              = 0.01f;                                    /* How close the viewer is to the near clipping plane. */
    float frustum_z_far               = 100.0f;                                   /* How far the viewer is from the far clipping plane.  */
    float camera_distance             = 2.5f;                                     /* Distance from camera to scene center.               */

    /* Matrix that stores temporary matrix data for translation transformations. */
    Matrix mat4_translate  = Matrix::createTranslation(-0.5, -0.5, -0.5);

    /* Matrix that stores temporary matrix data for scale transformations. */
    Matrix mat4_scale      = Matrix::createScaling    ( 2.0,  2.0,  2.0);

    /* Matrix that transforms the vertices from model space to world space. */
    /* Translate and scale coordinates from [0..1] to [-1..1] range for full visibility. */
    Matrix mat4_model_view = mat4_scale * mat4_translate;

    /* Pull the camera back from the scene center. */
    mat4_model_view[14] -= float(camera_distance);

    /* Create the perspective matrix from frustum parameters. */
    Matrix mat4_perspective = Matrix::matrixPerspective(degreesToRadiansCoefficient * frustum_fovy, frustum_aspect, frustum_z_near, frustum_z_far);

    /* MVP (Model View Perspective) matrix is a result of multiplication of Perspective Matrix by Model View Matrix. */
    mvp = mat4_perspective * mat4_model_view;
}


/** Initialises OpenGL ES and model environments.
 *
 *  @param width  window width reported by operating system
 *  @param height window width reported by operating system
 */
void setupGraphics(int width, int height)
{
    /*load shaders*/
    LoadAllShaders();
    
    /* Store window width and height. */
    window_width  = width;
    window_height = height;

    /* Specify one byte alignment for pixels rows in memory for pack and unpack buffers. */
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CHECK(glPixelStorei(GL_PACK_ALIGNMENT,   1));

    /* 1. Calculate sphere positions stage. */
    /* Create sphere updater program object. */
    spheres_updater_program_id = GL_CHECK(glCreateProgram());

    /* Load and compile sphere updater shaders. */
    Shader::processShader(&spheres_updater_vert_shader_id, spheres_updater_vert_shader.c_str(), GL_VERTEX_SHADER  );
    Shader::processShader(&spheres_updater_frag_shader_id, spheres_updater_frag_shader.c_str(), GL_FRAGMENT_SHADER);

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(spheres_updater_program_id, spheres_updater_vert_shader_id));
    GL_CHECK(glAttachShader(spheres_updater_program_id, spheres_updater_frag_shader_id));

    /* [Stage 1 Specifying output variables] */
    /* Specify shader varyings (output variables) we are interested in capturing. */
    GL_CHECK(glTransformFeedbackVaryings(spheres_updater_program_id, 1, &sphere_position_varying_name, GL_SEPARATE_ATTRIBS));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(spheres_updater_program_id));
    /* [Stage 1 Specifying output variables] */

    /* [Stage 1 Specifying input variables] */
    /* Get input uniform location. */
    spheres_updater_uniform_time_id = GL_CHECK(glGetUniformLocation(spheres_updater_program_id, spheres_updater_uniform_time_name));
    /* [Stage 1 Specifying input variables] */

    /* Activate spheres updater program. */
    GL_CHECK(glUseProgram(spheres_updater_program_id));

    /* [Stage 1 Allocate buffer for output values] */
    /* Generate buffer object id. Define required storage space sufficient to hold sphere positions data. */
    GL_CHECK(glGenBuffers(1, &spheres_updater_sphere_positions_buffer_object_id));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, spheres_updater_sphere_positions_buffer_object_id));
    GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, n_spheres * n_sphere_position_components * sizeof(GLfloat), NULL, GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));
    /* [Stage 1 Allocate buffer for output values] */

    /* [Stage 1 Transform feedback object initialization] */
    /* Generate and bind transform feedback object. */
    GL_CHECK(glGenTransformFeedbacks(1, &spheres_updater_transform_feedback_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, spheres_updater_transform_feedback_object_id));

    /* Bind buffers to store calculated sphere positions. */
    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, spheres_updater_sphere_positions_buffer_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));
    /* [Stage 1 Transform feedback object initialization] */

    /* 2. Scalar field generation stage. */
    /* Create scalar field generator program object. */
    scalar_field_program_id = GL_CHECK(glCreateProgram());

    /* Load and compile scalar field generator shaders. */
    Shader::processShader(&scalar_field_vert_shader_id, scalar_field_vert_shader.c_str(), GL_VERTEX_SHADER  );
    Shader::processShader(&scalar_field_frag_shader_id, scalar_field_frag_shader.c_str(), GL_FRAGMENT_SHADER);

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(scalar_field_program_id, scalar_field_vert_shader_id));
    GL_CHECK(glAttachShader(scalar_field_program_id, scalar_field_frag_shader_id));

    /* Specify shader varyings (output variables) we are interested in capturing. */
    GL_CHECK(glTransformFeedbackVaryings(scalar_field_program_id, 1, &scalar_field_value_varying_name, GL_SEPARATE_ATTRIBS));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(scalar_field_program_id));

    /* Get input uniform locations. */
    scalar_field_uniform_samples_per_axis_id = GL_CHECK(glGetUniformLocation  (scalar_field_program_id, scalar_field_uniform_samples_per_axis_name));
    scalar_field_uniform_spheres_id          = GL_CHECK(glGetUniformBlockIndex(scalar_field_program_id, scalar_field_uniform_spheres_name         ));

    /* Activate scalar field generating program. */
    GL_CHECK(glUseProgram(scalar_field_program_id));

    /* Initialize uniforms constant throughout rendering loop. */
    GL_CHECK(glUniform1i(scalar_field_uniform_samples_per_axis_id, samples_per_axis));

    /* Set binding point for uniform block. */
    GL_CHECK(glUniformBlockBinding(scalar_field_program_id, scalar_field_uniform_spheres_id, 0));
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, spheres_updater_sphere_positions_buffer_object_id));

    /* Generate buffer object id. Define required storage space sufficient to hold scalar field data. */
    GL_CHECK(glGenBuffers(1, &scalar_field_buffer_object_id));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, scalar_field_buffer_object_id));

    GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, samples_in_3d_space * sizeof(GLfloat), NULL, GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));

    /* Generate and bind transform feedback object. */
    GL_CHECK(glGenTransformFeedbacks(1, &scalar_field_transform_feedback_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, scalar_field_transform_feedback_object_id));

    /* Bind buffer to store calculated scalar field values. */
    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, scalar_field_buffer_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    /* [Stage 2 Creating texture] */
    /* Generate texture object to hold scalar field data. */
    GL_CHECK(glGenTextures(1, &scalar_field_texture_object_id));

    /* Scalar field uses GL_TEXTURE_3D target of texture unit 1. */
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_3D, scalar_field_texture_object_id));

    /* Prepare texture storage for scalar field values. */
    GL_CHECK(glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, samples_per_axis, samples_per_axis, samples_per_axis));
    /* [Stage 2 Creating texture] */

    /* Tune texture settings to use it as a data source. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL,  0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE));


    /* 3. Marching Cubes cell-splitting stage. */
    /* Create a program object to execute Marching Cubes algorithm cell splitting stage. */
    marching_cubes_cells_program_id = GL_CHECK(glCreateProgram());

    /* Marching cubes algorithm shaders initialisation. */
    Shader::processShader(&marching_cubes_cells_vert_shader_id, marching_cubes_cells_vert_shader.c_str(), GL_VERTEX_SHADER  );
    Shader::processShader(&marching_cubes_cells_frag_shader_id, marching_cubes_cells_frag_shader.c_str(), GL_FRAGMENT_SHADER);

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(marching_cubes_cells_program_id, marching_cubes_cells_vert_shader_id));
    GL_CHECK(glAttachShader(marching_cubes_cells_program_id, marching_cubes_cells_frag_shader_id));

    /* Specify shader varyings (output variables) we are interested in capturing. */
    GL_CHECK(glTransformFeedbackVaryings(marching_cubes_cells_program_id, 1, &marching_cubes_cells_varying_name, GL_SEPARATE_ATTRIBS));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(marching_cubes_cells_program_id));

    /* Get input uniform locations. */
    marching_cubes_cells_uniform_cells_per_axis_id       = GL_CHECK(glGetUniformLocation(marching_cubes_cells_program_id, marching_cubes_cells_uniform_cells_per_axis_name));
    marching_cubes_cells_uniform_scalar_field_sampler_id = GL_CHECK(glGetUniformLocation(marching_cubes_cells_program_id, marching_cubes_cells_uniform_scalar_field_sampler_name));
    marching_cubes_cells_uniform_isolevel_id             = GL_CHECK(glGetUniformLocation(marching_cubes_cells_program_id, marching_cubes_cells_uniform_isolevel_name));

    /* Activate cell-splitting program. */
    GL_CHECK(glUseProgram(marching_cubes_cells_program_id));

    /* Initialize uniforms constant throughout rendering loop. */
    GL_CHECK(glUniform1i(marching_cubes_cells_uniform_cells_per_axis_id,       cells_per_axis  ));
    GL_CHECK(glUniform1f(marching_cubes_cells_uniform_isolevel_id,             isosurface_level));
    GL_CHECK(glUniform1i(marching_cubes_cells_uniform_scalar_field_sampler_id, 1               ));

    /* Generate buffer object id and allocate memory to store scalar field values. */
    GL_CHECK(glGenBuffers(1, &marching_cubes_cells_types_buffer_id));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, marching_cubes_cells_types_buffer_id));
    GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, cells_in_3d_space * sizeof(GLint), NULL, GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));

    /* Generate and bind transform feedback object. */
    GL_CHECK(glGenTransformFeedbacks(1, &marching_cubes_cells_transform_feedback_object_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, marching_cubes_cells_transform_feedback_object_id));

    /* Bind buffer to store calculated cell type data. */
    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, marching_cubes_cells_types_buffer_id));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    /* [Stage 3 Creating texture] */
    /* Generate a texture object to hold cell type data. (We will explain why the texture later). */
    GL_CHECK(glGenTextures(1, &marching_cubes_cells_types_texture_object_id));

    /* Marching cubes cell type data uses GL_TEXTURE_3D target of texture unit 2. */
    GL_CHECK(glActiveTexture(GL_TEXTURE2));
    GL_CHECK(glBindTexture(GL_TEXTURE_3D, marching_cubes_cells_types_texture_object_id));

    /* Prepare texture storage for marching cube cell type data. */
    GL_CHECK(glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32I, cells_per_axis, cells_per_axis, cells_per_axis));
    /* [Stage 3 Creating texture] */

    /* Tune texture settings to use it as a data source. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL,  0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE));


    /* 4. Marching Cubes algorithm triangle generation and rendering stage. */
    /* Create a program object that we will use for triangle generation and rendering stage. */
    marching_cubes_triangles_program_id = GL_CHECK(glCreateProgram());

    /* Initialize shaders for the triangle generation and rendering stage. */
    Shader::processShader(&marching_cubes_triangles_vert_shader_id, marching_cubes_triangles_vert_shader.c_str(), GL_VERTEX_SHADER  );
    Shader::processShader(&marching_cubes_triangles_frag_shader_id, marching_cubes_triangles_frag_shader.c_str(), GL_FRAGMENT_SHADER);

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(marching_cubes_triangles_program_id, marching_cubes_triangles_vert_shader_id));
    GL_CHECK(glAttachShader(marching_cubes_triangles_program_id, marching_cubes_triangles_frag_shader_id));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(marching_cubes_triangles_program_id));

    /* Get input uniform locations. */
    marching_cubes_triangles_uniform_time_id                 = GL_CHECK(glGetUniformLocation  (marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_time_name                ));
    marching_cubes_triangles_uniform_samples_per_axis_id     = GL_CHECK(glGetUniformLocation  (marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_samples_per_axis_name    ));
    marching_cubes_triangles_uniform_isolevel_id             = GL_CHECK(glGetUniformLocation  (marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_isolevel_name            ));
    marching_cubes_triangles_uniform_mvp_id                  = GL_CHECK(glGetUniformLocation  (marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_mvp_name                 ));
    marching_cubes_triangles_uniform_cell_types_sampler_id   = GL_CHECK(glGetUniformLocation  (marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_cell_types_sampler_name  ));
    marching_cubes_triangles_uniform_tri_table_sampler_id    = GL_CHECK(glGetUniformLocation  (marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_tri_table_sampler_name   ));
    marching_cubes_triangles_uniform_scalar_field_sampler_id = GL_CHECK(glGetUniformLocation  (marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_scalar_field_sampler_name));
    marching_cubes_triangles_uniform_sphere_positions_id     = GL_CHECK(glGetUniformBlockIndex(marching_cubes_triangles_program_id, marching_cubes_triangles_uniform_sphere_positions_name    ));

    /* Activate triangle generating and rendering program. */
    GL_CHECK(glUseProgram(marching_cubes_triangles_program_id));

    /* Initialize model view projection matrix. */
    calc_mvp(mvp);

    /* Initialize uniforms constant throughout rendering loop. */
    GL_CHECK(glUniform1f(marching_cubes_triangles_uniform_isolevel_id,             isosurface_level));
    GL_CHECK(glUniform1i(marching_cubes_triangles_uniform_samples_per_axis_id,     samples_per_axis));
    GL_CHECK(glUniform1i(marching_cubes_triangles_uniform_tri_table_sampler_id,    4               ));
    GL_CHECK(glUniform1i(marching_cubes_triangles_uniform_cell_types_sampler_id,   2               ));
    GL_CHECK(glUniform1i(marching_cubes_triangles_uniform_scalar_field_sampler_id, 1               ));
    GL_CHECK(glUniformMatrix4fv(marching_cubes_triangles_uniform_mvp_id, 1, GL_FALSE, mvp.getAsArray()));

    /* Allocate memory for buffer */
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, spheres_updater_sphere_positions_buffer_object_id));

    /* Generate an Id for a texture object to hold look-up array data (tri_table). */
    GL_CHECK(glGenTextures(1, &marching_cubes_triangles_lookup_table_texture_id));

    /* Lookup array (tri_table) uses GL_TEXTURE_2D target of texture unit 4. */
    GL_CHECK(glActiveTexture(GL_TEXTURE4));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, marching_cubes_triangles_lookup_table_texture_id));

    /* Tune texture settings to use it as a data source. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));

    /* Load lookup table (tri_table) into texture. */
    GL_CHECK(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, mc_vertices_per_cell, mc_cells_types_count));
    GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,        /* Use texture bound to GL_TEXTURE_2D               */
                             0,                    /* Base image level                                 */
                             0,                    /* From the texture origin                          */
                             0,                    /* From the texture origin                          */
                             mc_vertices_per_cell, /* Width will represent vertices in all 5 triangles */
                             mc_cells_types_count, /* Height will represent cell type                  */
                             GL_RED_INTEGER,       /* Texture will have only one component             */
                             GL_INT,               /* ... of type int                                  */
                             tri_table             /* Data will be copied directly from tri_table      */
                            ));

    /* Generate a vertex array object. We'll go with the explanation later. */
    GL_CHECK(glGenVertexArrays(1, &marching_cubes_triangles_vao_id));

    /* In OpenGL ES, draw calls require a bound vertex array object.
     * Even though we're not using any per-vertex attribute data, we still need to bind a vertex array object.
     */
    GL_CHECK(glBindVertexArray(marching_cubes_triangles_vao_id));

    /* Enable facet culling, depth testing and specify front face for polygons. */
    GL_CHECK(glEnable   (GL_DEPTH_TEST));
    GL_CHECK(glEnable   (GL_CULL_FACE ));
    GL_CHECK(glFrontFace(GL_CW        ));

    /* Start counting time. */
    timer.reset();
}

/** Draws one frame. */
void renderFrame(void)
{
    /* Update time. */
    model_time = timer.getTime();

    /*
     * Rendering section
     */
    /* Clear the buffers that we are going to render to in a moment. */
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* [Stage 1 Calculate sphere positions stage] */
    /* 1. Calculate sphere positions stage.
     *
     * At this stage we calculate new sphere positions in space
     * according to current time moment.
     */
    /* [Stage 1 Bind buffers to store calculated sphere position values] */
    /* Bind buffers to store calculated sphere position values. */
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, spheres_updater_transform_feedback_object_id));
    /* [Stage 1 Bind buffers to store calculated sphere position values] */

    /* [Stage 1 Enable GL_RASTERIZER_DISCARD] */
    /* Shorten GL pipeline: we will use vertex shader only. */
    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    /* [Stage 1 Enable GL_RASTERIZER_DISCARD] */
    {
        /* Select program for sphere positions generation stage. */
        GL_CHECK(glUseProgram(spheres_updater_program_id));

        /* [Stage 1 Specify input arguments to vertex shader] */
        /* Specify input arguments to vertex shader. */
        GL_CHECK(glUniform1f(spheres_updater_uniform_time_id, model_time));
        /* [Stage 1 Specify input arguments to vertex shader] */

        /* [Stage 1 Activate transform feedback mode] */
        /* Activate transform feedback mode. */
        GL_CHECK(glBeginTransformFeedback(GL_POINTS));
        /* [Stage 1 Activate transform feedback mode] */
        {
            /* [Stage 1 Execute n_spheres times vertex shader] */
            /* Run sphere positions calculation. */
            GL_CHECK(glDrawArrays(GL_POINTS, 0, n_spheres));
            /* [Stage 1 Execute n_spheres times vertex shader] */
        }
        /* [Stage 1 Deactivate transform feedback mode] */
        GL_CHECK(glEndTransformFeedback());
        /* [Stage 1 Deactivate transform feedback mode] */
    }
    /* [Stage 1 Disable GL_RASTERIZER_DISCARD] */
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));
    /* [Stage 1 Disable GL_RASTERIZER_DISCARD] */

    /* Unbind buffers used at this stage. */
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));
    /* [Stage 1 Calculate sphere positions stage] */


    /* [Stage 2 Scalar field generation stage] */
    /* 2. Scalar field generation stage.
     *
     * At this stage we calculate scalar field and store it in buffer
     * and later copy from buffer to texture.
     */
    /* Bind sphere positions data buffer to GL_UNIFORM_BUFFER. */
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, spheres_updater_sphere_positions_buffer_object_id));

    /* Bind buffer object to store calculated scalar field values. */
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, scalar_field_transform_feedback_object_id));

    /* Shorten GL pipeline: we will use vertex shader only. */
    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    {
        /* Select program for scalar field generation stage. */
        GL_CHECK(glUseProgram(scalar_field_program_id));

        /* Activate transform feedback mode. */
        GL_CHECK(glBeginTransformFeedback(GL_POINTS));
        {
            /* Run scalar field calculation for all vertices in space. */
            GL_CHECK(glDrawArrays(GL_POINTS, 0, samples_in_3d_space));
        }
        GL_CHECK(glEndTransformFeedback());
    }
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));

    /* Unbind buffers used at this stage. */
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));
    /* [Stage 2 Scalar field generation stage] */

    /* Copy scalar field values from buffer into texture bound to target GL_TEXTURE_3D of texture unit 1.
     * We need to move this data to a texture object, as there is no way we could access data
     * stored within a buffer object in an OpenGL ES 3.0 shader.
     */
    /* [Stage 2 Scalar field generation stage move data to texture] */
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindBuffer   (GL_PIXEL_UNPACK_BUFFER, scalar_field_buffer_object_id));
    GL_CHECK(glTexSubImage3D(GL_TEXTURE_3D,    /* Use texture bound to GL_TEXTURE_3D                                     */
                             0,                /* Base image level                                                       */
                             0,                /* From the texture origin                                                */
                             0,                /* From the texture origin                                                */
                             0,                /* From the texture origin                                                */
                             samples_per_axis, /* Texture have same width as scalar field in buffer                      */
                             samples_per_axis, /* Texture have same height as scalar field in buffer                     */
                             samples_per_axis, /* Texture have same depth as scalar field in buffer                      */
                             GL_RED,           /* Scalar field gathered in buffer has only one component                 */
                             GL_FLOAT,         /* Scalar field gathered in buffer is of float type                       */
                             NULL              /* Scalar field gathered in buffer bound to GL_PIXEL_UNPACK_BUFFER target */
                            ));
    /* [Stage 2 Scalar field generation stage move data to texture] */


    /* 3. Marching cube algorithm cell splitting stage.
     *
     * At this stage we analyze isosurface in each cell of space and
     * assign one of 256 possible types to each cell. Cell type data
     * for each cell is stored in attached buffer.
     */
    /* Bind buffer to store cell type data. */
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, marching_cubes_cells_transform_feedback_object_id));

    /* Shorten GL pipeline: we will use vertex shader only. */
    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    {
        /* Select program for Marching Cubes algorthim's cell splitting stage. */
        GL_CHECK(glUseProgram(marching_cubes_cells_program_id));

        /* Activate transform feedback mode. */
        GL_CHECK(glBeginTransformFeedback(GL_POINTS));
        {
            /* [Stage 3 Execute vertex shader] */
            /* Run Marching Cubes algorithm cell splitting stage for all cells. */
            GL_CHECK(glDrawArrays(GL_POINTS, 0, cells_in_3d_space));
            /* [Stage 3 Execute vertex shader] */
        }
        GL_CHECK(glEndTransformFeedback());
    }
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));

    /* Unbind buffers used at this stage. */
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    /* Copy data from buffer into texture bound to target GL_TEXTURE2 in texture unit 2.
     * We need to move this data to a texture object, as there is no way we could access data
     * stored within a buffer object in a OpenGL ES 3.0 shader.
     */
    GL_CHECK(glActiveTexture(GL_TEXTURE2));
    GL_CHECK(glBindBuffer   (GL_PIXEL_UNPACK_BUFFER, marching_cubes_cells_types_buffer_id));
    GL_CHECK(glTexSubImage3D(GL_TEXTURE_3D,  /* Use texture bound to GL_TEXTURE_3D                                   */
                             0,              /* Base image level                                                     */
                             0,              /* From the texture origin                                              */
                             0,              /* From the texture origin                                              */
                             0,              /* From the texture origin                                              */
                             cells_per_axis, /* Texture have same width as cells by width in buffer                  */
                             cells_per_axis, /* Texture have same height as cells by height in buffer                */
                             cells_per_axis, /* Texture have same depth as cells by depth in buffer                  */
                             GL_RED_INTEGER, /* Cell types gathered in buffer have only one component                */
                             GL_INT,         /* Cell types gathered in buffer are of int type                        */
                             NULL            /* Cell types gathered in buffer bound to GL_PIXEL_UNPACK_BUFFER target */
                            ));


    /* 4. Marching Cubes algorithm triangle generation stage.
     *
     * At this stage, we render exactly (3 vertices * 5 triangles per cell *
     * amount of cells the scalar field is split to) triangle vertices.
     * Then render triangularized geometry.
     */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));

    /* Activate triangle generating and rendering program. */
    GL_CHECK(glUseProgram(marching_cubes_triangles_program_id));

    /* Specify input arguments to vertex shader. */
    GL_CHECK(glUniform1f(marching_cubes_triangles_uniform_time_id, model_time));

    /* [Stage 4 Run triangle generating and rendering program] */
    /* Run triangle generating and rendering program. */
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, cells_in_3d_space * triangles_per_cell * vertices_per_triangle));
    /* [Stage 4 Run triangle generating and rendering program] */
}

/** Deinitialises OpenGL ES environment. */
void cleanup()
{
    GL_CHECK(glDeleteVertexArrays      (1, &marching_cubes_triangles_vao_id                  ));
    GL_CHECK(glDeleteShader            (    marching_cubes_triangles_frag_shader_id          ));
    GL_CHECK(glDeleteShader            (    marching_cubes_triangles_vert_shader_id          ));
    GL_CHECK(glDeleteProgram           (    marching_cubes_triangles_program_id              ));
    GL_CHECK(glDeleteTextures          (1, &marching_cubes_triangles_lookup_table_texture_id ));
    GL_CHECK(glDeleteTextures          (1, &marching_cubes_cells_types_texture_object_id     ));
    GL_CHECK(glDeleteTransformFeedbacks(1, &marching_cubes_cells_transform_feedback_object_id));
    GL_CHECK(glDeleteBuffers           (1, &marching_cubes_cells_types_buffer_id             ));
    GL_CHECK(glDeleteShader            (    marching_cubes_cells_frag_shader_id              ));
    GL_CHECK(glDeleteShader            (    marching_cubes_cells_vert_shader_id              ));
    GL_CHECK(glDeleteProgram           (    marching_cubes_cells_program_id                  ));
    GL_CHECK(glDeleteTextures          (1, &scalar_field_texture_object_id                   ));
    GL_CHECK(glDeleteTransformFeedbacks(1, &scalar_field_transform_feedback_object_id        ));
    GL_CHECK(glDeleteBuffers           (1, &scalar_field_buffer_object_id                    ));
    GL_CHECK(glDeleteShader            (    scalar_field_frag_shader_id                      ));
    GL_CHECK(glDeleteShader            (    scalar_field_vert_shader_id                      ));
    GL_CHECK(glDeleteProgram           (    scalar_field_program_id                          ));
    GL_CHECK(glDeleteTransformFeedbacks(1, &spheres_updater_transform_feedback_object_id     ));
    GL_CHECK(glDeleteBuffers           (1, &spheres_updater_sphere_positions_buffer_object_id));
    GL_CHECK(glDeleteShader            (    spheres_updater_frag_shader_id                   ));
    GL_CHECK(glDeleteShader            (    spheres_updater_vert_shader_id                   ));
    GL_CHECK(glDeleteProgram           (    spheres_updater_program_id                       ));
}



