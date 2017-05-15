#include "ShaderCode.h"

/* 4. Marching Cubes algorithm triangle generation and rendering stage variable data. */
GLuint        marching_cubes_triangles_program_id                        = 0;
GLuint        marching_cubes_triangles_frag_shader_id                    = 0;
GLuint        marching_cubes_triangles_vert_shader_id                    = 0;

const GLchar* marching_cubes_triangles_uniform_samples_per_axis_name     = "samples_per_axis";
GLuint        marching_cubes_triangles_uniform_samples_per_axis_id       = 0;

const GLchar* marching_cubes_triangles_uniform_isolevel_name             = "iso_level";
GLuint        marching_cubes_triangles_uniform_isolevel_id               = 0;

const GLchar* marching_cubes_triangles_uniform_time_name                 = "time";
GLuint        marching_cubes_triangles_uniform_time_id                   = 0;

const GLchar* marching_cubes_triangles_uniform_mvp_name                  = "mvp";
GLuint        marching_cubes_triangles_uniform_mvp_id                    = 0;

const GLchar* marching_cubes_triangles_uniform_cell_types_sampler_name   = "cell_types";
GLuint        marching_cubes_triangles_uniform_cell_types_sampler_id     = 0;

const GLchar* marching_cubes_triangles_uniform_scalar_field_sampler_name = "scalar_field";
GLuint        marching_cubes_triangles_uniform_scalar_field_sampler_id   = 0;

const GLchar* marching_cubes_triangles_uniform_sphere_positions_name     = "sphere_positions_uniform_block";
GLuint        marching_cubes_triangles_uniform_sphere_positions_id       = 0;

const GLchar* marching_cubes_triangles_uniform_tri_table_sampler_name    = "tri_table";
GLuint        marching_cubes_triangles_uniform_tri_table_sampler_id      = 0;

GLuint        marching_cubes_triangles_lookup_table_texture_id           = 0;
GLuint        marching_cubes_triangles_vao_id                            = 0;


const GLchar* marching_cubes_triangles_vert_shader = GLSL(

	precision highp isampler2D; /**< Specify high precision for isampler2D type. */
	precision highp isampler3D; /**< Specify high precision for isampler3D type. */
	precision highp sampler2D;  /**< Specify high precision for sampler2D type. */
	precision highp sampler3D;  /**< Specify high precision for sampler3D type. */



	/** Maximum amount of vertices a single cell can define. */
	const int mc_vertices_per_cell = 15;

	/* Uniforms: */
	/** Amount of samples taken for each axis of a scalar field. */
	uniform int samples_per_axis;

	/** A signed integer 3D texture is used to deliver cell type data. */
	uniform isampler3D cell_types;

	/** A 3D texture is used to deliver scalar field data. */
	uniform sampler3D scalar_field;

	/** A 2D texture representing tri_table lookup array. Array contains edge numbers (in sense of Marching Cubes algorithm).
	    As input parameters (indices to texture) should be specified cell type index and combined vertex-triangle number. */
	uniform isampler2D tri_table;

	/** Combined model view and projection matrices. */
	uniform mat4 mvp;

	/** Isosurface level. */
	uniform float iso_level;

	/* Phong shading output variables for fragment shader. */
	out vec4 phong_vertex_position;      /**< position of the vertex in world space.  */
	out vec3 phong_vertex_normal_vector; /**< surface normal vector in world space.   */
	out vec3 phong_vertex_color;         /**< vertex color for fragment colorisation. */

	/** Precision to avoid division-by-zero errors. */
	const float EPSILON = 0.000001f;

	/** Amount of cells taken for each axis of a scalar field. */
	
	/** Function approximates scalar field derivative along begin_vertex<->end_vertex axis.
	 *  Field derivative calculated as a scalar field difference between specified vertices
	 *  divided by distance between vertices.
	 *
	 *  @param begin_vertex begin vertex
	 *  @param end_vertex   end vertex
	 *  @return             scalar field derivative along begin_vertex<->end_vertex axis
	 */
	float calc_partial_derivative(vec3 begin_vertex, vec3 end_vertex)
	{
	    float field_value_begin = textureLod(scalar_field, begin_vertex, 0.0).r;
	    float field_value_end   = textureLod(scalar_field, end_vertex,   0.0).r;

	    return (field_value_end - field_value_begin) / distance(begin_vertex, end_vertex);
	}

	/** Finds normal in given cell corner vertex. Normal calculated as a vec3(dF/dx, dF/dy, dF/dz)
	 *  dFs are calculated as difference of scalar field values in corners of this or adjacent cells.
	 *
	 *  @param p1 vertex for which normal is to be calculated
	 *  @return   normal vector to surface in p1
	 */
	vec3 calc_cell_corner_normal(in vec3 p1)
    {
        vec3 normal = textureLod(scalar_field, p1, 0.0).gba;
        if(length(normal) < 0.00001){
        	return vec3(0);
        }
        return normalize(normal);
    }

	/** Calculates normal for an edge vertex like in an orignal SIGGRAPH paper.
	 *  First finds normal vectors in edge begin vertex and in edge end vertex, then interpolate.
	 *
	 *  @param start_vertex_portion influence of edge_start vertex
	 *  @param edge_start           normalized coordinates of edge start vertex
	 *  @param edge_end             normalized coordinates of edge end vertex
	 *  @return                     normal to surface vector in edge position specified
	 */
	vec3 calc_phong_normal(in float start_vertex_portion, in vec3 edge_start, in vec3 edge_end)
	{
	    /* Find normal vector in begin vertex of the edge. */
	    vec3 edge_start_normal = calc_cell_corner_normal(edge_start);
	    /* Find normal vector in end vertex of the edge. */
	    vec3 edge_end_normal   = calc_cell_corner_normal(edge_end);
	    if(length(edge_start_normal) < 0.00001){
	    	return edge_end_normal;
	    }

	    if(length(edge_end_normal) < 0.00001){
	    	return edge_start_normal;
	    }
	    /* Interpolate normal vector. */
	    return mix(edge_end_normal, edge_start_normal, start_vertex_portion);
	}

	/** Decodes cell coordinates from vertex identifier.
	 *  Assumes 3D space of CELLS_PER_AXIS cells for each axis and
	 *  mc_vertices_per_cell triangles-generating vertices per cell
	 *  encoded in vertex identifier according to following formula:
	 *    encoded_position = mc_vertex_no + mc_vertices_per_cell * (x + CELLS_PER_AXIS * (y + CELLS_PER_AXIS * z))
	 *
	 *  @param  encoded_position_argument encoded position
	 *  @return                           cell coordinates ranged [0 .. CELLS_PER_AXIS-1] in x,y,z, and decoded vertex number in w.
	 */
	/* [Stage 4 decode_cell_position] */
	ivec4 decode_cell_position(in int encoded_position_argument)
	{
	    int CELLS_PER_AXIS = (samples_per_axis - 1);

	    ivec4 cell_position;
	    int   encoded_position = encoded_position_argument;

	    /* Decode combined triangle and vertex number. */
	    cell_position.w  = encoded_position % mc_vertices_per_cell;
	    encoded_position = encoded_position / mc_vertices_per_cell;

	    /* Decode coordinates from encoded position. */
	    cell_position.x  = encoded_position % CELLS_PER_AXIS;
	    encoded_position = encoded_position / CELLS_PER_AXIS;

	    cell_position.y  = encoded_position % CELLS_PER_AXIS;
	    encoded_position = encoded_position / CELLS_PER_AXIS;

	    cell_position.z  = encoded_position;

	    return cell_position;
	}
	/* [Stage 4 decode_cell_position] */

	/** Identifies cell type for provided cell position.
	 *
	 *  @param cell_position non-normalized cell position in space
	 *  @return              cell type in sense of Macrhing Cubes algorithm
	 */
	int get_cell_type(in ivec3 cell_position)
	{
	    int CELLS_PER_AXIS = (samples_per_axis - 1);

	    vec3 cell_position_normalized = vec3(cell_position) / float(CELLS_PER_AXIS - 1);

	    /* Get cell type index of cell to which currently processed vertex (triangle_and_vertex_number) belongs */
	    int  cell_type_index          = textureLod(cell_types, cell_position_normalized, 0.0).r;

	    return cell_type_index;
	}

	/** Performs a table lookup with cell type index and combined vertex-triangle number specified
	 *  to locate an edge number which vertex is currently processed.
	 *
	 *  @param cell_type_index                    cell type index (in Marching Cubes algorthm sense)
	 *  @param combined_triangle_no_and_vertex_no combined vertex and triangle numbers (by formula tringle*3 + vertex)
	 *
	 *  @return                                   edge number (in sense of Marching Cubes algorithm) or -1 if vertex does not belong to any edge
	 */
	int get_edge_number(in int cell_type_index, in int combined_triangle_no_and_vertex_no)
	{
	    /* Normalize indices for texture lookup: [0..14] -> [0.0..1.0], [0..255] -> [0.0..1.0]. */
	    vec2 tri_table_index = vec2(float(combined_triangle_no_and_vertex_no)/14.0, float(cell_type_index)/255.0);

	    return textureLod(tri_table, tri_table_index, 0.0).r;
	}

	/** Function calculates edge begin or edge end coordinates for specified cell and edge.
	 *
	 *  @param cell_origin_corner_coordinates normalized cell origin coordinates
	 *  @param edge_number                    edge number which coorinates being calculated
	 *  @param is_edge_start_vertex           true to request edge start vertex coordinates, false for end edge vertex
	 *  @return                               normalized edge start or end vertex coordinates
	*/
	vec3 get_edge_coordinates(in vec3 cell_origin_corner_coordinates, in int edge_number, in bool is_edge_start_vertex)
	{
	    /* These two arrays contain vertex indices which define a cell edge specified by index of arrays. */
	    const int   edge_begins_in_cell_corner[12]  = int[] ( 0,1,2,3,4,5,6,7,0,1,2,3 );
	    const int   edge_ends_in_cell_corner[12]    = int[] ( 1,2,3,0,5,6,7,4,4,5,6,7 );
	    /* Defines offsets by axes for each of 8 cell corneres. */
	    const ivec3 cell_corners_offsets[8]         = ivec3[8]
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

	    /* Edge corner number (number is in sense of Marching Cubes algorithm). */
	    int edge_corner_no;

	    if (is_edge_start_vertex)
	    {
	        /* Use start cell corner of the edge. */
	        edge_corner_no = edge_begins_in_cell_corner[edge_number];
	    }
	    else
	    {
	        /* Use end cell corner of the edge. */
	        edge_corner_no = edge_ends_in_cell_corner[edge_number];
	    }

	    /* Normalized cell corner coordinate offsets (to cell origin corner). */
	    vec3 normalized_corner_offsets = vec3(cell_corners_offsets[edge_corner_no]) / float(samples_per_axis - 1);

	    /* Normalized cell corner coordinates. */
	    vec3 edge_corner = cell_origin_corner_coordinates + normalized_corner_offsets;

	    return edge_corner;
	}

	/** Function calculates how close start_corner vertex to intersetction point.
	 *
	 *  @param start_corner beginning of edge
	 *  @param end_corner   end of edge
	 *  @param iso_level    scalar field value level defining isosurface
	 *  @return             start vertex portion (1.0, if isosurface comes through start vertex)
	 */
	float get_start_corner_portion(in vec3 start_corner, in vec3 end_corner, in float iso_level)
	{
	    float result;
	    float start_field_value = textureLod(scalar_field, start_corner, 0.0).r;
	    float end_field_value   = textureLod(scalar_field, end_corner, 0.0).r;
	    float field_delta       = abs(start_field_value - end_field_value);

	    if (field_delta > EPSILON)
	    {
	        /* Calculate start vertex portion. */
	        result = abs(end_field_value - iso_level) / field_delta;
	    }
	    else
	    {
	        /* Field values are too close in value to evaluate. Assume middle of an edge. */
	        result = 0.5;
	    }

	    return result;
	}

	/** Shader entry point. */
	void main()
	{
	    /* [Stage 4 Decode space position] */
	    /* Split gl_vertexID into cell position and vertex number processed by this shader instance. */
	    ivec4 cell_position_and_vertex_no = decode_cell_position(gl_VertexID);
	    ivec3 cell_position               = cell_position_and_vertex_no.xyz;
	    int   triangle_and_vertex_number  = cell_position_and_vertex_no.w;
	    /* [Stage 4 Decode space position] */

	    /* [Stage 4 Get cell type and edge number] */
	    /* Get cell type for cell current vertex belongs to. */
	    int   cell_type_index             = get_cell_type(cell_position);

	    /* Get edge of the cell to which belongs processed vertex. */
	    int   edge_number                 = get_edge_number(cell_type_index, triangle_and_vertex_number);
	    /* [Stage 4 Get cell type and edge number] */

	    /* Check if this is not a vertex of dummy triangle. */
	    if (edge_number != -1)
	    {
	        /* [Stage 4 Calculate cell origin] */
	        /* Calculate normalized coordinates in space of cell origin corner. */
	        vec3 cell_origin_corner    = vec3(cell_position) / float(samples_per_axis - 1);
	        /* [Stage 4 Calculate cell origin] */

	        /* [Stage 4 Calculate start and end edge coordinates] */
	        /* Calculate start and end edge coordinates. */
	        vec3 start_corner          = get_edge_coordinates(cell_origin_corner, edge_number, true);
	        vec3 end_corner            = get_edge_coordinates(cell_origin_corner, edge_number, false);
	        /* [Stage 4 Calculate start and end edge coordinates] */

	        /* [Stage 4 Calculate middle edge vertex] */
	        /* Calculate share of start point of an edge. */
	        float start_vertex_portion = get_start_corner_portion(start_corner, end_corner, iso_level);

	        /* Calculate ''middle'' edge vertex. This vertex is moved closer to start or end vertices of the edge. */
	        vec3 edge_middle_vertex    = mix(end_corner, start_corner, start_vertex_portion);
	        /* [Stage 4 Calculate middle edge vertex] */

	        /* [Stage 4 Calculate middle edge normal] */
	        /* Calculate normal to surface in the ''middle'' vertex. */
	        vec3 vertex_normal         = calc_phong_normal(start_vertex_portion, start_corner, end_corner);
	        /* [Stage 4 Calculate middle edge normal] */

	        /* Update vertex shader outputs. */
	        gl_Position                = mvp * vec4(edge_middle_vertex, 1.0);        /* Transform vertex position with MVP-matrix.        */
	        phong_vertex_position      = gl_Position;                                /* Set vertex position for fragment shader.          */
	        phong_vertex_normal_vector = vertex_normal;                              /* Set normal vector to surface for fragment shader. */
	        phong_vertex_color         = vec3(0.3,1.0,1.0);                                  /* Set vertex color for fragment shader.             */
	    }
	    else
	    {
	        /* [Stage 4 Discard dummy triangle] */
	        /* This cell type generates fewer triangles, and this particular one should be discarded. */
	        gl_Position                = vec4(0);                                    /* Discard vertex by setting its coordinate in infinity. */
	        phong_vertex_position      = gl_Position;
	        phong_vertex_normal_vector = vec3(0);
	        phong_vertex_color         = vec3(0);
	        /* [Stage 4 Discard dummy triangle] */
	    }
	}
);
const GLchar* marching_cubes_triangles_frag_shader = GLSL(

	/** Specify low precision for float type. */
	precision lowp float;

	/* Uniforms: */
	/** Current time moment. */
	uniform float time;

	/** Position of the vertex (and fragment) in world space. */
	in  vec4 phong_vertex_position;

	/** Surface normal vector in world space. */
	in  vec3 phong_vertex_normal_vector;

	/** Color passed from vertex shader. */
	in  vec3 phong_vertex_color;

	/* Output data: */
	/** Fragment color. */
	out vec4 FragColor;

	/** Shader entry point. Main steps are described in comments below. */
	void main()
	{
	    /* Distance to light source. */
	    const float light_distance = 5.0;

	    /* Add some movement to light source. */
	    float theta = float(time);
	    float phi   = float(time)/3.0;

	    vec3 light_location = vec3
	    (
	        light_distance * cos(theta) * sin(phi),
	        light_distance * cos(theta) * cos(phi),
	        light_distance * sin(theta)
	    );

	    /* Scene ambient color. */
	    const vec3  ambient_color = vec3(0.1, 0.1, 0.1);
	    const float attenuation   = 0.5;
	    const float shiness       = 3.0;

	    /* Normalize directions. */
	    vec3 normal_direction = normalize(phong_vertex_normal_vector);
	    vec3 view_direction   = normalize(vec3(vec4(0.0, 0.0, 1.0, 0.0) - phong_vertex_position));
	    vec3 light_direction  = normalize(light_location);

	    /** Calculate ambient lighting component of directional light. */
	    vec3 ambient_lighting    = ambient_color * phong_vertex_color;

	    /** Calculate diffuse reflection lighting component of directional light. */
	    vec3 diffuse_reflection  = attenuation * phong_vertex_color
	                             * max(0.0, dot(normal_direction, light_direction));

	    /** Calculate specular reflection lighting component of directional light. */
	    vec3 specular_reflection = vec3(0.0, 0.0, 0.0);

	    if (dot(normal_direction, light_direction) >= 0.0)
	    {
	        /* Light source on the right side. */
	        specular_reflection = attenuation * phong_vertex_color
	                            * pow(max(0.0, dot(reflect(-light_direction, normal_direction), view_direction)), shiness);
	    }

	    /** Calculate fragment lighting as sum of previous three component. */
	    FragColor = vec4(ambient_lighting + diffuse_reflection + specular_reflection, 1.0);
	    // FragColor = vec4(phong_vertex_normal_vector, 1.0);
        // FragColor = phong_vertex_position;
	}
);

void SetUpMarchingCubesTriangles(){

    /* 4. Marching Cubes algorithm triangle generation and rendering stage. */
    /* Create a program object that we will use for triangle generation and rendering stage. */
    marching_cubes_triangles_program_id = GL_CHECK(glCreateProgram());

    /* Initialize shaders for the triangle generation and rendering stage. */
    Shader::processShader(&marching_cubes_triangles_vert_shader_id, marching_cubes_triangles_vert_shader, GL_VERTEX_SHADER  );
    Shader::processShader(&marching_cubes_triangles_frag_shader_id, marching_cubes_triangles_frag_shader, GL_FRAGMENT_SHADER);

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
}

void DrawMarchingCubesTriangles(){

    /* 4. Marching Cubes algorithm triangle generation stage.
     *
     * At this stage, we render exactly (3 vertices * 5 triangles per cell *
     * amount of cells the scalar field is split to) triangle vertices.
     * Then render triangularized geometry.
     */
    // GL_CHECK(glActiveTexture(GL_TEXTURE0));

    /* Activate triangle generating and rendering program. */
    GL_CHECK(glUseProgram(marching_cubes_triangles_program_id));

    /* Specify input arguments to vertex shader. */
    GL_CHECK(glUniform1f(marching_cubes_triangles_uniform_time_id, model_time));

    /* [Stage 4 Run triangle generating and rendering program] */
    /* Run triangle generating and rendering program. */
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, cells_in_3d_space * triangles_per_cell * vertices_per_triangle));
    /* [Stage 4 Run triangle generating and rendering program] */
}

void CleanUpMarchingCubesTriangles(){
	GL_CHECK(glDeleteVertexArrays      (1, &marching_cubes_triangles_vao_id                  ));
    GL_CHECK(glDeleteShader            (    marching_cubes_triangles_frag_shader_id          ));
    GL_CHECK(glDeleteShader            (    marching_cubes_triangles_vert_shader_id          ));
    GL_CHECK(glDeleteProgram           (    marching_cubes_triangles_program_id              ));
    GL_CHECK(glDeleteTextures          (1, &marching_cubes_triangles_lookup_table_texture_id ));
}