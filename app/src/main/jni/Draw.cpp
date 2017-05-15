#include "Draw.h"

// int frames = 0;
// double update_particles_time = 0;
// double scalar_field_draw = 0;
// double marching_cubes_draw = 0;

double sort_time = 0;
double find_time = 0;
double update_time = 0;
double field_time = 0;
double mc_time = 0;
int frame_count = 0;


void calc_mvp(Matrix& mvp)
{
    /* Define projection properties. */
    float degreesToRadiansCoefficient = atanf(1) / 45;                            /* Coefficient to recalculate degrees to radians.      */
    float frustum_fovy                = 45.0f;                                    /* 45 degrees field of view in the y direction.        */
    float frustum_aspect              = (float)window_width/(float)window_height; /* Aspect ratio.                                       */
    float frustum_z_near              = 0.01f;                                    /* How close the viewer is to the near clipping plane. */
    float frustum_z_far               = 100.0f;                                   /* How far the viewer is from the far clipping plane.  */
    float camera_distance             = 2.5f;                                     /* Distance from camera to scene center.               */

    /* Matrix that stores temporary matrix data for translation transformations. */
    Matrix mat4_translate  = Matrix::createTranslation(-0.5, -0.5, -0.5);

    /* Matrix that stores temporary matrix data for scale transformations. */
    Matrix mat4_scale      = Matrix::createScaling    ( 2.0,  2.0,  2.0);

    /* Matrix that transforms the vertices from model space to world space. */
    /* Translate and scale coordinates from [0..1] to [-1..1] range for full visibility. */
    Matrix mat4_model_view = mat4_scale * mat4_translate;

    /* Pull the camera back from the scene center. */
    mat4_model_view[14] -= float(camera_distance);

    /* Create the perspective matrix from frustum parameters. */
    Matrix mat4_perspective = Matrix::matrixPerspective(degreesToRadiansCoefficient * frustum_fovy, frustum_aspect, frustum_z_near, frustum_z_far);

    /* MVP (Model View Perspective) matrix is a result of multiplication of Perspective Matrix by Model View Matrix. */
    mvp = mat4_perspective * mat4_model_view;
}


void setupGraphics(int width, int height)
{
    /* Store window width and height. */
    window_width  = width;
    window_height = height;

    /* Specify one byte alignment for pixels rows in memory for pack and unpack buffers. */
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CHECK(glPixelStorei(GL_PACK_ALIGNMENT,   1));

    GL_CHECK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
    /* Initialize model view projection matrix. */
    calc_mvp(mvp);

    // SetUpSphereUpdater();
    // SetUpScalarField();

    
    SetUpParticleUpdater();
    SetUpParticleSort();
    SetUpFindStartIndex();
    SetUpParticleScalarField();

    // SetUpDrawPoints();

    SetUpMarchingCubesCells();
    SetUpMarchingCubesTriangles();
    // SetUpMarchingCubes();

    /* Enable facet culling, depth testing and specify front face for polygons. */
    GL_CHECK(glEnable   (GL_DEPTH_TEST));
    GL_CHECK(glEnable   (GL_CULL_FACE ));
    GL_CHECK(glFrontFace(GL_CW        ));

    /* Start counting time. */
    timer.reset();
    timer.getDelta();
}

void print_time(){
    LOGI("particles : %d\ncells : %d\nsamples : %d\n",n_particles,space_cells_per_axis,samples_per_axis);
    LOGI("**************\n");
    LOGI("sort: %f\n",sort_time/100.0);
    LOGI("find: %f\n",find_time/100.0);
    LOGI("update: %f\n",update_time/100.0);
    LOGI("field: %f\n",field_time/100.0);
    LOGI("mc: %f\n",mc_time/100.0);
    LOGI("**************\n");
}

/** Draws one frame. */
void renderFrame(void)
{
    // /* Update time. */
    // model_time = timer.getTime();
    // // time_delta = timer.getDelta();
    // time_delta = 0.02;

    // /* Clear the buffers that we are going to render to in a moment. */
    // GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // // DrawSphereUpdater();
    // // DrawScalarField();

    // UpdateParticles();
    // UpdateParticleScalarField();
    // DrawScalarFieldPoints();
    
    // // DrawPoints();
    
   
    // // DrawMarchingCubes();
    
    // DrawMarchingCubesCells();
    // DrawMarchingCubesTriangles();

    /* Update time. */
    // model_time = timer.getTime();
    // time_delta = timer.getDelta();
    time_delta = 0.02;

    /* Clear the buffers that we are going to render to in a moment. */
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // DrawSphereUpdater();
    // DrawScalarField();

    UpdateParticles();
    
    // timer.getDelta();
    UpdateParticleScalarField();
    // field_time += timer.getDelta();
    // // DrawScalarFieldPoints();
    
    // // DrawPoints();
    
   
    // // DrawMarchingCubes();
    
    DrawMarchingCubesCells();
    DrawMarchingCubesTriangles();

    // mc_time += timer.getDelta();

    // frame_count ++;
    // if(frame_count == 100){
    //     print_time();
    //     sort_time = 0;
    //     find_time = 0;
    //     update_time = 0;
    //     field_time = 0;
    //     mc_time = 0;
    //     frame_count = 0;
    // }
}

/** Deinitialises OpenGL ES environment. */
void cleanup()
{

    // CleanUpSphereUpdater();
    // CleanUpScalarField();
    // CleanUpMarchingCubesCells();
    // CleanUpMarchingCubesTriangles();
    // CleanUpMarchingCubes();
}



