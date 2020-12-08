//Kellie Turchetta
//CS-330
//Final Project

// Header Inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM math header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; //standard namespace

#define WINDOW_TITLE "Modern OpenGL" //Window title macro

//Shader program macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

// Variable declarations for shader, window size initialization, buffer & array objects
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, EBO, LightVAO, ChairVAO, texture;

//Subject position and scale
glm::vec3 ChairPosition(0.0f, 0.0f, 0.0f);
glm::vec3 ChairScale(2.0f);

//light color
glm::vec3 ObjectColor(1.0f, 0.0f, 0.0f);
glm::vec3 LightColor(1.0f, 1.0f, 1.0f);

//Light position & scale
glm::vec3 LightPosition(0.5f, 0.5f, -3.0f);
glm::vec3 LightScale(0.3f);


GLfloat cameraSpeed = 0.0005f; // movement speed per frame

GLchar currentKey; //will store key pressed

GLfloat lastMouseX = 400, lastMouseY = 300; // Locks mouse cursor at center of screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f; // mouse offset, yaw, and pitch variables
GLfloat sensitivity = 0.005f; // used for mouse/camera rotation sensitivity
bool mouseDetected = true; // initially true when mouse movement is detected

//Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); // initial camera position placed 5 units in z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); // temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); // temporary z unit vector
glm::vec3 front; // temporary z unit vector for mouse


//function prototypes
void UResizeWindow(int,int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UMouseMove(int x, int y);
void UKeyboard(unsigned char key, int x, int y);
void UKeyReleased(unsigned char key, int x, int y);

// Vertex Shader Source Code
const GLchar * vertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
	layout (location = 1) in vec3 color; //Color data from Vertex Attrib Pointer 1

	out vec3 Normal; //For outgoing normals to fragment shader
	out vec3 FragmentPos; //For outgoing color / pixels to fragment shader
	out vec3 mobileColor; // variable to transfer color data to fragment shader

	//Global variables for transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

		mobileColor = color; // references incoming color data
	}
);

// Fragment Shader Source Code
const GLchar * fragmentShaderSource = GLSL(330,

	in vec3 Normal; //for incoming normals
	in vec3 FragmentPos; //For incoming fragment position
	in vec3 mobileColor; // Variable to hold incoming color data from vertex shader

	out vec4 gpuColor; //variable to pass color data to GPU

	uniform vec3 LightColor;
	uniform vec3 LightPos;
	uniform vec3 viewPosition;
	uniform vec3 ObjectColor;

	void main() {


		gpuColor = vec4(mobileColor, 1.0);


	}
);

// Main Program
int main(int argc, char * argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
		if(glewInit() != GLEW_OK) {
			std::cout << "Failed to initialize GLEW" << std:: endl;
			return -1;
		}

	UCreateShader();

	UCreateBuffers(); // Call Buffer function

	glUseProgram(shaderProgram); //Use the Shader Program

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Sets background color

	glutDisplayFunc(URenderGraphics);

	glutKeyboardFunc(UKeyboard); //Detects key press

	glutKeyboardUpFunc(UKeyReleased); //Detects key release

	glutPassiveMotionFunc(UMouseMove); //Detects mouse movement

	glutMainLoop();

	//Destroy Buffer objects once used
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	return 0;
}

/*Resize window*/
void UResizeWindow(int w, int h) {
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

/*Renders graphics*/
void URenderGraphics(void) {

	glEnable(GL_DEPTH_TEST); // enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clears the screen

	glBindVertexArray(VAO); // Activate vertex array object before rendering and transforming them

	//Camera movement logic
	CameraForwardZ = front; //replaces camera forward vector with Radians normalized as a unit vector

	if(currentKey == 'w')
		cameraPosition += cameraSpeed * CameraForwardZ;

	if(currentKey == 's')
		cameraPosition -= cameraSpeed * CameraForwardZ;

	if(currentKey == 'a')
		cameraPosition -= glm::normalize(glm::cross(CameraForwardZ, CameraUpY)) * cameraSpeed;

	if(currentKey == 'd')
		cameraPosition += glm::normalize(glm::cross(CameraForwardZ, CameraUpY)) * cameraSpeed;



	//Transform object
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // place object at center of viewport
	model = glm::rotate(model, 45.0f, glm::vec3(1.0f, 1.0f, 1.0f)); // rotates object 45 degrees on xyz
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); // Increase scale of object by scale of 2

	//transform camera
	glm::mat4 view;
	view = glm::lookAt(CameraForwardZ, cameraPosition, CameraUpY);

	//Create perspective projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);

	//Retrieves and passes transform matrices to the shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glutPostRedisplay();

	glDrawElements(GL_TRIANGLES, 96, GL_UNSIGNED_INT, 0); //Draw triangles

	glBindVertexArray(0); // Deactivate vertex array object

	glutSwapBuffers(); // Flips back buffer with front buffer every frame
}

/*Create Shader program*/
void UCreateShader() {
	//Vertex Shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // create vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // attach vertex shader to source code
	glCompileShader(vertexShader); // compile vertex shader

	//Fragment Shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // create fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // attach fragment shader to source code
	glCompileShader(fragmentShader); // compile fragment shader

	//Shader program
	shaderProgram = glCreateProgram(); // create shader program and return ID
	glAttachShader(shaderProgram, vertexShader); // attach vertex shader to program
	glAttachShader(shaderProgram, fragmentShader); // attach fragment shader to program
	glLinkProgram(shaderProgram); // link vertex and fragment shaders to program

	//Delete vertex and fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

/*Create buffer and array objects*/
void UCreateBuffers() {
	//Position and color data
	GLfloat vertices[] = {
			//Positions			        //Color (red)
			0.5f, 0.1f, 0.0f, 		1.0f, 0.0f, 0.0f, // Top Right back of seat
			0.5f, 0.1f, 0.0f, 		1.0f, 0.0f, 0.0f, // Bottom Right back of seat
			-0.5f, 0.0f, 0.0f,		1.0f, 0.0f, 0.0f, // Bottom Right front of seat
			-0.5f, 0.1f, 0.0f, 		1.0f, 0.0f, 0.0f, // Top Right front of seat

			0.5f, 0.0f, -1.0f,		1.0f, 0.0f, 0.0f, // Bottom Left back of seat
			0.5f, 0.1f, -1.0f, 		1.0f, 0.0f, 0.0f, // Top Left back of seat
			-0.5f, 0.1f, -1.0f,		1.0f, 0.0f, 0.0f, // Top Left front of seat
			-0.5f, 0.0f, -1.0f,		1.0f, 0.0f, 0.0f, // Bottom Left front of seat

			0.5f, 1.5f, 0.0f, 		1.0f, 0.0f, 0.0f, // Top Right back of chair front
			0.5f, 1.5f, -1.0f, 		1.0f, 0.0f, 0.0f, // Top Left back of chair front
			0.6f, 1.5f, 0.0f, 		1.0f, 0.0f, 0.0f, // Top Right back of chair back
			0.6f, 1.5f, -1.0f, 		1.0f, 0.0f, 0.0f, // Top Left back of chair back
			0.6f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, // Bottom Right back of chair back
			0.6f, 0.0f, -1.0f, 		1.0f, 0.0f, 0.0f, // Bottom Left back of chair back

			// Chair Legs
			0.5f, -1.0f, -1.0f,		1.0f, 0.0f, 0.0f,
			0.6f, -1.0f, -1.0f, 	1.0f, 0.0f, 0.0f,

			0.5f, -1.0f, 0.0f, 		1.0f, 0.0f, 0.0f,
			0.6f, -1.0f, 0.0f, 		1.0f, 0.0f, 0.0f,

			-0.4f, 0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
			-0.5f, -1.0f, 0.0f, 	1.0f, 0.0f, 0.0f,
			-0.4f, -1.0f, 0.0f,		1.0f, 0.0f, 0.0f,

			-0.4f, 0.0f, -1.0f,		1.0f, 0.0f, 0.0f,
			-0.5f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f,
			-0.4f, -1.0f, -1.0f, 	1.0f, 0.0f, 0.0f,

	};

	//Index data to share position data
	GLuint indices[] = {

			//Seat of Chair
			0, 1, 3,
			1, 2, 3,
			0, 1, 4,
			0, 4, 5,
			0, 5, 6,
			0, 3, 6,
			4, 5, 6,
			4, 6, 7,
			2, 3, 6,
			2, 6, 7,
			1, 4, 7,
			1, 2, 7,

			//Back of Chair
			1, 4, 8,
			4, 8, 9,
			8, 9, 10,
			9, 10, 11,
			1, 12, 13,
			1, 4, 13,
			10, 12, 13,
			10, 11, 13,
			1, 8, 10,
			1, 10, 12,
			4, 9, 11,
			4, 11, 13,

			//Chair Leg (back left)
			4, 13, 14,
			13, 14, 15,

			//Chair Leg (back right)
			1, 12, 16,
			12, 16, 17,

			//Chair Leg (front right)
			2, 18, 19,
			18, 19, 20,

			//Chair Leg (front left)
			7, 21, 22,
			21, 22, 23,
	};

	//Generate buffer IDs
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO); // activate vertex array object before binding and setting any VBOs and Vertex Attribute Pointers

	//Activate VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy vertices to VBO

	//Activate EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // copy vertices to EBO

	//Set attribute pointer 0 to hold position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // enable vertex attribute

	//Set attribute pointer 1 to hold position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); // enable vertex attribute

	glBindVertexArray(0); // deactivates the VAO - good practice

}

//Implements the UKeyboard function
void UKeyboard(unsigned char key, GLint x, GLint y)
{
	switch(key){
	case 'w' :
		currentKey = key;
		cout<<"You pressed W!"<<endl;
		break;

	case 's' :
		currentKey = key;
		cout<<"You pressed S!"<<endl;
		break;

	case 'a' :
		currentKey = key;
		cout<<"You pressed A!"<<endl;
		break;

	case 'd' :
		currentKey = key;
		cout<<"You pressed D!"<<endl;
		break;

	default:
		cout<<"Press a key!"<<endl;
	}
}

//Implements the UKeyReleased function
void UKeyReleased(unsigned char key, GLint x, GLint y)
{
	cout <<"Key Released!"<<endl;
	currentKey = 0;
}


//Implements UMouseMove function
void UMouseMove(int x, int y) {
	//immediately replaces center locked coordinates with new mouse coordinates
	if(mouseDetected) {
		lastMouseX = x;
		lastMouseY = y;
		mouseDetected = false;
	}

		//Get direction mouse was moved in x and y
		mouseXOffset = x - lastMouseX;
		mouseYOffset = lastMouseY - y; // Inverted y

		//Updates with new mouse coordinates
		lastMouseX = x;
		lastMouseY = y;

		//Applies sensitivity to mouse direction
		mouseXOffset *= sensitivity;
		mouseYOffset *= sensitivity;

		//accumulates yaw and pitch variables
		yaw += mouseXOffset;
		pitch += mouseYOffset;

		//Orbits around center
		front.x = 10.0f * cos(yaw);
		front.y = 10.0f * sin(pitch);
		front.z = sin(yaw) * cos(pitch) * 10.0f;
}


