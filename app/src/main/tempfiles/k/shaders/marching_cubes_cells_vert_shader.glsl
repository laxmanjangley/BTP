#version 330 core
// #version 300 es

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