// 这行代码应该是红色！
#include "include/Angel.h"
#include "include/TriMesh.h"

#include <vector>
#include <string>

// 常量定义和全局变量
const int X_AXIS = 0;
const int Y_AXIS = 1;
const int Z_AXIS = 2;
const int TRANSFORM_SCALE = 0;
const int TRANSFORM_ROTATE = 1;
const int TRANSFORM_TRANSLATE = 2;
const double DELTA_DELTA = 0.3;
const double DEFAULT_DELTA = 0.5;

double scaleDelta = DEFAULT_DELTA;
double rotateDelta = DEFAULT_DELTA;
double translateDelta = DEFAULT_DELTA;
glm::vec3 scaleTheta(1.0, 1.0, 1.0);
glm::vec3 rotateTheta(0.0, 0.0, 0.0);
glm::vec3 translateTheta(0.0, 0.0, 0.0);
int currentTransform = TRANSFORM_ROTATE;
int mainWindow;
bool isAnimating = true;
int rotationAxis = Y_AXIS;

struct openGLObject {
	GLuint vao, vbo, program;
	std::string vshader, fshader;
	GLuint pLocation, cLocation, matrixLocation;
};
openGLObject model_object;
TriMesh* mesh = new TriMesh();

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void bindObjectAndData(TriMesh* mesh, openGLObject& object, const std::string& vshader, const std::string& fshader) {
    glGenVertexArrays(1, &object.vao);
	glBindVertexArray(object.vao);
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
	glBufferData(GL_ARRAY_BUFFER,
		mesh->getPoints().size() * sizeof(glm::vec3) + mesh->getColors().size() * sizeof(glm::vec3),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->getPoints().size() * sizeof(glm::vec3), &mesh->getPoints()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, mesh->getPoints().size() * sizeof(glm::vec3), mesh->getColors().size() * sizeof(glm::vec3), &mesh->getColors()[0]);
	object.vshader = vshader;
	object.fshader = fshader;
	object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->getPoints().size() * sizeof(glm::vec3)));
	object.matrixLocation = glGetUniformLocation(object.program, "matrix");
}

void init()
{
	std::string vshader, fshader;
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";

    if (!mesh->readOff("Models/cow.off")) { // <--- 确保文件名和路径正确
        printf("Failed to read OFF file. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    
    // 使用XYZ坐标映射来生成丰富的渐变色
    mesh->setPositionalColor();

    // 准备好顶点和颜色数据
    mesh->storeFacesPoints();

	bindObjectAndData(mesh, model_object, vshader, fshader);

	// 设置背景色
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(model_object.program);
    glBindVertexArray(model_object.vao);
	glm::mat4 m(1.0);
	m = glm::translate(m, translateTheta);
	m = glm::rotate(m, glm::radians(rotateTheta.x), glm::vec3(1,0,0));
	m = glm::rotate(m, glm::radians(rotateTheta.y), glm::vec3(0,1,0));
	m = glm::rotate(m, glm::radians(rotateTheta.z), glm::vec3(0,0,1));
	m = glm::scale(m, scaleTheta);
	glUniformMatrix4fv(model_object.matrixLocation, 1, GL_FALSE, glm::value_ptr(m));
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

void updateTheta(int axis, int sign) {
	switch (currentTransform) {
	case TRANSFORM_SCALE: scaleTheta[axis] += sign * scaleDelta; break;
	case TRANSFORM_ROTATE: rotateTheta[axis] += sign * rotateDelta; break;
	case TRANSFORM_TRANSLATE: translateTheta[axis] += sign * translateDelta; break;
	}
}
void resetTheta() {
	scaleTheta = glm::vec3(1.0, 1.0, 1.0);
	rotateTheta = glm::vec3(0.0, 0.0, 0.0);
	translateTheta = glm::vec3(0.0, 0.0, 0.0);
	scaleDelta = DEFAULT_DELTA;
	rotateDelta = DEFAULT_DELTA;
	translateDelta = DEFAULT_DELTA;
}
void updateDelta(int sign) {
	switch (currentTransform) {
	case TRANSFORM_SCALE: scaleDelta += sign * DELTA_DELTA; break;
	case TRANSFORM_ROTATE: rotateDelta += sign * DELTA_DELTA; break;
	case TRANSFORM_TRANSLATE: translateDelta += sign * DELTA_DELTA; break;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT) { return; }
	switch (key) {	
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
		case GLFW_KEY_1: currentTransform = TRANSFORM_SCALE; break;
		case GLFW_KEY_2: currentTransform = TRANSFORM_ROTATE; break;
		case GLFW_KEY_3: currentTransform = TRANSFORM_TRANSLATE; break;
		case GLFW_KEY_4: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
		case GLFW_KEY_5: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
		case GLFW_KEY_Q: updateTheta(X_AXIS, 1); break;
		case GLFW_KEY_A: updateTheta(X_AXIS, -1); break;
		case GLFW_KEY_W: updateTheta(Y_AXIS, 1); break;
		case GLFW_KEY_S: updateTheta(Y_AXIS, -1); break;
		case GLFW_KEY_E: updateTheta(Z_AXIS, 1); break;
		case GLFW_KEY_D: updateTheta(Z_AXIS, -1); break;
		case GLFW_KEY_R: updateDelta(1); break;
		case GLFW_KEY_F: updateDelta(-1); break;
		case GLFW_KEY_T: resetTheta(); break;
        case GLFW_KEY_X: rotationAxis = X_AXIS; break;
        case GLFW_KEY_Y: rotationAxis = Y_AXIS; break;
        case GLFW_KEY_Z: rotationAxis = Z_AXIS; break;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: isAnimating = true; break;
            case GLFW_MOUSE_BUTTON_RIGHT: isAnimating = false; break;
        }
    }
}

void printHelp() {
	printf("%s\n\n", "3D Model Reader and Controller");
	printf("Mouse options:\n");
    printf("Left Button:  Start / Resume animation\n");
    printf("Right Button: Pause animation\n\n");
	printf("Keyboard options:\n");
    printf("x/y/z:        Set rotation axis to X, Y, or Z\n");
	printf("1/2/3:        Switch transform mode (Scale/Rotate/Translate)\n");
	printf("q/a:          Increase/Decrease X value\n");
	printf("w/s:          Increase/Decrease Y value\n");
	printf("e/d:          Increase/Decrease Z value\n");
	printf("r/f:          Increase/Decrease delta of current transform\n");
	printf("t:            Reset all transformations\n");
    printf("4/5:          Switch draw mode (Line/Fill)\n");
    printf("ESC:          Exit\n");
}

void cleanData() {
	mesh->cleanData();
	delete mesh;
	mesh = NULL;
    glDeleteVertexArrays(1, &model_object.vao);
	glDeleteBuffers(1, &model_object.vbo);
	glDeleteProgram(model_object.program);
}

int main(int argc, char** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	GLFWwindow* window = glfwCreateWindow(800, 800, "2023044007_ZhongQimin_Lab2", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	init();
	printHelp();
	glEnable(GL_DEPTH_TEST);
	
	while (!glfwWindowShouldClose(window))
	{
        if (isAnimating) {
            rotateTheta[rotationAxis] += rotateDelta;
            if (rotateTheta[rotationAxis] > 360.0f) {
                rotateTheta[rotationAxis] -= 360.0f;
            }
        }
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	cleanData();
	return 0;
}// 这是在dev分支上增加的注释
