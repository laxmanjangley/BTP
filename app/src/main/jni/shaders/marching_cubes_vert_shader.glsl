#version 330 core
// #version 300 es

uniform int samples_per_axis;

out ivec3 space_position_in;

ivec3 decode_space_position(in int cell_index)
{
    int cells_per_axis = 63;//samples_per_axis - 1;
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