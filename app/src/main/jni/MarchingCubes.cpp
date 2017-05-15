#include "ShaderCode.h"


GLuint        marching_cubes_program_id                           = 0;
GLuint        marching_cubes_geo_shader_id                        = 0;
GLuint        marching_cubes_vert_shader_id                       = 0;
GLuint        marching_cubes_frag_shader_id                       = 0;

const GLchar* marching_cubes_uniform_time_name                    = "time";
GLuint 		  marching_cubes_uniform_time_id                      = 0;

const GLchar* marching_cubes_uniform_samples_per_axis_name        = "samples_per_axis";
GLuint 		  marching_cubes_uniform_samples_per_axis_id          = 0;

const GLchar* marching_cubes_uniform_iso_level_name               = "iso_level";
GLuint 	      marching_cubes_uniform_iso_level_id                 = 0;

const GLchar* marching_cubes_uniform_mvp_name                     = "mvp";
GLuint 		  marching_cubes_uniform_mvp_id                       = 0;

const GLchar* marching_cubes_uniform_scalar_field_sampler_name    = "scalar_field";
GLuint 		  marching_cubes_uniform_scalar_field_sampler_id      = 0;

const GLchar* marching_cubes_uniform_tri_table_sampler_name       = "tri_table";
GLuint 		  marching_cubes_uniform_tri_table_sampler_id         = 0;

GLuint        marching_cubes_lookup_table_texture_id  			   = 0;
GLuint        marching_cubes_vao_id								   = 0;

const GLchar* marching_cubes_vert_shader = GLSL(

	uniform int samples_per_axis;

	out ivec3 space_position_in;

	ivec3 decode_space_position(in int cell_index)
	{
	    int cells_per_axis = samples_per_axis - 1;
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


	/** Shader entry point. */
	void main()
	{
		space_position_in = decode_space_position(gl_VertexID);
	}
);

const GLchar* marching_cubes_geo_shader = GLSL(
    
    layout(points) in;
    layout(triangle_strip, max_vertices = 15) out;

    precision highp isampler2D; /**< Specify high precision for isampler2D type. */
    precision highp isampler3D; /**< Specify high precision for isampler3D type. */
    precision highp sampler2D;  /**< Specify high precision for sampler2D type. */
    precision highp sampler3D;  /**< Specify high precision for sampler3D type. */

    in ivec3 space_position_in[];

    const int mc_vertices_per_cell = 15;
    uniform int samples_per_axis;
    uniform sampler3D scalar_field;
    uniform isampler2D tri_table;
    uniform mat4 mvp;
    uniform float iso_level;

    out vec4 phong_vertex_position;
    out vec3 phong_vertex_normal_vector;
    out vec3 phong_vertex_color;

    const float EPSILON = 0.000001f;

    float calc_partial_derivative(vec3 begin_vertex, vec3 end_vertex)
    {
        float field_value_begin = textureLod(scalar_field, begin_vertex, 0.0).r;
        float field_value_end   = textureLod(scalar_field, end_vertex,   0.0).r;

        return (field_value_end - field_value_begin) / distance(begin_vertex, end_vertex);
    }


    vec3 calc_cell_corner_normal(in vec3 p1)
    {
        vec3 normal = textureLod(scalar_field, p1, 0.0).gba;
        if(length(normal) < 0.00001){
            return vec3(0.0);
        }
        return normalize(normal);
    }


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

    int get_cell_type_index(in float cell_corner_field_value[8], in float isolevel)
        {
            int cell_type_index = 0;
            for (int i = 0; i < 8; i++)
            {
                if (cell_corner_field_value[i] < isolevel)
                {
                    cell_type_index |= (1<<i);
                }
            }

            return cell_type_index;
        }

    int get_edge_number(in int cell_type_index, in int combined_triangle_no_and_vertex_no)
    {
        vec2 tri_table_index = vec2(float(combined_triangle_no_and_vertex_no)/14.0, float(cell_type_index)/255.0);

        return textureLod(tri_table, tri_table_index, 0.0).r;
    }


    vec3 get_edge_coordinates(in vec3 cell_origin_corner_coordinates, in int edge_number, in bool is_edge_start_vertex)
    {
        const int   edge_begins_in_cell_corner[12]  = int[] ( 0,1,2,3,4,5,6,7,0,1,2,3 );
        const int   edge_ends_in_cell_corner[12]    = int[] ( 1,2,3,0,5,6,7,4,4,5,6,7 );
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
        int edge_corner_no;
        if (is_edge_start_vertex)
        {
            edge_corner_no = edge_begins_in_cell_corner[edge_number];
        }
        else
        {
            edge_corner_no = edge_ends_in_cell_corner[edge_number];
        }
        vec3 normalized_corner_offsets = vec3(cell_corners_offsets[edge_corner_no]) / float(samples_per_axis - 1);
        vec3 edge_corner = cell_origin_corner_coordinates + normalized_corner_offsets;
        return edge_corner;
    }

    float get_start_corner_portion(in vec3 start_corner, in vec3 end_corner, in float iso_level)
    {
        float result;
        float start_field_value = textureLod(scalar_field, start_corner, 0.0).r;
        float end_field_value   = textureLod(scalar_field, end_corner, 0.0).r;
        float field_delta       = abs(start_field_value - end_field_value);

        if (field_delta > EPSILON)
        {
            result = abs(end_field_value - iso_level) / field_delta;
        }
        else
        {
            result = 0.5;
        }

        return result;
    }

    void main()
    {
        ivec3 cell_position               = space_position_in[0];
        const int corners_in_cell = 8;
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

        vec3 scalar_field_normalizers = vec3(textureSize(scalar_field, 0)) - vec3(1, 1, 1);
        float scalar_field_in_cell_corners[8];
        ivec3 space_position = cell_position;
        for (int i = 0; i < corners_in_cell; i++)
        {
            ivec3 cell_corner = space_position + cell_corners_offsets[i];
            vec3 normalized_cell_corner  = vec3(cell_corner) / scalar_field_normalizers;
            scalar_field_in_cell_corners[i] = textureLod(scalar_field, normalized_cell_corner, 0.0).r;
        }
        int cell_type_index = get_cell_type_index(scalar_field_in_cell_corners, iso_level);
        int vertex_counter = 0;
        for(int triangle_and_vertex_number = 0; triangle_and_vertex_number!=15; triangle_and_vertex_number++ ){
            int   edge_number                 = get_edge_number(cell_type_index, triangle_and_vertex_number);
            if (edge_number != -1)
            {
                vec3 cell_origin_corner    = vec3(cell_position) / float(samples_per_axis - 1);
                vec3 start_corner          = get_edge_coordinates(cell_origin_corner, edge_number, true);
                vec3 end_corner            = get_edge_coordinates(cell_origin_corner, edge_number, false);
                float start_vertex_portion = get_start_corner_portion(start_corner, end_corner, iso_level);
                vec3 edge_middle_vertex    = mix(end_corner, start_corner, start_vertex_portion);
                vec3 vertex_normal         = calc_phong_normal(start_vertex_portion, start_corner, end_corner);
                gl_Position                = mvp * vec4(edge_middle_vertex, 1.0);        /* Transform vertex position with MVP-matrix.        */
                phong_vertex_position      = gl_Position;                                /* Set vertex position for fragment shader.          */
                phong_vertex_normal_vector = vertex_normal;                              /* Set normal vector to surface for fragment shader. */
                phong_vertex_color         = vec3(0.3,1.0,1.0); 
                EmitVertex();
                vertex_counter++;
                if(vertex_counter==3){
                    EndPrimitive();
                    vertex_counter=0;
                }
            } 
        }
    }
);

const GLchar* marching_cubes_frag_shader = GLSL(
   
    /** Specify low precision for float type. */
    precision lowp float;

    /* Uniforms: */
    /** Current time moment. */
    uniform float time;

    /** Position of the vertex (and fragment) in world space. */
    in  vec4 phong_vertex_position;
    in  vec3 phong_vertex_normal_vector;
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
        vec3 diffuse_reflection  = attenuation * phong_vertex_color * max(0.0, dot(normal_direction, light_direction));

        /** Calculate specular reflection lighting component of directional light. */
        vec3 specular_reflection = vec3(0.0, 0.0, 0.0);

        if (dot(normal_direction, light_direction) >= 0.0)
        {
            /* Light source on the right side. */
            specular_reflection = attenuation * phong_vertex_color
                                * pow(max(0.0, dot(reflect(-light_direction, normal_direction), view_direction)), shiness);
        }

        // * Calculate fragment lighting as sum of previous three component. 
        FragColor = vec4(diffuse_reflection.yzx, 1.0);
    }
);

void SetUpMarchingCubes(){

    /*Marching cubes using geometry shaders*/

    /*geometry shaders*/

    marching_cubes_program_id = GL_CHECK(glCreateProgram());;

    Shader::processShader(&marching_cubes_geo_shader_id, marching_cubes_geo_shader, GL_EXT_geometry_shader);
    Shader::processShader(&marching_cubes_vert_shader_id, marching_cubes_vert_shader, GL_VERTEX_SHADER);
    Shader::processShader(&marching_cubes_frag_shader_id, marching_cubes_frag_shader, GL_FRAGMENT_SHADER);

    GL_CHECK(glAttachShader(marching_cubes_program_id, marching_cubes_geo_shader_id));
    GL_CHECK(glAttachShader(marching_cubes_program_id, marching_cubes_vert_shader_id));
    GL_CHECK(glAttachShader(marching_cubes_program_id, marching_cubes_frag_shader_id));
    
    GL_CHECK(glLinkProgram(marching_cubes_program_id));
    
    marching_cubes_uniform_time_id                                  = GL_CHECK(glGetUniformLocation(marching_cubes_program_id, marching_cubes_uniform_time_name));
    marching_cubes_uniform_samples_per_axis_id                      = GL_CHECK(glGetUniformLocation(marching_cubes_program_id, marching_cubes_uniform_samples_per_axis_name));
    marching_cubes_uniform_iso_level_id                             = GL_CHECK(glGetUniformLocation(marching_cubes_program_id, marching_cubes_uniform_iso_level_name));
    marching_cubes_uniform_mvp_id                                   = GL_CHECK(glGetUniformLocation(marching_cubes_program_id, marching_cubes_uniform_mvp_name));
    marching_cubes_uniform_scalar_field_sampler_id                  = GL_CHECK(glGetUniformLocation(marching_cubes_program_id, marching_cubes_uniform_scalar_field_sampler_name));
    marching_cubes_uniform_tri_table_sampler_id                     = GL_CHECK(glGetUniformLocation(marching_cubes_program_id, marching_cubes_uniform_tri_table_sampler_name));
    
    GL_CHECK(glUseProgram(marching_cubes_program_id));

    /* Initialize uniforms constant throughout rendering loop. */
    GL_CHECK(glUniform1i(marching_cubes_uniform_samples_per_axis_id,            samples_per_axis));
    GL_CHECK(glUniform1f(marching_cubes_uniform_iso_level_id,                   isosurface_level));
    GL_CHECK(glUniform1i(marching_cubes_uniform_tri_table_sampler_id,   4               ));
    GL_CHECK(glUniform1i(marching_cubes_uniform_scalar_field_sampler_id,1               ));
    GL_CHECK(glUniformMatrix4fv(marching_cubes_uniform_mvp_id, 1, GL_FALSE,     mvp.getAsArray()));


    GL_CHECK(glGenTextures(1, &marching_cubes_lookup_table_texture_id));

    /* Lookup array (tri_table) uses GL_TEXTURE_2D target of texture unit 4. */
    GL_CHECK(glActiveTexture(GL_TEXTURE4));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, marching_cubes_lookup_table_texture_id));

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



    GL_CHECK(glGenVertexArrays(1, &marching_cubes_vao_id));

    GL_CHECK(glBindVertexArray(marching_cubes_vao_id));
}


void DrawMarchingCubes(){

	GL_CHECK(glUseProgram(marching_cubes_program_id));
    GL_CHECK(glUniform1f(marching_cubes_uniform_time_id, model_time));
    GL_CHECK(glDrawArrays(GL_POINTS, 0, cells_in_3d_space));
}

void CleanUpMarchingCubes(){
    GL_CHECK(glDeleteVertexArrays      (1, &marching_cubes_vao_id                            ));
    GL_CHECK(glDeleteShader            (    marching_cubes_frag_shader_id                    ));
    GL_CHECK(glDeleteShader            (    marching_cubes_vert_shader_id                    ));
    GL_CHECK(glDeleteShader            (    marching_cubes_geo_shader_id                     ));
    GL_CHECK(glDeleteProgram           (    marching_cubes_program_id                        ));
    GL_CHECK(glDeleteTextures          (1, &marching_cubes_lookup_table_texture_id           ));
}