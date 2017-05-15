#include "ShaderCode.h"

/* 3. Marching Cubes cell-splitting stage variable data. */

GLuint        marching_cubes_cells_program_id                            = 0;
GLuint        marching_cubes_cells_vert_shader_id                        = 0;
GLuint        marching_cubes_cells_frag_shader_id                        = 0;

const GLchar* marching_cubes_cells_uniform_cells_per_axis_name           = "cells_per_axis";
GLuint        marching_cubes_cells_uniform_cells_per_axis_id             = 0;

const GLchar* marching_cubes_cells_uniform_isolevel_name                 = "iso_level";
GLuint        marching_cubes_cells_uniform_isolevel_id                   = 0;

const GLchar* marching_cubes_cells_uniform_scalar_field_sampler_name     = "scalar_field";
GLuint        marching_cubes_cells_uniform_scalar_field_sampler_id       = 0;

const GLchar* marching_cubes_cells_varying_name                          = "cell_type_index";

GLuint        marching_cubes_cells_transform_feedback_object_id          = 0;

GLuint        marching_cubes_cells_types_buffer_id                       = 0;

GLuint        marching_cubes_cells_types_texture_object_id               = 0;


const GLchar* marching_cubes_cells_vert_shader = GLSL(

	/** Specify low precision for sampler3D type. */
	precision lowp sampler3D;

	/* Uniforms: */
	/** Scalar field is stored in a 3D texture. */
	uniform sampler3D scalar_field;

	/** Amount of samples taken for each axis of a scalar field. */
	uniform int cells_per_axis;

	/** Isosurface level. */
	uniform float iso_level;

	/* Output data: */
	/** Cell type index. */
	flat out int cell_type_index;

	/** Calculates cell type index for provided cell and isosurface level.
	 *
	 *  @param cell_corner_field_value Scalar field values in cell corners
	 *  @param isolevel                Scalar field value which defines isosurface level
	 */
	/* [Stage 3 get_cell_type_index] */
	int get_cell_type_index(in float cell_corner_field_value[8], in float isolevel)
	{
	    int cell_type_index = 0;

	    /* Iterate through all cell corners. */
	    for (int i = 0; i < 8; i++)
	    {
	        /* If corner is inside isosurface then set bit in cell type index index. */
	        if (cell_corner_field_value[i] < isolevel)
	        {
	            /* Set appropriate corner bit in cell type index. */
	            cell_type_index |= (1<<i);
	        }
	    }

	    return cell_type_index;
	}
	/* [Stage 3 get_cell_type_index] */

	/** Decode coordinates in space from cell number.
	 *  Assume cubical space of cells_per_axis cells length by each axis and following encoding:
	 *  encoded_position = x + y * cells_per_axis + z * cells_per_axis * cells_per_axis
	 *
	 *  @param  cell_index Encoded cell position
	 *  @return            Coordinates of a cell in space ranged [0 .. cells_per_axis-1]
	 */
	/* [Stage 3 decode_space_position] */
	ivec3 decode_space_position(in int cell_index)
	{
	    ivec3 space_position;
	    int   encoded_position = cell_index;

	    /* Calculate coordinates from encoded position */
	    space_position.x       = encoded_position % cells_per_axis;
	    encoded_position       = encoded_position / cells_per_axis;

	    space_position.y       = encoded_position % cells_per_axis;
	    encoded_position       = encoded_position / cells_per_axis;

	    space_position.z       = encoded_position;

	    return space_position;
	}
	/* [Stage 3 decode_space_position] */

	/** Shader entry point. */
	void main()
	{
	    /* Cubic cell has exactly 8 corners. */
	    const int corners_in_cell = 8;

	    /* Cell corners in space relatively to cell's base point [0]. */
	    const ivec3 cell_corners_offsets[corners_in_cell] = ivec3[]
	    (
	        ivec3(0, 0, 0),
	        ivec3(1, 0, 0),
	        ivec3(1, 0, 1),
	        ivec3(0, 0, 1),
	        ivec3(0, 1, 0),
	        ivec3(1, 1, 0),
	        ivec3(1, 1, 1),
	        ivec3(0, 1, 1)
	    );

	    /* Scalar field texture size, used for normalization purposes. */
	    vec3 scalar_field_normalizers = vec3(textureSize(scalar_field, 0)) - vec3(1, 1, 1);

	    /* Scalar field value in corners. Corners numbered according to Marching Cubes algorithm. */
	    float scalar_field_in_cell_corners[8];

	    /* Find cell position processed by this shader instance (defined by gl_VertexID). */
	    ivec3 space_position = decode_space_position(gl_VertexID);

	    /* [Stage 3 Gather values for the current cell] */
	    /* Find scalar field values in cell corners. */
	    for (int i = 0; i < corners_in_cell; i++)
	    {
	        /* Calculate cell corner processed at this iteration. */
	        ivec3 cell_corner = space_position + cell_corners_offsets[i];

	        /* Calculate cell corner's actual position ([0.0 .. 1.0] range.) */
	        vec3 normalized_cell_corner  = vec3(cell_corner) / scalar_field_normalizers;

	        /* Get scalar field value in cell corner from scalar field texture. */
	        scalar_field_in_cell_corners[i] = textureLod(scalar_field, normalized_cell_corner, 0.0).r;
	    }
	    /* [Stage 3 Gather values for the current cell] */

	    /* Get cube type index. */
	    cell_type_index = get_cell_type_index(scalar_field_in_cell_corners, iso_level);
	}
);

const GLchar* marching_cubes_cells_frag_shader = GLSL(

	/** Shader entry point. */
	void main()
	{
	}
);

void SetUpMarchingCubesCells(){
	/* 3. Marching Cubes cell-splitting stage. */
    /* Create a program object to execute Marching Cubes algorithm cell splitting stage. */
    marching_cubes_cells_program_id = GL_CHECK(glCreateProgram());

    /* Marching cubes algorithm shaders initialisation. */
    Shader::processShader(&marching_cubes_cells_vert_shader_id, marching_cubes_cells_vert_shader, GL_VERTEX_SHADER  );
    Shader::processShader(&marching_cubes_cells_frag_shader_id, marching_cubes_cells_frag_shader, GL_FRAGMENT_SHADER);

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

}

void DrawMarchingCubesCells(){
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

}

void CleanUpMarchingCubesCells(){
	GL_CHECK(glDeleteTextures          (1, &marching_cubes_cells_types_texture_object_id     ));
    GL_CHECK(glDeleteTransformFeedbacks(1, &marching_cubes_cells_transform_feedback_object_id));
    GL_CHECK(glDeleteBuffers           (1, &marching_cubes_cells_types_buffer_id             ));
    GL_CHECK(glDeleteShader            (    marching_cubes_cells_frag_shader_id              ));
    GL_CHECK(glDeleteShader            (    marching_cubes_cells_vert_shader_id              ));
    GL_CHECK(glDeleteProgram           (    marching_cubes_cells_program_id                  ));
}