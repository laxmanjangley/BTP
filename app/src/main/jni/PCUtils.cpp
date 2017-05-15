#ifdef PCIMPL
#include "PCUtils.h"

GLFWwindow* window;

double oldTime = glfwGetTime();
int frames = 0;

void SetupWindowAndGLContext(){
	
	srand(time(NULL));

	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return ;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow( 1366, 768, "Fluid Simmulator", glfwGetPrimaryMonitor(), NULL);
	
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return ;
	}
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
}

void SwapBuffer(){
	glfwSwapBuffers(window);
	frames++;
	double now = glfwGetTime ();
	if(now - oldTime > 1.0){
		double delta = now - oldTime;
		cout<<float(frames)/delta<<endl;
		frames = 0;
		oldTime = now;
	}
}

bool PollForESC(){
	glfwPollEvents();
	if(glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS || glfwWindowShouldClose(window)){
		glfwDestroyWindow(window);
		glfwTerminate();
		return true;
	}
	return false;
}

void PrintNewTriTable(){
	vector<vector<int> > compressedTable;
	for(int i=0;i!=256;i++){
		vector<int> tempBuff;
		tempBuff.clear();
		for(int j=0;j!=15;j++){
			if(tri_table[i*15+j]!= -1){
				tempBuff.push_back(tri_table[i*15+j]);
			}
		}
		compressedTable.push_back(tempBuff);
	}
	int counter = 0;
	vector<int> start;
	cout<<"{ ";

	for(int i=0;i!=compressedTable.size();i++){
		start.push_back(counter);
		for(int j=0;j!=compressedTable[i].size();j++){
			counter++;
			cout<<compressedTable[i][j]<<" , ";
			if(counter%9==0)
				cout<<endl;
		}
	}
	cout<<"}"<<endl;
	start.push_back(counter);
	cout<<endl<<endl<<"{ ";
	for(int i=0;i!=start.size();i++){
		cout<<start[i]<<" , ";
	}
	cout<<"}"<<endl;
}

#endif 