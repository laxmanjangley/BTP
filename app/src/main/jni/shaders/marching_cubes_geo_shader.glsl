#version 330 core
// #version 300 es

layout(points) in;
// layout(triangle_strip, max_vertices = 15) out;
layout(points, max_vertices = 15) out;

precision highp isampler2D; /**< Specify high precision for isampler2D type. */
precision highp isampler3D; /**< Specify high precision for isampler3D type. */
precision highp sampler2D;  /**< Specify high precision for sampler2D type. */
precision highp sampler3D;  /**< Specify high precision for sampler3D type. */

/** Precision to avoid division-by-zero errors. */
#define EPSILON 0.000001f

/** Amount of cells taken for each axis of a scalar field. */
#define CELLS_PER_AXIS (samples_per_axis - 1)

/** Maximum amount of vertices a single cell can define. */
const int mc_vertices_per_cell = 15;


in ivec3 space_position_in[];

/* Uniforms: */
/** Amount of samples taken for each axis of a scalar field. */
uniform int samples_per_axis;

/** A 3D texture is used to deliver scalar field data. */
uniform sampler3D scalar_field;

/** A 2D texture representing tri_table lookup array. Array contains edge numbers (in sense of Marching Cubes algorithm).
    As input parameters (indices to texture) should be specified cell type index and combined vertex-triangle number. */



/** Combined model view and projection matrices. */
uniform mat4 mvp;

/** Isosurface level. */
uniform float iso_level;

/* Phong shading output variables for fragment shader. */
out vec4 phong_vertex_position;      /**< position of the vertex in world space.  */
out vec3 phong_vertex_normal_vector; /**< surface normal vector in world space.   */
out vec3 phong_vertex_color;         /**< vertex color for fragment colorisation. */


float scalar_field_in_cell_corners[8];
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

int get_cell_type_index(in float isolevel)
{
    int cell_type_index = 0;

    /* Iterate through all cell corners. */
    for (int i = 0; i < 8; i++)
    {
        /* If corner is inside isosurface then set bit in cell type index index. */
        if (scalar_field_in_cell_corners[i] < isolevel)
        {
            /* Set appropriate corner bit in cell type index. */
            cell_type_index |= (1<<i);
        }
    }

    return cell_type_index;
}


float calc_partial_derivative(vec3 begin_vertex, vec3 end_vertex)
{
    float field_value_begin = textureLod(scalar_field, begin_vertex, 0.0).r;
    float field_value_end   = textureLod(scalar_field, end_vertex,   0.0).r;

    return (field_value_end - field_value_begin) / distance(begin_vertex, end_vertex);
}


vec3 calc_cell_corner_normal(in vec3 p1)
{
    vec3 result;
    vec3 delta;

    /* Use neighbour samples to calculate derivative. */
    delta = vec3(1.0/float(samples_per_axis - 1), 0, 0);
    result.x = calc_partial_derivative(p1 - delta, p1 + delta);

    delta = vec3(0.0, 1.0/float(samples_per_axis - 1), 0.0);
    result.y = calc_partial_derivative(p1 - delta, p1 + delta);

    delta = vec3(0.0, 0.0, 1.0/float(samples_per_axis - 1));
    result.z = calc_partial_derivative(p1 - delta, p1 + delta);

    return result;
}


vec3 calc_phong_normal(in float start_vertex_portion, in vec3 edge_start, in vec3 edge_end)
{
    /* Find normal vector in begin vertex of the edge. */
    vec3 edge_start_normal = calc_cell_corner_normal(edge_start);
    /* Find normal vector in end vertex of the edge. */
    vec3 edge_end_normal   = calc_cell_corner_normal(edge_end);

    /* Interpolate normal vector. */
    return mix(edge_end_normal, edge_start_normal, start_vertex_portion);
}

float get_edge_scalar_fields(in vec3 cell_origin_corner_coordinates, in int edge_number, in bool is_edge_start_vertex)
{
    /* These two arrays contain vertex indices which define a cell edge specified by index of arrays. */
    

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

    return scalar_field_in_cell_corners[edge_corner_no];
}


float get_start_corner_portion(in float start_field_value, in float end_field_value, in float iso_level)
{
    float result;
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

/** Shader entry point. */

void main()
{
    gl_Position = vec4(vec3(space_position_in[0]) / float(63),1);
    EmitVertex();
    EndPrimitive();
    return;

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

    ivec3 space_position = space_position_in[0];
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
    int cell_type_index = get_cell_type_index(iso_level);

    int vertex_counter = 0;
    
    vec3 cell_origin_corner    = vec3(space_position) / float(samples_per_axis - 1);
    

    for(int edge_number_index =0; edge_number_index != 3; edge_number_index++) {

    	int   edge_number = edge_number_index%12;//trin_table[cell_type_index*15+edge_number_index];
    	
    	// if(!(edge_number>=0&&edge_number<=11))
    	// 	continue;
        
        /* [Stage 4 Calculate cell origin] */
        /* Calculate normalized coordinates in space of cell origin corner. */
        /* [Stage 4 Calculate cell origin] */

        /* [Stage 4 Calculate start and end edge coordinates] */
        /* Calculate start and end edge coordinates. */
        float start_field_value          = scalar_field_in_cell_corners[edge_begins_in_cell_corner[edge_number]];
        // float start_field_value          =  get_edge_scalar_fields(cell_origin_corner, edge_number, true);
        float end_field_value            = scalar_field_in_cell_corners[edge_ends_in_cell_corner[edge_number]];
        // float end_field_value            = get_edge_scalar_fields(cell_origin_corner, edge_number, false);
        // vec3 start_corner                = get_edge_coordinates(cell_origin_corner, edge_number, true);
        vec3 start_corner                = cell_origin_corner + vec3(cell_corners_offsets[edge_begins_in_cell_corner[edge_number]]) / float(samples_per_axis - 1);
        // vec3 end_corner                  = get_edge_coordinates(cell_origin_corner, edge_number, false);
        vec3 end_corner                  = cell_origin_corner + vec3(cell_corners_offsets[edge_ends_in_cell_corner[edge_number]]) / float(samples_per_axis - 1);
        
        /* [Stage 4 Calculate start and end edge coordinates] */

        /* [Stage 4 Calculate middle edge vertex] */
        /* Calculate share of start point of an edge. */
        float start_vertex_portion = get_start_corner_portion(start_field_value, end_field_value, iso_level);

        // /* Calculate ''middle'' edge vertex. This vertex is moved closer to start or end vertices of the edge. */
        vec3 edge_middle_vertex    = mix(end_corner, start_corner, start_vertex_portion);
        // /* [Stage 4 Calculate middle edge vertex] */

        // /* [Stage 4 Calculate middle edge normal] */
        // /* Calculate normal to surface in the ''middle'' vertex. */
        // vec3 vertex_normal         = calc_phong_normal(start_vertex_portion, start_corner, end_corner);
        vec3 vertex_normal = vec3(1);
        /* [Stage 4 Calculate middle edge normal] */

        /* Update vertex shader outputs. */
        // gl_Position                = mvp * vec4(edge_middle_vertex, 1.0);        /* Transform vertex position with MVP-matrix.        */
        // phong_vertex_position      = gl_Position;                                /* Set vertex position for fragment shader.          */
        // phong_vertex_normal_vector = vertex_normal;                              /* Set normal vector to surface for fragment shader. */
        // phong_vertex_color         = vec3(0.7);                                  /* Set vertex color for fragment shader.             */
        
        // EmitVertex();
        // vertex_counter++;
        // if(vertex_counter == 3){
        // 	EndPrimitive();
        // 	vertex_counter = 0;
        // }
    }
    EndPrimitive();
}









