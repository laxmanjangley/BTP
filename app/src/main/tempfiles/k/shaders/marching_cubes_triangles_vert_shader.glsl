#version 330 core
// #version 300 es

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
        phong_vertex_color         = vec3(0.7);                                  /* Set vertex color for fragment shader.             */
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