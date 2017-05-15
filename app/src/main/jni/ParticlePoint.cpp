#include "ShaderCode.h"


GLuint        draw_points_program_id                                     = 0;

GLuint        draw_points_frag_shader_id                                 = 0;
GLuint        draw_points_vert_shader_id                                 = 0;

const GLchar* draw_points_uniform_mvp_name                               = "mvp";
GLuint        draw_points_uniform_mvp_id                                 = 0;

GLuint        draw_points_vao_id                                         = 0;

GLuint        draw_field_program_id                                     = 0;

GLuint        draw_field_frag_shader_id                                 = 0;
GLuint        draw_field_vert_shader_id                                 = 0;

const GLchar* draw_field_uniform_mvp_name                               = "mvp";
GLuint        draw_field_uniform_mvp_id                                 = 0;
const GLchar* draw_field_uniform_samples_per_axis_name                  = "samples_per_axis";
GLuint        draw_field_uniform_samples_per_axis_id                    = 0;

GLuint        draw_field_vao_id                                         = 0;



const GLchar* draw_points_vert_shader = GLSL(
    
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 velocity;
    layout (location = 2) in float pressure;

    uniform mat4 mvp;
    out vec4 color;

    void main(){
        vec3 t = position;
        t.x = pressure;
        gl_Position = mvp * vec4(position,1.0);
        
        if(pressure > 0.1){
            color = vec4(0.0,0.0,1.0,1.0)*pressure;// - vec4(1,1,0,0) * pressure;
        }
        else{
            color = vec4(1.0);
        }
    }
);

const GLchar* draw_points_frag_shader = GLSL(
    precision lowp float;
    out vec4 FragColor;
    in vec4 color;

    void main(){
        FragColor = color;
    }
);

const GLchar* draw_field_vert_shader = GLSL(
    
    layout (location = 0) in vec4 value;

    uniform mat4 mvp;
    uniform int samples_per_axis;
    out vec4 color;

    vec3 decode_position(int index){
        vec3 pos;
        pos.x = float(index % samples_per_axis)/float(samples_per_axis - 1);
        index /= samples_per_axis;
        pos.y = float(index % samples_per_axis)/float(samples_per_axis - 1);
        index /= samples_per_axis;
        pos.z = float(index)/float(samples_per_axis - 1);

        return pos;
    }

    void main(){
        vec3 position = decode_position(gl_VertexID);
        float field = value.x;
        vec3 normal = value.yzw;
        gl_Position = mvp * vec4(position,1.0);
        if(field>0.0){
            color = vec4(0.0,0.0,1.0,1.0);
        }
        else{
            color = vec4(1.0);
        }
    }
);

const GLchar* draw_field_frag_shader = GLSL(
    precision lowp float;
    out vec4 FragColor;
    in vec4 color;

    void main(){
        FragColor = color;
    }
);

void SetUpDrawPoints(){

	draw_points_program_id = GL_CHECK(glCreateProgram());

    Shader::processShader(&draw_points_vert_shader_id, draw_points_vert_shader, GL_VERTEX_SHADER  );
    Shader::processShader(&draw_points_frag_shader_id, draw_points_frag_shader, GL_FRAGMENT_SHADER);

    GL_CHECK(glAttachShader(draw_points_program_id, draw_points_vert_shader_id));
    GL_CHECK(glAttachShader(draw_points_program_id, draw_points_frag_shader_id));

    GL_CHECK(glLinkProgram(draw_points_program_id));

    draw_points_uniform_mvp_id = GL_CHECK(glGetUniformLocation  (draw_points_program_id, draw_points_uniform_mvp_name));
    
    GL_CHECK(glUseProgram(draw_points_program_id));

    GL_CHECK(glUniformMatrix4fv(draw_points_uniform_mvp_id, 1, GL_FALSE, mvp.getAsArray()));

    draw_field_program_id = GL_CHECK(glCreateProgram());

    Shader::processShader(&draw_field_vert_shader_id, draw_field_vert_shader, GL_VERTEX_SHADER  );
    Shader::processShader(&draw_field_frag_shader_id, draw_field_frag_shader, GL_FRAGMENT_SHADER);

    GL_CHECK(glAttachShader(draw_field_program_id, draw_field_vert_shader_id));
    GL_CHECK(glAttachShader(draw_field_program_id, draw_field_frag_shader_id));

    GL_CHECK(glLinkProgram(draw_field_program_id));

    draw_field_uniform_mvp_id = GL_CHECK(glGetUniformLocation  (draw_field_program_id, draw_field_uniform_mvp_name));
    draw_field_uniform_samples_per_axis_id = GL_CHECK(glGetUniformLocation  (draw_field_program_id, draw_field_uniform_samples_per_axis_name));
    
    GL_CHECK(glUseProgram(draw_field_program_id));

    GL_CHECK(glUniformMatrix4fv(draw_field_uniform_mvp_id, 1, GL_FALSE, mvp.getAsArray()));
    GL_CHECK(glUniform1i(draw_field_uniform_samples_per_axis_id, samples_per_axis));

    GL_CHECK(glGenVertexArrays(1, &draw_field_vao_id));

    GL_CHECK(glBindVertexArray(draw_field_vao_id));

    //GL_CHECK(glPointSize(3));
}

void DrawPoints(){

	GL_CHECK(glUseProgram(draw_points_program_id));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, particle_updater_position_buffer_id));
    GL_CHECK(glVertexAttribPointer(
        0,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    ));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, particle_updater_velocity_buffer_id));
    GL_CHECK(glVertexAttribPointer(
        1,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    ));
    GL_CHECK(glEnableVertexAttribArray(2));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, particle_updater_pressure_buffer_id));
    GL_CHECK(glVertexAttribPointer(
        2,                  // attribute
        1,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    ));

	GL_CHECK(glDrawArrays(GL_POINTS, 0, n_particles));    
}


void DrawScalarFieldPoints(){
    GL_CHECK(glUseProgram(draw_field_program_id));
    GL_CHECK(glEnableVertexAttribArray(0));
    int buffer = (particle_scalar_field_buffer_id)? particle_scalar_field_buffer_id: scalar_field_buffer_object_id;
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, particle_scalar_field_buffer_id));
    GL_CHECK(glVertexAttribPointer(
        0,                  // attribute
        4,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    ));
    

    GL_CHECK(glDrawArrays(GL_POINTS, 0, samples_in_3d_space));
}