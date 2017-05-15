#ifdef PCIMPL
#ifndef PCUTILS_H
#define PCUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <ctime>
#include "TriangulationTable.h"

using namespace std;

void SetupWindowAndGLContext();
void SwapBuffer();
bool PollForESC();
void PrintNewTriTable();

#endif /*PCUTILS_H*/
#endif