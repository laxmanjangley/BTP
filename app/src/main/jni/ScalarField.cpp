#include "ShaderCode.h"

/* 2. Scalar field generation stage variable data. */
GLuint        scalar_field_program_id                                    = 0;
GLuint        scalar_field_vert_shader_id                                = 0;
GLuint        scalar_field_frag_shader_id                                = 0;

GLuint        scalar_field_buffer_object_id                              = 0;

GLuint        scalar_field_transform_feedback_object_id                  = 0;

const GLchar* scalar_field_uniform_samples_per_axis_name                 = "samples_per_axis";
GLuint        scalar_field_uniform_samples_per_axis_id                   = 0;

const GLchar* scalar_field_uniform_spheres_name                          = "spheres_uniform_block";
GLuint        scalar_field_uniform_spheres_id                            = 0;

const GLchar* scalar_field_value_varying_name                            = "scalar_field_value";

GLuint        scalar_field_texture_object_id                             = 0;


const GLchar* scalar_field_vert_shader = GLSL(
	/** Precision to avoid division-by-zero errors. */
	const float EPSILON = 0.000001f;

	/** Amount of spheres defining scalar field. This value should be synchronized between all files. */
	const int N_SPHERES = 3;

	/* [Stage 2 Uniforms] */
	/* Uniforms: */
	/** Amount of samples taken for each axis of a scalar field; */
	uniform int samples_per_axis;

	/** Uniform block encapsulating sphere locations. */
	uniform spheres_uniform_block
	{
	    vec4 input_spheres[N_SPHERES];
	};
	/* [Stage 2 Uniforms] */

	/* [Stage 2 Output data] */
	/* Output data: */
	/** Calculated scalar field value. */
	out vec4 scalar_field_value;
	/* [Stage 2 Output data] */

	/* [Stage 2 decode_space_position] */
	/** Decode coordinates in space from vertex number.
	 *  Assume 3D space of samples_per_axis length for each axis and following encoding:
	 *  encoded_position = x + y * samples_per_axis + z * samples_per_axis * samples_per_axis
	 *
	 *  @param  vertex_index Encoded vertex position
	 *  @return              Coordinates of a vertex in space ranged [0 .. samples_per_axis-1]
	 */
	ivec3 decode_space_position(in int vertex_index)
	{
	    int   encoded_position = vertex_index;
	    ivec3 space_position;

	    /* Calculate coordinates from vertex number. */
	    space_position.x = encoded_position % samples_per_axis;
	    encoded_position = encoded_position / samples_per_axis;

	    space_position.y = encoded_position % samples_per_axis;
	    encoded_position = encoded_position / samples_per_axis;

	    space_position.z = encoded_position;

	    return space_position;
	}
	/* [Stage 2 decode_space_position] */

	/** Normalizes each coordinate interpolating input coordinates
	 *  from range [0 .. samples_per_axis-1] to [0.0 .. 1.0] range.
	 *
	 *  @param  space_position Coordinates in range [0 .. samples_per_axis-1]
	 *  @return Coordinates in range [0.0 .. 1.0]
	 */
	/* [Stage 2 normalize_space_position_coordinates] */
	vec3 normalize_space_position_coordinates(in ivec3 space_position)
	{
	    vec3 normalized_space_position = vec3(space_position) / float(samples_per_axis - 1);

	    return normalized_space_position;
	}
	/* [Stage 2 normalize_space_position_coordinates] */

	/** Calculates scalar field at user-defined location.
	 *
	 *  @param position Space position for which scalar field value is calculated
	 *  @return         Scalar field value
	 */
	/* [Stage 2 calculate_scalar_field_value] */
	vec4 calculate_scalar_field_value(in vec3 position)
	{
	    float field_value = 0.0f;
        vec3 normal = vec3(0.0f);

	    /* Field value in given space position influenced by all spheres. */
	    for (int i = 0; i < N_SPHERES; i++)
	    {
	        vec3  sphere_position         = input_spheres[i].xyz;
	        float vertex_sphere_distance  = length(distance(sphere_position, position));

	        /* Field value is a sum of all spheres fields in a given space position.
	         * Sphere weight (or charge) is stored in w-coordinate.
	         */
	        field_value += input_spheres[i].w / pow(max(EPSILON, vertex_sphere_distance), 2.0);
            normal += normalize(sphere_position-position) * input_spheres[i].w / pow(max(EPSILON, vertex_sphere_distance), 3.0);
	    }

	    return vec4(field_value,normal);
	}
	/* [Stage 2 calculate_scalar_field_value] */

	/** Shader entry point. */
	void main()
	{
	    /* Decode point space position defined by gl_VertexID. */
	    ivec3 space_position      = decode_space_position(gl_VertexID);

	    /* Normalize point space position. */
	    vec3  normalized_position = normalize_space_position_coordinates(space_position);

	    /* Calculate field value and assign field value to output variable. */
	    scalar_field_value = calculate_scalar_field_value(normalized_position);
	}
);

const GLchar* scalar_field_frag_shader= GLSL(

	/** Shader entry point. */
	void main()
	{
	}
);

void SetUpScalarField(){
	    /* 2. Scalar field generation stage. */
    /* Create scalar field generator program object. */
    scalar_field_program_id = GL_CHECK(glCreateProgram());

    /* Load and compile scalar field generator shaders. */
    Shader::processShader(&scalar_field_vert_shader_id, scalar_field_vert_shader, GL_VERTEX_SHADER  );
    Shader::processShader(&scalar_field_frag_shader_id, scalar_field_frag_shader, GL_FRAGMENT_SHADER);

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

    GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, samples_in_3d_space * 4 * sizeof(GL_FLOAT), NULL, GL_STATIC_DRAW));
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
    GL_CHECK(glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, samples_per_axis, samples_per_axis, samples_per_axis));
    /* [Stage 2 Creating texture] */

    /* Tune texture settings to use it as a data source. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST      ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL,  0               ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE));
}

void DrawScalarField(){
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
                             GL_RGBA,          /* Scalar field gathered in buffer has only one component                 */
                             GL_FLOAT,         /* Scalar field gathered in buffer is of float type                       */
                             NULL              /* Scalar field gathered in buffer bound to GL_PIXEL_UNPACK_BUFFER target */
                            ));
    /* [Stage 2 Scalar field generation stage move data to texture] */

}

void CleanUpScalarField(){
    GL_CHECK(glDeleteTextures          (1, &scalar_field_texture_object_id                   ));
    GL_CHECK(glDeleteTransformFeedbacks(1, &scalar_field_transform_feedback_object_id        ));
    GL_CHECK(glDeleteBuffers           (1, &scalar_field_buffer_object_id                    ));
    GL_CHECK(glDeleteShader            (    scalar_field_frag_shader_id                      ));
    GL_CHECK(glDeleteShader            (    scalar_field_vert_shader_id                      ));
    GL_CHECK(glDeleteProgram           (    scalar_field_program_id                          ));
}