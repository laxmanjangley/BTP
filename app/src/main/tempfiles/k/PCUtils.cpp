// #include "PCUtils.h"

// GLFWwindow* window;

// double oldTime = glfwGetTime();
// int frames = 0;

// void setupWindowAndGLContext(){
	
// 	srand(time(NULL));

// 	if( !glfwInit() )
// 	{
// 		fprintf( stderr, "Failed to initialize GLFW\n" );
// 		return ;
// 	}

// 	glfwWindowHint(GLFW_SAMPLES, 4);
// 	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
// 	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
// 	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// 	window = glfwCreateWindow( 1366, 768, "Fluid Simmulator", glfwGetPrimaryMonitor(), NULL);
	
// 	glfwMakeContextCurrent(window);

// 	glewExperimental = true;
// 	if (glewInit() != GLEW_OK) {
// 		fprintf(stderr, "Failed to initialize GLEW\n");
// 		return ;
// 	}
// 	glfwSwapInterval(1);
// 	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
// 	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
// }

// void SwapBuffer(){
// 	glfwSwapBuffers(window);
// 	frames++;
// 	double now = glfwGetTime ();
// 	if(now - oldTime > 1.0){
// 		cout<<frames<<endl;
// 		frames = 0;
// 		oldTime = now;
// 	}
// }

// bool PollForESC(){
// 	glfwPollEvents();
// 	if(glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS || glfwWindowShouldClose(window)){
// 		glfwDestroyWindow(window);
// 		glfwTerminate();
// 		return true;
// 	}
// 	return false;
// }