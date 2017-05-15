#ifndef PCUTILS_H
#define PCUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
// #include <GL/glew.h>
// #include <GLFW/glfw3.h>
#include <vector>
#include <ctime>

using namespace std;

void setupWindowAndGLContext();
void SwapBuffer();
bool PollForESC();

#endif /* PCUTILS_H */