#version 330 core
// #version 300 es

/** Precision to avoid division-by-zero errors. */
#define EPSILON 0.000001f

/** Amount of spheres defining scalar field. This value should be synchronized between all files. */
#define N_SPHERES 3

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
out float scalar_field_value;
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
float calculate_scalar_field_value(in vec3 position)
{
    float field_value = 0.0f;

    /* Field value in given space position influenced by all spheres. */
    for (int i = 0; i < N_SPHERES; i++)
    {
        vec3  sphere_position         = input_spheres[i].xyz;
        float vertex_sphere_distance  = length(distance(sphere_position, position));

        /* Field value is a sum of all spheres fields in a given space position.
         * Sphere weight (or charge) is stored in w-coordinate.
         */
        field_value += input_spheres[i].w / pow(max(EPSILON, vertex_sphere_distance), 2.0);
    }

    return field_value;
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