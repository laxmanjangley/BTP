#version 330 core
// #version 300 es

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
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    return;
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
    const float attenuation   = 1.0;
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
}