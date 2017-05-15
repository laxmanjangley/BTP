#include "ShaderCode.h"

/* 1. Calculate sphere positions stage variable data. */

GLuint        spheres_updater_program_id                                 = 0;
GLuint        spheres_updater_vert_shader_id                             = 0;
GLuint        spheres_updater_frag_shader_id                             = 0;

GLuint        spheres_updater_sphere_positions_buffer_object_id          = 0;

GLuint        spheres_updater_transform_feedback_object_id               = 0;

const GLchar* spheres_updater_uniform_time_name                          = "time";
GLuint        spheres_updater_uniform_time_id                            = 0;

const GLchar* sphere_position_varying_name                               = "sphere_position";


const GLchar* spheres_updater_vert_shader =GLSL(

	/** Structure that describes parameters of a single sphere moving across the scalar field. */
	struct sphere_descriptor
	{
	    /* Coefficients for Lissajou equations. Current coordinates calculated by formula:
	     * v(t) = start_center + lissajou_amplitude * sin(lissajou_frequency * t + lissajou_phase) */
	    vec3  start_center;        /* Center in space around which sphere moves.  */
	    vec3  lissajou_amplitude;  /* Lissajou equation amplitudes for all axes.  */
	    vec3  lissajou_frequency;  /* Lissajou equation frequencies for all axes. */
	    vec3  lissajou_phase;      /* Lissajou equation phases for all axes.      */
	    /* Other sphere parameters. */
	    float size;                /* Size of a sphere (weight or charge).        */
	};

	/* [Stage 1 Uniforms] */
	/** Current time moment. */
	uniform float time;
	/* [Stage 1 Uniforms] */

	/* [Stage 1 Output data] */
	/** Calculated sphere positions. */
	out vec4 sphere_position;
	/* [Stage 1 Output data] */

	/** Shader entry point. */
	void main()
	{
	    /* Stores information on spheres moving across the scalar field. Specified in model coordinates (range 0..1]) */
	    sphere_descriptor spheres[] = sphere_descriptor[]
	    (
	        /*                      (---- center ----)      (--- amplitude --)      (--- frequency ---)      (----- phase -----) (weight)*/
	        sphere_descriptor(  vec3(0.50, 0.50, 0.50), vec3(0.20, 0.25, 0.25), vec3( 11.0, 21.0, 31.0), vec3( 30.0, 45.0, 90.0),  0.100),
	        sphere_descriptor(  vec3(0.50, 0.50, 0.50), vec3(0.25, 0.20, 0.25), vec3( 22.0, 32.0, 12.0), vec3( 45.0, 90.0,120.0),  0.050),
	        sphere_descriptor(  vec3(0.50, 0.50, 0.50), vec3(0.25, 0.25, 0.20), vec3( 33.0, 13.0, 23.0), vec3( 90.0,120.0,150.0),  0.250)
	    );

	    /* Calculate new xyz coordinates of the sphere. */
	    vec3 sphere_position3 = spheres[gl_VertexID].start_center
	                          + spheres[gl_VertexID].lissajou_amplitude
	                          * sin(radians(spheres[gl_VertexID].lissajou_frequency) * time + radians(spheres[gl_VertexID].lissajou_phase));

	    /* Update sphere position coordinates. w-coordinte represents sphere weight. */
	    sphere_position = vec4(sphere_position3, spheres[gl_VertexID].size);
	}

);

const GLchar* spheres_updater_frag_shader = GLSL(

	/** Shader entry point. */
	void main()
	{
	}

);

void SetUpSphereUpdater(){
	/* 1. Calculate sphere positions stage. */
    /* Create sphere updater program object. */
    spheres_updater_program_id = GL_CHECK(glCreateProgram());

    /* Load and compile sphere updater shaders. */
    Shader::processShader(&spheres_updater_vert_shader_id, spheres_updater_vert_shader, GL_VERTEX_SHADER  );
    Shader::processShader(&spheres_updater_frag_shader_id, spheres_updater_frag_shader, GL_FRAGMENT_SHADER);

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
}

void DrawSphereUpdater(){

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
}

void CleanUpSphereUpdater(){
    GL_CHECK(glDeleteTransformFeedbacks(1, &spheres_updater_transform_feedback_object_id     ));
    GL_CHECK(glDeleteBuffers           (1, &spheres_updater_sphere_positions_buffer_object_id));
    GL_CHECK(glDeleteShader            (    spheres_updater_frag_shader_id                   ));
    GL_CHECK(glDeleteShader            (    spheres_updater_vert_shader_id                   ));
    GL_CHECK(glDeleteProgram           (    spheres_updater_program_id                       ));
}