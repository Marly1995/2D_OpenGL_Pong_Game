// tag::C++11check[]
#define STRING2(x) #x
#define STRING(x) STRING2(x)

#if __cplusplus < 201103L
#pragma message("WARNING: the compiler may not be C++11 compliant. __cplusplus version is : " STRING(__cplusplus))
#endif
// end::C++11check[]

// tag::includes[]
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <math.h>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
// end::includes[]

// tag::namespace[]
using namespace std;
// end::namespace[]


// tag::globalVariables[]
std::string exeName;
SDL_Window *win; //pointer to the SDL_Window
SDL_GLContext context; //the SDL_GLContext
int frameCount = 0;
std::string frameLine = "";
GLfloat vecCol[] = { 0.0f, 1.0f, 1.0f, 1.0f };
bool isOne = true;
// end::globalVariables[]

// tag::vertexShader[]
//string holding the **source** of our vertex shader, to save loading from a file
const std::string strVertexShader = R"(
	#version 330
	in vec2 position;
	uniform vec2 rotation;
	uniform vec2 offset;
	uniform vec2 world;
	
	in vec2 texture;
	out vec2 Texture;
	void main()
	{
		Texture = texture;

		vec2 tmposition = position;
		gl_Position = vec4(((tmposition.x * cos(rotation.x))-(tmposition.y * sin(rotation.x))) + offset.x + world.x, ((tmposition.x * sin(rotation.x))+(tmposition.y * cos(rotation.x))) + offset.y + world.y, 0.0, 1.0);
		//gl_Position = vec4(tmposition, 0.0, 1.0);
	}
)";
// end::vertexShader[]

// tag::fragmentShader[]
//string holding the **source** of our fragment shader, to save loading from a file
const std::string strFragmentShader = R"(
	#version 330
	uniform vec4 vecCol;
	in vec2 Texture;
	uniform sampler2D tex;
	uniform bool isTex;
	out vec4 outputColor;
	void main()
	{
	   outputColor = texture(tex, Texture) * vecCol;
	}
)";
// end::fragmentShader[]

//our variables
bool done = false;

// Left Paddle vertex data
const GLfloat LeftvertexData[] = {
	// X	Y	VERTEX	  X	   Y	TEXTURE
	-0.90f, 0.25f,		0.0f, 1.0f,
	-0.85f, 0.25f,		1.0f, 1.0f,
	-0.90f,-0.25f,		0.0f, 0.0f,

	-0.90f,-0.25f,		0.0f, 0.0f,
	-0.85f,-0.25f,		1.0f, 0.0f,
	-0.85f, 0.25f,		1.0f, 1.0f
};
// Right Paddle vertex data
const GLfloat RightvertexData[] = {
	// X	Y	VERTEX	  X	   Y	TEXTURE
	0.90f, 0.25f,		1.0f, 1.0f,
	0.85f, 0.25f,		0.0f, 1.0f,
	0.90f,-0.25f,		1.0f, 0.0f,

	0.90f,-0.25f,		1.0f, 0.0f,
	0.85f,-0.25f,		0.0f, 0.0f,
	0.85f, 0.25f,		0.0f, 1.0f	
};
// Boundarys vertex data
const GLfloat boundsVertexData[] = {
	// X	Y	VERTEX		   No data for texture co-ordinates as none is needed
	-0.95f, -0.95f,			// This is due to the boundarys being all white
	-0.95f,  0.95f,			// The only data they need the color
	-0.98f,  0.95f,

	-0.95f, -0.95f,
	-0.98f,  0.95f,
	-0.98f, -0.95f,

	-0.98f, -0.95f,
	 0.95f, -0.95f,
	 0.95f, -0.98f,

	 0.95f, -0.98f,
	-0.98f, -0.98f,
	-0.98f, -0.95f,

	 0.95f, -0.98f,
	 0.98f, -0.98f,
	 0.95f,  0.95f,

	 0.95f,  0.95f,
	 0.98f,  0.95f,
	 0.98f, -0.98f,

	 0.98f,  0.95f,
	 0.98f,  0.98f,
	-0.98f,  0.95f,

	-0.98f,  0.98f,
	-0.98f,  0.95f,
	 0.98f,  0.98f,
};
// left background vertex data
const GLfloat leftScoreVertexData[] = {
	// X	Y	VERTEX	  X	   Y	TEXTURE
	-0.95f, -0.95f,		0.0f, 1.0f,
	-0.95f,  0.95f,		0.0f, 0.0f,
	 0.00f, -0.95f,		1.0f, 1.0f,

	 0.00f, -0.95f,		1.0f, 1.0f,
	-0.95f,  0.95f,		0.0f, 0.0f,
	 0.00f,  0.95f,		1.0f, 0.0f
};
// right background vertex data
const GLfloat rightScoreVertexData[] = {
	// X	Y	VERTEX	  X	   Y	TEXTURE
	 0.95f, -0.95f,		1.0f, 1.0f,
	 0.95f,  0.95f,		1.0f, 0.0f,
	 0.00f, -0.95f,		0.0f, 1.0f,

	 0.00f, -0.95f,		0.0f, 1.0f,
	 0.95f,  0.95f,		1.0f, 0.0f,
	 0.00f,  0.95f,		0.0f, 0.0f
};
// container for ball vertex data which is generated in the calcCircle function
GLfloat ballVertexData[12000];

float LPoffset[] = { 0.0, 0.0 };
float RPoffset[] = { 0.0, 0.0 };
float balloffset[] = { 0.0, 0.0 };
float worldoffset[] = { 0.0, 0.0 };

float ballOffsetVelocity[] = { 0.02, 0.01 };
float ballRotation[] = { 0, 0 };
float ballColor[] = { 1.0, 1.0, 1.0, 1.0 };

int LPscore = 0;
int RPscore = 0;
// paddle movement booleans
bool lpDown = false;
bool lpUp = false;

bool rpDown = false;
bool rpUp = false;

bool go = false;
bool gameOver = false;

//our GL and GLSL variables

GLuint theProgram; //GLuint that we'll fill in to refer to the GLSL program (only have 1 at this point)
GLint positionLocation; 
GLint offsetLocation; 
GLint colorLocation;
GLint rotationLocation;
GLint worldLocation;
GLint textureLocation;

GLuint LeftPaddleVertexDataBufferObject;
GLuint LeftPaddleVertexArrayObject;
GLuint LeftPaddleTexture;

GLuint RightPaddleVertexDataBufferObject;
GLuint RightPaddleVertexArrayObject;
GLuint RightPaddleTexture;

GLuint BallVertexDataBufferObject;
GLuint BallVertexArrayObject;
GLuint ballTexture;

GLuint boundsVertexDataBufferObject;
GLuint boundsVertexArrayObject;
GLuint boundsTexture;

GLuint leftScoreVertexDataBufferObject;
GLuint leftScoreVertexArrayObject;

GLuint rightScoreVertexDataBufferObject;
GLuint rightScoreVertexArrayObject;

GLuint Rscore0Texture;
GLuint Rscore1Texture;
GLuint Rscore2Texture;
GLuint RloserTexture;
GLuint RwinnerTexture;

GLuint Lscore0Texture;
GLuint Lscore1Texture;
GLuint Lscore2Texture;
GLuint LloserTexture;
GLuint LwinnerTexture;

// end Global Variables
/////////////////////////

// http://stackoverflow.com/questions/22444450/drawing-circle-with-opengl
// function to draw a circle by calculating vertex data for 1000 triangles
void calcCircle(float radius, int numberSegments) {
	float theta = 0.0f;
	float prev[] = { 0.05, 0.0 };

	for (int i = 0; i <= numberSegments * 6; )
	{
		theta = i * 2.0f * 3.1415926 / ((float)numberSegments * 6);
		// x & y co-ordinates for the center of the circle for every triangle
		ballVertexData[i++] = 0.0;
		ballVertexData[i++] = 0.0;
		ballVertexData[i++] = 0.5;		// texture location in central
		ballVertexData[i++] = 0.5;
		// x & y co-ordinates set for the second point of the triangle
		// this point is always the last point made so that the triangles link together
		ballVertexData[i++] = prev[0];
		ballVertexData[i++] = prev[1];
		ballVertexData[i++] = (prev[0] + 0.05) * 10;		// work out the texture co-ordinates
		ballVertexData[i++] = (prev[1] + 0.05) * 10;		// by making the objects scale 0-1
		// x & y co-ordinates calculated for the new point
		prev[0] = ballVertexData[i++] = radius * cosf(theta);
		prev[1] = ballVertexData[i++] = radius * sinf(theta);
		ballVertexData[i++] = (prev[0] + 0.05) * 10;
		ballVertexData[i++] = (prev[1] + 0.05) * 10;
	}
}

void initialise()

{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		exit(1);
	}
	cout << "SDL initialised OK!\n";
}

void createWindow()
{
	//get executable name, and use as window title
	int beginIdxWindows = exeName.rfind("\\"); //find last occurrence of a backslash
	int beginIdxLinux = exeName.rfind("/"); //find last occurrence of a forward slash
	int beginIdx = max(beginIdxWindows, beginIdxLinux);
	std::string exeNameEnd = exeName.substr(beginIdx + 1);
	const char *exeNameCStr = exeNameEnd.c_str();

	//create window
	win = SDL_CreateWindow(exeNameCStr, 300, 200, 1000, 1000, SDL_WINDOW_OPENGL); //same height and width makes the window square ...

																				//error handling
	if (win == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}
	cout << "SDL CreatedWindow OK!\n";
}

void setGLAttributes()
{
	int major = 3;
	int minor = 3;
	cout << "Built for OpenGL Version " << major << "." << minor << endl; //ahttps://en.wikipedia.org/wiki/OpenGL_Shading_Language#Versions
																		  // set the opengl context version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); //core profile
	cout << "Set OpenGL context to versicreate remote branchon " << major << "." << minor << " OK!\n";
}

void createContext()
{
	setGLAttributes();

	context = SDL_GL_CreateContext(win);
	if (context == nullptr) {
		SDL_DestroyWindow(win);
		std::cout << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}
	cout << "Created OpenGL context OK!\n";
}

void initGlew()
{
	GLenum rev;
	glewExperimental = GL_TRUE; //GLEW isn't perfect - see https://www.opengl.org/wiki/OpenGL_Loading_Library#GLEW
	rev = glewInit();
	if (GLEW_OK != rev) {
		std::cout << "GLEW Error: " << glewGetErrorString(rev) << std::endl;
		SDL_Quit();
		exit(1);
	}
	else {
		cout << "GLEW Init OK!\n";
	}
}

GLuint createShader(GLenum eShaderType, const std::string &strShaderFile)
{
	GLuint shader = glCreateShader(eShaderType);
	//error check
	const char *strFileData = strShaderFile.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch (eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		delete[] strInfoLog;
	}

	return shader;
}

GLuint createProgram(const std::vector<GLuint> &shaderList)
{
	GLuint program = glCreateProgram();

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}

void initializeProgram()
{
	std::vector<GLuint> shaderList;

	shaderList.push_back(createShader(GL_VERTEX_SHADER, strVertexShader));
	shaderList.push_back(createShader(GL_FRAGMENT_SHADER, strFragmentShader));

	theProgram = createProgram(shaderList);
	if (theProgram == 0)
	{
		cout << "GLSL program creation error." << std::endl;
		SDL_Quit();
		exit(1);
	}
	else {
		cout << "GLSL program creation OK! GLUint is: " << theProgram << std::endl;
	}

	positionLocation = glGetAttribLocation(theProgram, "position");
	offsetLocation = glGetUniformLocation(theProgram, "offset");
	colorLocation = glGetUniformLocation(theProgram, "vecCol");
	rotationLocation = glGetUniformLocation(theProgram, "rotation");
	worldLocation = glGetUniformLocation(theProgram, "world");
	textureLocation = glGetAttribLocation(theProgram, "texture");
	//clean up shaders (we don't need them anymore as they are no in theProgram)
	for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
}

void initializeVertexBuffer()
{
	//Left Paddle
	glGenBuffers(1, &LeftPaddleVertexDataBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, LeftPaddleVertexDataBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(LeftvertexData), LeftvertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	cout << "vertexDataBufferObject created OK! GLUint is: " << LeftPaddleVertexDataBufferObject << std::endl;

	//Right Paddle
	glGenBuffers(1, &RightPaddleVertexDataBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, RightPaddleVertexDataBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RightvertexData), RightvertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	cout << "vertexDataBufferObject created OK! GLUint is: " << RightPaddleVertexDataBufferObject << std::endl;

	//Ball
	glGenBuffers(1, &BallVertexDataBufferObject);
	calcCircle(0.05f, 1000);
	glBindBuffer(GL_ARRAY_BUFFER, BallVertexDataBufferObject);
	int size = sizeof(ballVertexData);
	glBufferData(GL_ARRAY_BUFFER, size, ballVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	cout << "vertexDataBufferObject created OK! GLUint is: " << BallVertexDataBufferObject << std::endl;

	// Bounds
	glGenBuffers(1, &boundsVertexDataBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, boundsVertexDataBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boundsVertexData), boundsVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	cout << "vertexDataBufferObject created OK! GLUint is: " << boundsVertexDataBufferObject << std::endl;

	// Left background
	glGenBuffers(1, &leftScoreVertexDataBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, leftScoreVertexDataBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(leftScoreVertexData), leftScoreVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	cout << "vertexDataBufferObject created OK! GLUint is: " << leftScoreVertexDataBufferObject << std::endl;

	// Right background
	glGenBuffers(1, &rightScoreVertexDataBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, rightScoreVertexDataBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rightScoreVertexData), rightScoreVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	cout << "vertexDataBufferObject created OK! GLUint is: " << rightScoreVertexDataBufferObject << std::endl;
}
void loadAssets()
{
	initializeProgram(); //create GLSL Shaders, link into a GLSL program, and get IDs of attributes and variables

	initializeVertexBuffer(); //load data into a vertex buffer

	cout << "Loaded Assets OK!\n";
}

void setupvertexArrayObject()
{
	//bounds
	glGenVertexArrays(1, &boundsVertexArrayObject); //create a Vertex Array Object
	cout << "Vertex Array Object created OK! GLUint is: " << boundsVertexArrayObject << std::endl;
	glBindVertexArray(boundsVertexArrayObject); //make the just created vertexArrayObject the active one
	glBindBuffer(GL_ARRAY_BUFFER, boundsVertexDataBufferObject); //bind vertexDataBufferObject
	glEnableVertexAttribArray(positionLocation); //enable attribute at index positionLocation
	glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0); //specify that position data contains four floats per vertex, and goes into attribute index positionLocation
	glBindVertexArray(0); //unbind the vertexArrayObject so we can't change it

	// left background
	glGenVertexArrays(1, &leftScoreVertexArrayObject); //create a Vertex Array Object
	cout << "Vertex Array Object created OK! GLUint is: " << leftScoreVertexArrayObject << std::endl;
	glBindVertexArray(leftScoreVertexArrayObject); //make the just created vertexArrayObject the active one
	glBindBuffer(GL_ARRAY_BUFFER, leftScoreVertexDataBufferObject); //bind vertexDataBufferObject
	glEnableVertexAttribArray(positionLocation); //enable attribute at index positionLocation
	glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(0 * sizeof(GLfloat))); //specify that position data contains four floats per vertex, and goes into attribute index positionLocation
	glEnableVertexAttribArray(textureLocation);
	glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(2 * sizeof(GLfloat)));
	glBindVertexArray(0); //unbind the vertexArrayObject so we can't change it

	//right background
	glGenVertexArrays(1, &rightScoreVertexArrayObject); //create a Vertex Array Object
	cout << "Vertex Array Object created OK! GLUint is: " << rightScoreVertexArrayObject << std::endl;
	glBindVertexArray(rightScoreVertexArrayObject); //make the just created vertexArrayObject the active one
	glBindBuffer(GL_ARRAY_BUFFER, rightScoreVertexDataBufferObject); //bind vertexDataBufferObject
	glEnableVertexAttribArray(positionLocation); //enable attribute at index positionLocation
	glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(0 * sizeof(GLfloat))); //specify that position data contains four floats per vertex, and goes into attribute index positionLocation
	glEnableVertexAttribArray(textureLocation);
	glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(2 * sizeof(GLfloat)));
	glBindVertexArray(0); //unbind the vertexArrayObject so we can't change it

	//Left Paddle
	glGenVertexArrays(1, &LeftPaddleVertexArrayObject); //create a Vertex Array Object
	cout << "Vertex Array Object created OK! GLUint is: " << LeftPaddleVertexArrayObject << std::endl;
	glBindVertexArray(LeftPaddleVertexArrayObject); //make the just created vertexArrayObject the active one
	glBindBuffer(GL_ARRAY_BUFFER, LeftPaddleVertexDataBufferObject); //bind vertexDataBufferObject
	glEnableVertexAttribArray(positionLocation); //enable attribute at index positionLocation
	glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(0 * sizeof(GLfloat))); //specify that position data contains four floats per vertex, and goes into attribute index positionLocation
	glEnableVertexAttribArray(textureLocation);
	glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(2 * sizeof(GLfloat)));
	glBindVertexArray(0); //unbind the vertexArrayObject so we can't change it

	//Right Paddle
	glGenVertexArrays(1, &RightPaddleVertexArrayObject); //create a Vertex Array Object
	cout << "Vertex Array Object created OK! GLUint is: " << RightPaddleVertexArrayObject << std::endl;
	glBindVertexArray(RightPaddleVertexArrayObject); //make the just created vertexArrayObject the active one
	glBindBuffer(GL_ARRAY_BUFFER, RightPaddleVertexDataBufferObject); //bind vertexDataBufferObject
	glEnableVertexAttribArray(positionLocation); //enable attribute at index positionLocation
	glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(0 * sizeof(GLfloat))); //specify that position data contains four floats per vertex, and goes into attribute index positionLocation
	glEnableVertexAttribArray(textureLocation);
	glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(2 * sizeof(GLfloat)));
	glBindVertexArray(0); //unbind the vertexArrayObject so we can't change it

	// Ball
	glGenVertexArrays(1, &BallVertexArrayObject); //create a Vertex Array Object
	cout << "Vertex Array Object created OK! GLUint is: " << BallVertexArrayObject << std::endl;
	glBindVertexArray(BallVertexArrayObject); //make the just created vertexArrayObject the active one
	glBindBuffer(GL_ARRAY_BUFFER, BallVertexDataBufferObject); //bind vertexDataBufferObject
	glEnableVertexAttribArray(positionLocation); //enable attribute at index positionLocation
	glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(0 * sizeof(GLfloat))); //specify that position data contains four floats per vertex, and goes into attribute index positionLocation
	glEnableVertexAttribArray(textureLocation);
	glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(GL_FLOAT)), (GLvoid *)(2 * sizeof(GLfloat)));
	glBindVertexArray(0); //unbind the vertexArrayObject so we can't change it

	// TEXTURES
	//ball texture
	glGenTextures(1, &ballTexture);
	glBindTexture(GL_TEXTURE_2D, ballTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* ballImage = SDL_LoadBMP("ball.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ballImage->w, ballImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, ballImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(ballImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//left paddle texture
	glGenTextures(1, &LeftPaddleTexture);
	glBindTexture(GL_TEXTURE_2D, LeftPaddleTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* lpImage = SDL_LoadBMP("Lpaddle.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lpImage->w, lpImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, lpImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(lpImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// bounds texture
	glGenTextures(1, &boundsTexture);
	glBindTexture(GL_TEXTURE_2D, boundsTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* boundsImage = SDL_LoadBMP("bounds.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, boundsImage->w, boundsImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, boundsImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(boundsImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//right paddle texture
	glGenTextures(1, &RightPaddleTexture);
	glBindTexture(GL_TEXTURE_2D, RightPaddleTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* rpImage = SDL_LoadBMP("Rpaddle.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rpImage->w, rpImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, rpImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(rpImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//background textures
	// right score 0
	glGenTextures(1, &Rscore0Texture);
	glBindTexture(GL_TEXTURE_2D, Rscore0Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* Rscore0Image = SDL_LoadBMP("rs0.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Rscore0Image->w, Rscore0Image->h, 0, GL_BGR, GL_UNSIGNED_BYTE, Rscore0Image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(Rscore0Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	// right score 1
	glGenTextures(1, &Rscore1Texture);
	glBindTexture(GL_TEXTURE_2D, Rscore1Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* Rscore1Image = SDL_LoadBMP("rs1.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Rscore1Image->w, Rscore1Image->h, 0, GL_BGR, GL_UNSIGNED_BYTE, Rscore1Image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(Rscore1Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	// right score 2
	glGenTextures(1, &Rscore2Texture);
	glBindTexture(GL_TEXTURE_2D, Rscore2Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* Rscore2Image = SDL_LoadBMP("rs2.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Rscore2Image->w, Rscore2Image->h, 0, GL_BGR, GL_UNSIGNED_BYTE, Rscore2Image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(Rscore2Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	// right loser texture
	glGenTextures(1, &RloserTexture);
	glBindTexture(GL_TEXTURE_2D, RloserTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* RloserImage = SDL_LoadBMP("Rloser.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RloserImage->w, RloserImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, RloserImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(RloserImage);
	glBindTexture(GL_TEXTURE_2D, 0);
	// right winner texture
	glGenTextures(1, &RwinnerTexture);
	glBindTexture(GL_TEXTURE_2D, RwinnerTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* RwinnerImage = SDL_LoadBMP("Rwinner.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RwinnerImage->w, RwinnerImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, RwinnerImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(RwinnerImage);
	glBindTexture(GL_TEXTURE_2D, 0);
	// left score 0 texture
	glGenTextures(1, &Lscore0Texture);
	glBindTexture(GL_TEXTURE_2D, Lscore0Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* Lscore0Image = SDL_LoadBMP("ls0.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Lscore0Image->w, Lscore0Image->h, 0, GL_BGR, GL_UNSIGNED_BYTE, Lscore0Image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(Lscore0Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	// left score 1 texture
	glGenTextures(1, &Lscore1Texture);
	glBindTexture(GL_TEXTURE_2D, Lscore1Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* Lscore1Image = SDL_LoadBMP("ls1.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Lscore1Image->w, Lscore1Image->h, 0, GL_BGR, GL_UNSIGNED_BYTE, Lscore1Image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(Lscore1Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	//left score 2 texture
	glGenTextures(1, &Lscore2Texture);
	glBindTexture(GL_TEXTURE_2D, Lscore2Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* Lscore2Image = SDL_LoadBMP("ls2.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Lscore2Image->w, Lscore2Image->h, 0, GL_BGR, GL_UNSIGNED_BYTE, Lscore2Image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(Lscore2Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	// left loser texture
	glGenTextures(1, &LloserTexture);
	glBindTexture(GL_TEXTURE_2D, LloserTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* LloserImage = SDL_LoadBMP("Lloser.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, LloserImage->w, LloserImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, LloserImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(LloserImage);
	glBindTexture(GL_TEXTURE_2D, 0);
	// left winner texture
	glGenTextures(1, &LwinnerTexture);
	glBindTexture(GL_TEXTURE_2D, LwinnerTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_Surface* LwinnerImage = SDL_LoadBMP("Lwinner.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, LwinnerImage->w, LwinnerImage->h, 0, GL_BGR, GL_UNSIGNED_BYTE, LwinnerImage->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(LwinnerImage);
	glBindTexture(GL_TEXTURE_2D, 0);

						  //cleanup
	glDisableVertexAttribArray(positionLocation); //disable vertex attribute at index positionLocation
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind array buffer

}

void handleInput()
{
	SDL_Event event; //somewhere to store an event

					 //NOTE: there may be multiple events per frame
	while (SDL_PollEvent(&event)) //loop until SDL_PollEvent returns 0 (meaning no more events)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			done = true; //set donecreate remote branch flag if SDL wants to quit (i.e. if the OS has triggered a close event,
						 //  - such as window close, or SIGINT
			break;

			//keydown handling - we should to the opposite on key-up for direction controls (generally)
		case SDL_KEYDOWN:
			event.key.repeat = true;
			if (event.key.repeat)
				switch (event.key.keysym.sym)
				{
					//hit escape to exit
				case SDLK_ESCAPE: done = true; 
					break;
				case SDLK_SPACE: go = true;		// make game go
					// statement resets game
					//positions reset as well as ball velocity and scores
					if (gameOver == true) {
						go = false;
						gameOver = false;
						balloffset[0] = 0;
						balloffset[1] = 0;
						ballOffsetVelocity[0] = 0.02;
						ballOffsetVelocity[1] = 0.01;
						RPscore = 0;
						LPscore = 0;
						worldoffset[0] = 0;
						worldoffset[1] = 0;
					}
					break;
				case SDLK_w: lpUp = true;
					break;
				case SDLK_s: lpDown = true;
					break;
				case SDLK_o: rpUp = true;
					break;
				case SDLK_l: rpDown = true;
				}
			break;
		case SDL_KEYUP:
			event.key.repeat = true;
			if (event.key.repeat)
				switch (event.key.keysym.sym)
				{
				case SDLK_w: lpUp = false;
					break;
				case SDLK_s: lpDown = false;
					break;
				case SDLK_o: rpUp = false;
					break;
				case SDLK_l: rpDown = false;
				}
			break;
		}
	}
}

void updateSimulation(double simLength) //update simulation with an amount of time to simulate for (in seconds)
{
	// default movements for ball
	if (go == true) {
		balloffset[0] += ballOffsetVelocity[0];
		balloffset[1] += ballOffsetVelocity[1];
		ballRotation[0] += 0.2f;
	}

	worldoffset[0] -= balloffset[0] / 100;
	worldoffset[1] -= balloffset[1] / 100;

	if (worldoffset[0] > 0.1) {
		worldoffset[0] = 0.1;
	}
	if (worldoffset[0] < -0.1) {
		worldoffset[0] = -0.1;
	}
	if (worldoffset[1] > 0.1) {
		worldoffset[1] = 0.1;
	}
	if (worldoffset[1] < -0.1) {
		worldoffset[1] = -0.1;
	}
	// color update to white for ball
	ballColor[0] += 0.02;
	ballColor[1] += 0.02;
	ballColor[2] += 0.02;
	ballColor[3] = 1.0;
	// ball bounding collision
	if (balloffset[0] >= 1) {
		balloffset[0] = 0;
		balloffset[1] = 0;
		worldoffset[0] = 0;
		worldoffset[1] = 0;
		LPscore++;
		go = false;
	}
	if (balloffset[0] <= -1) {
		balloffset[0] = 0;
		balloffset[1] = 0;
		worldoffset[0] = 0;
		worldoffset[1] = 0;
		RPscore++;
		go = false;
	}
	if (balloffset[1] >= 0.90) {
		ballOffsetVelocity[1] = ballOffsetVelocity[1] * -1;
	}
	if (balloffset[1] <= -0.90) {
		ballOffsetVelocity[1] = ballOffsetVelocity[1] * -1;
	}
	if (RPoffset[1] >= 0.70) {
		RPoffset[1] = 0.70;
	}
	if (RPoffset[1] <= -0.70) {
		RPoffset[1] = -0.70;
	}
	if (LPoffset[1] >= 0.70) {
		LPoffset[1] = 0.70;
	}
	if (LPoffset[1] <= -0.70) {
		LPoffset[1] = -0.70;
	}
	//ball paddle collision
	// left paddle
	if (balloffset[0] <= -0.80f && balloffset[1] >= (LPoffset[1] - 0.10f) && balloffset[1] <= (LPoffset[1] + 0.10f)) {
		ballOffsetVelocity[0] = ballOffsetVelocity[0] * -1.005;
		ballColor[2] = 0.0;
		ballColor[1] = 0.0;
		ballColor[0] = 1.0;
	}
	if (balloffset[0] <= -0.80f && balloffset[1] >= (LPoffset[1] + 0.10f) && balloffset[1] <= (LPoffset[1] + 0.25f)) {
		ballOffsetVelocity[0] = ballOffsetVelocity[0] * -1.002;
		ballOffsetVelocity[1] += 0.005;
		ballColor[2] = 0.0;
		ballColor[1] = 0.0;
		ballColor[0] = 1.0;
	}
	if (balloffset[0] <= -0.80f && balloffset[1] >= (LPoffset[1] - 0.25f) && balloffset[1] <= (LPoffset[1] - 0.10f)) {
		ballOffsetVelocity[0] = ballOffsetVelocity[0] * -1.002;
		ballOffsetVelocity[1] -= 0.005;
		ballColor[2] = 0.0;
		ballColor[1] = 0.0;
		ballColor[0] = 1.0;
	}
	// right paddle
	if (balloffset[0] >= 0.80f && balloffset[1] >= (RPoffset[1] - 0.10f) && balloffset[1] <= (RPoffset[1] + 0.10f)) {
		ballOffsetVelocity[0] = ballOffsetVelocity[0] * -1.005;
		ballColor[2] = 1.0;
		ballColor[1] = 0.0;
		ballColor[0] = 0.0;
	}
	if (balloffset[0] >= 0.80f && balloffset[1] >= (RPoffset[1] + 0.10f) && balloffset[1] <= (RPoffset[1] + 0.25f)) {
		ballOffsetVelocity[0] = ballOffsetVelocity[0] * -1.002;
		ballOffsetVelocity[1] += 0.005;
		ballColor[2] = 1.0;
		ballColor[1] = 0.0;
		ballColor[0] = 0.0;
	}
	if (balloffset[0] >= 0.80f && balloffset[1] >= (RPoffset[1] - 0.25f) && balloffset[1] <= (RPoffset[1] - 0.10f)) {
		ballOffsetVelocity[0] = ballOffsetVelocity[0] * -1.002;
		ballOffsetVelocity[1] -= 0.005;
		ballColor[2] = 1.0;
		ballColor[1] = 0.0;
		ballColor[0] = 0.0;
	}

	// paddle movement
	//reset bools
	if (rpDown == true)
	{
		RPoffset[1] -= 0.02f;
	}
	if (rpUp == true)
	{
		RPoffset[1] += 0.02f;
	}
	if (lpDown == true)
	{
		LPoffset[1] -= 0.02f;
	}
	if (lpUp == true)
	{
		LPoffset[1] += 0.02f;
	}

}

void preRender()
{
	glViewport(0, 0, 1000, 1000); //set viewpoint
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //set clear colour
	glClear(GL_COLOR_BUFFER_BIT); //clear the window (technical the scissor box bounds)
}

void render()
{
	glUseProgram(theProgram); //installs the program object specified by program as part of current rendering state

	glUniform2f(worldLocation, worldoffset[0], worldoffset[1]);

	if (LPscore == 0) {
		glBindTexture(GL_TEXTURE_2D, Lscore0Texture);
	}
	if (LPscore == 1) {
		glBindTexture(GL_TEXTURE_2D, Lscore1Texture);
	}
	if (LPscore == 2) {
		glBindTexture(GL_TEXTURE_2D, Lscore2Texture);
	}
	if (LPscore == 3) {
		glBindTexture(GL_TEXTURE_2D, LwinnerTexture);
	}
	if (RPscore == 3) {
		glBindTexture(GL_TEXTURE_2D, LloserTexture);
		gameOver = true;
	}
	glBindVertexArray(leftScoreVertexArrayObject);
	glUniform2f(offsetLocation, 0, 0);
	glUniform4f(colorLocation, 1, 1, 1, 1);
	glUniform2f(rotationLocation, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6); //Draw something, using Triangles, and 3 vertices - i.e. one lonely triangle
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (RPscore == 0) {
		glBindTexture(GL_TEXTURE_2D, Rscore0Texture);
	}
	if (RPscore == 1) {
		glBindTexture(GL_TEXTURE_2D, Rscore1Texture);
	}
	if (RPscore == 2) {
		glBindTexture(GL_TEXTURE_2D, Rscore2Texture);
	}
	if (RPscore == 3) {
		glBindTexture(GL_TEXTURE_2D, RwinnerTexture);
	}
	if (LPscore == 3) {
		glBindTexture(GL_TEXTURE_2D, RloserTexture);
		gameOver = true;
	}
	glBindVertexArray(rightScoreVertexArrayObject);
	glUniform2f(offsetLocation, 0, 0);
	glUniform4f(colorLocation, 1, 1, 1, 1);
	glUniform2f(rotationLocation, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6); //Draw something, using Triangles, and 3 vertices - i.e. one lonely triangle
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, boundsTexture);
	glBindVertexArray(boundsVertexArrayObject);
	glUniform2f(offsetLocation, 0, 0);
	glUniform4f(colorLocation, 1, 1, 1, 1);
	glUniform2f(rotationLocation, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 24); //Draw something, using Triangles, and 3 vertices - i.e. one lonely triangle
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, LeftPaddleTexture);
	glBindVertexArray(LeftPaddleVertexArrayObject);
	glUniform2f(offsetLocation, LPoffset[0], LPoffset[1]);
	glUniform4f(colorLocation, 1, 1, 1, 1);
	glUniform2f(rotationLocation, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6); //Draw something, using Triangles, and 3 vertices - i.e. one lonely triangle
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, RightPaddleTexture);
	glBindVertexArray(RightPaddleVertexArrayObject);
	glUniform2f(offsetLocation, RPoffset[0], RPoffset[1]);
	glUniform4f(colorLocation, 1, 1, 1, 1);
	glUniform2f(rotationLocation, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6); //Draw something, using Triangles, and 3 vertices - i.e. one lonely triangle
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindVertexArray(BallVertexArrayObject);
	glBindTexture(GL_TEXTURE_2D, ballTexture);
	glUniform2f(offsetLocation, balloffset[0], balloffset[1]);
	glUniform4f(colorLocation, ballColor[0], ballColor[1], ballColor[2], ballColor[3]);
	glUniform2f(rotationLocation, ballRotation[0], ballRotation[1]);
	glDrawArrays(GL_TRIANGLES, 0, 3000); //Draw something, using Triangles, and 3 vertices - i.e. one lonely triangle
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	

	glUseProgram(0); //clean up

}

void postRender()
{
	SDL_GL_SwapWindow(win);; //present the frame buffer to the display (swapBuffers)
	frameLine += "Frame: " + std::to_string(frameCount++);
	cout << "\r" << frameLine << std::flush;
	frameLine = "";
}

void cleanUp()
{
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	cout << "Cleaning up OK!\n";
}

int main(int argc, char* args[])
{
	exeName = args[0];
	//setup
	//- do just once
	initialise();
	createWindow();

	createContext();

	initGlew();

	glViewport(0, 0, 1000, 1000); //should check what the actual window res is?

	SDL_GL_SwapWindow(win); //force a swap, to make the trace clearer

							//do stuff that only needs to happen once
							//- create shaders
							//- load vertex data
	loadAssets();



	//setup a GL object (a VertexArrayObject) that stores how to access data and from where
	setupvertexArrayObject();

	while (!done) //loop until done flag is set)
	{
		handleInput();

		updateSimulation(0.02); //call update simulation with an amount of time to simulate for (in seconds)
								//WARNING - we are always updating by a constant amount of time. This should be tied to how long has elapsed
								// see, for example, http://headerphile.blogspot.co.uk/2014/07/part-9-no-more-delays.html

		preRender();

		render(); //RENDER HERE - PLACEHOLDER

		postRender();

	}

	//cleanup and exit
	cleanUp();
	SDL_Quit();

	return 0;
}
// fin