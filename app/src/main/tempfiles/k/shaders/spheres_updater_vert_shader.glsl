#version 330 core
// #version 300 es

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