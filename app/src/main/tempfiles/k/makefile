PostFlags = -lGLEW -lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -lpthread
PreFlags = -std=c++11 -w -o Flow
CXX = g++
all: *.h *.cpp
	$(CXX) $(PreFlags) *.cpp $(PostFlags)
clean: 
	rm -f *.o *.ghc Flow 
