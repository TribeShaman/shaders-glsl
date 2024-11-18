#include<glad/gl.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<linmath.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

class PointLight
{
public:
	glm::vec3 position;
	glm::vec3 color;
	float constant;
	float linear;
	float quadratic;

	PointLight(float Constant, float Linear, float Quadratic)
	{
		position = glm::vec3(1.2f, 1.0f, 2.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		constant = Constant;
		linear = Linear;
		quadratic = Quadratic;
	}

};
PointLight light(1.0f, 0.09f, 0.032f);
bool controlLight = false; // false - sterowanie kamer¹, true - sterowanie œwiat³em


// Funkcje i zmienne globalne
vec3 cameraPosition = { 0.0f, 0.0f, 3.0f }; // Pocz¹tkowa pozycja kamery
float yaw = -90.0f; // Pocz¹tkowy obrót kamery w osi Y
float pitch = 0.0f; // Pocz¹tkowy obrót kamery w osi X
float FOV = 45.0f; // Pocz¹tkowy FOV
float cameraSpeed = 0.05f; // Prêdkoœæ poruszania siê kamery
float sensitivity = 0.1f; // Czu³oœæ myszy
float aspectRatio = 800.0f / 800.0f;
vec3 cameraFront = { 0.0f, 0.0f, -1.0f }; // Kierunek patrzenia kamery
vec3 cameraUp = { 0.0f, 1.0f, 0.0f }; // Wektor góry
vec3 cameraRight = { 1.0f, 0.0f, 0.0f }; // Wektor prawo

void mat4x4_perspective_manual(mat4x4 M, float fov, float aspect, float near, float far)
{
	float tanHalfFov = tanf(fov / 2.0f);

	mat4x4_identity(M);

	M[0][0] = 1.0f / (aspect * tanHalfFov);  // Skala w osi X
	M[1][1] = 1.0f / tanHalfFov;            // Skala w osi Y
	M[2][2] = -(far + near) / (far - near); // Przesuniêcie w osi Z (normalizacja Z)
	M[2][3] = -1.0f;                        // Wspó³czynnik perspektywy
	M[3][2] = -(2.0f * far * near) / (far - near); // Translacja w osi Z
	M[3][3] = 0.0f;                         // Ustawienie ostatniego wiersza na 0
}

void mat4x4_view_manual(mat4x4 view, vec3 pos, vec3 target, vec3 up) {
	vec3 f, s, u;
	vec3_sub(f, target, pos);
	vec3_norm(f, f);

	vec3_mul_cross(s, f, up);
	vec3_norm(s, s);

	vec3_mul_cross(u, s, f);

	mat4x4_identity(view);
	view[0][0] = s[0];
	view[1][0] = s[1];
	view[2][0] = s[2];
	view[0][1] = u[0];
	view[1][1] = u[1];
	view[2][1] = u[2];
	view[0][2] = -f[0];
	view[1][2] = -f[1];
	view[2][2] = -f[2];
	view[3][0] = -vec3_mul_inner(s, pos);
	view[3][1] = -vec3_mul_inner(u, pos);
	view[3][2] = vec3_mul_inner(f, pos);
}


// Funkcja callback dla myszy
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static bool firstMouse = true;
	static float lastX = 400.0f, lastY = 300.0f; // Œrodek ekranu
	float xOffset, yOffset;

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	xOffset = xpos - lastX;
	yOffset = lastY - ypos; // Odwrócone Y
	lastX = xpos;
	lastY = ypos;

	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	// Ograniczanie k¹ta patrzenia
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	// Aktualizacja front
	cameraFront[0] = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
	cameraFront[1] = sin(pitch * M_PI / 180.0f);
	cameraFront[2] = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
}

// Funkcja obs³ugi wejœcia klawiatury
void processInput(GLFWwindow* window, float deltaTime) {
	static bool keyPressed = false;

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !keyPressed) {
		controlLight = !controlLight; // Prze³¹cz tryb
		keyPressed = true;
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
		keyPressed = false;
	}

	float movementSpeed = 2.5f * deltaTime;
	vec3 offset;

	if (!controlLight) {
		// Sterowanie kamer¹
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			vec3_scale(offset, cameraFront, movementSpeed);
			vec3_add(cameraPosition, cameraPosition, offset);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			vec3_scale(offset, cameraFront, -movementSpeed);
			vec3_add(cameraPosition, cameraPosition, offset);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			vec3 right;
			vec3_mul_cross(right, cameraFront, cameraUp);
			vec3_norm(right, right);
			vec3_scale(offset, right, -movementSpeed);
			vec3_add(cameraPosition, cameraPosition, offset);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			vec3 right;
			vec3_mul_cross(right, cameraFront, cameraUp);
			vec3_norm(right, right);
			vec3_scale(offset, right, movementSpeed);
			vec3_add(cameraPosition, cameraPosition, offset);
		}
	}
	else {
		// Sterowanie œwiat³em
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			light.position.z -= movementSpeed; // W kierunku osi -Z
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			light.position.z += movementSpeed; // W kierunku osi +Z
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			light.position.x -= movementSpeed; // W kierunku osi -X
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			light.position.x += movementSpeed; // W kierunku osi +X
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			light.position.y += movementSpeed; // W kierunku osi +Y
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			light.position.y -= movementSpeed; // W kierunku osi -Y
		}
	}
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
		FOV -= 0.1f;
		if (FOV < 10.0f) FOV = 10.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
		FOV += 0.1f;
		if (FOV > 120.0f) FOV = 120.0f;
	}
}
int main()
{
	std::string _vertexShaderDiffuse;
	std::string _fragmentShaderDiffuse;
	std::string _vertexShaderSpecular;
	std::string _frgamentShaderSpecular;
	std::string _vertexShaderTexture;
	std::string _frgamentShaderTexture;
	std::string _vertexShaderLambert;
	std::string _fragmentShaderLambert;

	std::ifstream inputVertexDiffuse("shaders/vertexShaderDiffuse.txt");
	std::ifstream inputFragmentDiffuse("shaders/fragmentShaderDiffuse.txt");
	std::ifstream inputVertexSpecular("shaders/vertexShaderSpecular.txt");
	std::ifstream inputFragmentSpecular("shaders/fragmentShaderSpecular.txt");
	std::ifstream inputVertexTexture("shaders/vertexShaderTexture.txt");
	std::ifstream inputFragmentTexture("shaders/fragmentShaderTexture.txt");
	std::ifstream inputVertexLambert("shaders/vertexShaderLambert.txt");
	std::ifstream inputFragmentLambert("shaders/fragmentShaderLambert.txt");

	if (!(inputVertexDiffuse.is_open() && inputFragmentDiffuse.is_open())) {
		std::cerr << "Nie mo¿na otworzyæ plików shaderów Diffuse!" << std::endl;
		return 1;
	}

	if (!(inputVertexSpecular.is_open() && inputFragmentSpecular.is_open())) {
		std::cerr << "Nie mo¿na otworzyæ plików shaderów Specular!" << std::endl;
		return 1;
	}

	if (!(inputVertexTexture.is_open() && inputFragmentTexture.is_open())) {
		std::cerr << "Nie mo¿na otworzyæ plików shaderów Texture!" << std::endl;
		return 1;
	}

	if (!(inputVertexLambert.is_open() && inputFragmentLambert.is_open())) {
		std::cerr << "Nie mo¿na otworzyæ plików shaderów Lambert!" << std::endl;
		return 1;
	}

	std::string line;
	while (std::getline(inputVertexDiffuse, line)) {
		_vertexShaderDiffuse += line + "\n";
	}

	while (std::getline(inputFragmentDiffuse, line)) {
		_fragmentShaderDiffuse += line + "\n";
	}

	while (std::getline(inputVertexSpecular, line)) {
		_vertexShaderSpecular += line + "\n";
	}

	while (std::getline(inputFragmentSpecular, line)) {
		_frgamentShaderSpecular += line + "\n";
	}

	while (std::getline(inputVertexTexture, line)) {
		_vertexShaderTexture += line + "\n";
	}

	while (std::getline(inputFragmentTexture, line)) {
		_frgamentShaderTexture += line + "\n";
	}

	while (std::getline(inputVertexLambert, line)) {
		_vertexShaderLambert += line + "\n";
	}

	while (std::getline(inputFragmentLambert, line)) {
		_fragmentShaderLambert += line + "\n";
	}

	inputVertexDiffuse.close();
	inputFragmentDiffuse.close();
	inputVertexSpecular.close();
	inputFragmentSpecular.close();
	inputVertexTexture.close();
	inputFragmentTexture.close();
	inputVertexLambert.close();
	inputFragmentLambert.close();

	const char* vertexShaderDiffuse = _vertexShaderDiffuse.c_str();
	const char* fragmentShaderDiffuse = _fragmentShaderDiffuse.c_str();
	const char* vertexShaderSpecular = _vertexShaderSpecular.c_str();
	const char* fragmentShaderSpecular = _frgamentShaderSpecular.c_str();
	const char* vertexShaderTexture = _vertexShaderTexture.c_str();
	const char* fragmentShaderTexture = _frgamentShaderTexture.c_str();
	const char* vertexShaderLambert = _vertexShaderLambert.c_str();
	const char* fragmentShaderLambert = _fragmentShaderLambert.c_str();



	//inicjalizacja glfw
	glfwInit();
	//ustawianie wersji maksymalnej i minimalnej glfw oraz ustawianie profilu (core zawiera tylko aktualne komendy)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLfloat vertices[] = {
		// Przednia œciana
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  // dolny lewy
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  // dolny prawy
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  // górny prawy
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  // górny lewy

		// Tylna œciana
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,  // dolny lewy
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,  // dolny prawy
		 0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,  // górny prawy
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,  // górny lewy

		// Lewa œciana
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // dolny tylny
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // dolny przedni
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // górny przedni
		-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // górny tylny

		// Prawa œciana
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // dolny tylny
		 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // dolny przedni
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // górny przedni
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // górny tylny

		 // Dolna œciana
		 -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  // lewy tylny
		  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  // prawy tylny
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  // prawy przedni
		 -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  // lewy przedni

		 // Górna œciana
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  // lewy tylny
		  0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  // prawy tylny
		  0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  // prawy przedni
		 -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f   // lewy przedni
	};



	GLuint indices[] = {
		0, 1, 2,   0, 2, 3,   // Przednia œciana
		4, 5, 6,   4, 6, 7,   // Tylna œciana
		8, 9, 10,  8, 10, 11, // Lewa œciana
		12, 13, 14, 12, 14, 15, // Prawa œciana
		16, 17, 18, 16, 18, 19, // Dolna œciana
		20, 21, 22, 20, 22, 23  // Górna œciana
	};
	vec3 cubePositions[5] = {
	{ 0.0f, 0.0f, 0.0f },
	{ 1.5f, 0.0f, 0.0f },
	{ 3.0f, 0.0f, 0.0f },
	{ 4.5f, 0.0f, 0.0f },
	{ 6.0f, 0.0f, 0.0f }
	};


	//tworzenie okna 
	GLFWwindow* window = glfwCreateWindow(800, 800, "learning opengl", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//informacja ¿e teraz przetwaramy akurat nasze okno 
	glfwMakeContextCurrent(window);
	//przekazenie do glad uzywanej wersji opengl
	gladLoadGL(glfwGetProcAddress);
	glfwSetCursorPosCallback(window, mouse_callback); // Rejestracja callbacku myszy
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Ukrycie kursora


	//ustawienie obszaru okna ktore bedzie przetwarzac opengl (tu cale od lewego górnego rogu a¿ do prawego dolnego)
	glViewport(0, 0, 800, 800);
	//shadery diffuse
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderDiffuse, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderDiffuse, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgramDiffuse = glCreateProgram();
	glAttachShader(shaderProgramDiffuse, vertexShader);
	glAttachShader(shaderProgramDiffuse, fragmentShader);
	glLinkProgram(shaderProgramDiffuse);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//shadery specular
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSpecular, NULL);
	glCompileShader(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSpecular, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgramSpecular = glCreateProgram();
	glAttachShader(shaderProgramSpecular, vertexShader);
	glAttachShader(shaderProgramSpecular, fragmentShader);
	glLinkProgram(shaderProgramSpecular);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//shadery texture
	GLuint vertexShaderTex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderTex, 1, &vertexShaderTexture, NULL);
	glCompileShader(vertexShaderTex);

	GLuint fragmentShaderTex = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderTex, 1, &fragmentShaderTexture, NULL);
	glCompileShader(fragmentShaderTex);

	GLuint shaderProgramTexture = glCreateProgram();
	glAttachShader(shaderProgramTexture, vertexShaderTex);
	glAttachShader(shaderProgramTexture, fragmentShaderTex);
	glLinkProgram(shaderProgramTexture);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//shadery lambert
	GLuint vertexShaderLam = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderLam, 1, &vertexShaderLambert, NULL);
	glCompileShader(vertexShaderLam);

	GLuint fragmentShaderLam = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderLam, 1, &fragmentShaderLambert, NULL);
	glCompileShader(fragmentShaderLam);

	GLuint shaderProgramLambert = glCreateProgram();
	glAttachShader(shaderProgramLambert, vertexShaderLam);
	glAttachShader(shaderProgramLambert, fragmentShaderLam);
	glLinkProgram(shaderProgramLambert);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Vertex Array Object gromadzi kilka VBO i pozwala siê miêdzy nimi prze³¹czaæ
	GLuint VAO;
	// tworzymy VAO i wskazujemy ¿e mamy tylko jeden obiekt
	glGenVertexArrays(1, &VAO);
	//bindowanie vertex array
	glBindVertexArray(VAO);
	//Vertex Buffer Object to kontener do przes³ania wierzcho³ków do GPU (normalnie by³a by to lista ale mamy tu tylko jeden obiekt)
	// zmienna VBO to referencja na ten kontener

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	GLuint VBO;
	//stworzenie buforu który bêdziemy przesy³aæ(1 bo jeden obiekt a potem wskazujemy na referencje)
	glGenBuffers(1, &VBO);
	// bindowanie bufforu, czyli ustawienie faktu ¿e modyfikuj¹c dany tym buforu modyfikujemy przez nas wskazany
	//bindujemy do array buffer bo ten bufor odpowiada wierzcho³kom
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//buforujemy ju¿ konkretne dane wzkazuj¹c na typ buforu, wielkoœæ danych przesy³anych w bajtach i konkretne dane które buforujemy, i ustawiamy do czego bêdziemy u¿ywaæ 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Definiowanie sposobu interpretacji danych w buforze przez shadery
	//numer indeksu atrybutu wierzcho³ka(okreœlany w shaderze), liczba komponentów(wierzcho³ków), typ w jakim s¹ komponenty, czy znormalizowane, odleg³oœci miedzy kolejnymi zestawami danych
	//(np suma bajtów potrzebnych do przechowania 3 wierzcho³ków czyli jednego trójk¹ta), miejsce zaczêcia siê danych danego artybutu w buforze (czyli jak mamy w dancyh trójk¹ta
	// i wierzcho³ki i kolor to wiercho³ki s¹ na pocz¹tku linii czyli 0 a kolor nastêpny czyli zaczyna siê od sizeof(iloœæ bajtów do przechowania danych wierzcho³ków)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	//udostêpniamy podaj¹c indeks naszego VBO czyli pierwszy zatem 0
	glEnableVertexAttribArray(0);

	//atrybut koloru
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//atrybut normali
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);


	//odbindowujemy wszstkie bufory( u nas VBO) od array buffer ¿eby przypadkowo przy pó¿niejszym jego edytowaniu nie edytowaæ innych buforów których byœmy nie chcieli
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//to samo dla vertex array
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// ³adowanie textur
	GLuint texture = 0; // Domyœlna wartoœæ
	int width, height, nrChannels;

	unsigned char* data = stbi_load("textures/wood_texture.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Failed to load texture: textures/wood_texture.jpg" << std::endl;
		// Jeœli tekstura jest krytyczna dla dzia³ania programu, zakoñcz program
		glfwTerminate();
		return -1;
	}

	// Ustawienia tekstury
	if (texture != 0)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cerr << "Texture was not initialized!" << std::endl;
	}
	
	//ustawienie koloru t³a (clear), ustawienie naszego koloru na koloru na color tylnego buforu, zamiana buforów
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);

	float lastFrame = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window, deltaTime);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//modelLoc to lokalizacjia zmiennej "model" w programie shaderowym
		GLint modelLoc = glGetUniformLocation(shaderProgramDiffuse, "model");
		GLuint projectionLoc = glGetUniformLocation(shaderProgramDiffuse, "projection");
		GLint viewLoc = glGetUniformLocation(shaderProgramDiffuse, "view");

		mat4x4 view;
		vec3 target;
		vec3_add(target, cameraPosition, cameraFront);
		mat4x4_view_manual(view, cameraPosition, target, cameraUp);

		mat4x4 projection;
		mat4x4_perspective_manual(projection, FOV * (M_PI / 180.0f), aspectRatio, 0.1f, 100.0f);
		//rysowanie diffuse
		glUseProgram(shaderProgramDiffuse);

		glUniform3fv(glGetUniformLocation(shaderProgramDiffuse, "light.position"), 1, glm::value_ptr(light.position));
		glUniform3fv(glGetUniformLocation(shaderProgramDiffuse, "light.color"), 1, glm::value_ptr(light.color));
		glUniform1f(glGetUniformLocation(shaderProgramDiffuse, "light.constant"), light.constant);
		glUniform1f(glGetUniformLocation(shaderProgramDiffuse, "light.linear"), light.linear);
		glUniform1f(glGetUniformLocation(shaderProgramDiffuse, "light.quadratic"), light.quadratic);

		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);

		mat4x4 model;
		mat4x4_identity(model);
		mat4x4_translate_in_place(model, cubePositions[0][0], cubePositions[0][1], cubePositions[0][2]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
		glUniform4f(glGetUniformLocation(shaderProgramDiffuse, "overrideColor"), 0.0f, 0.0f, 0.0f, 0.0f);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		mat4x4_identity(model);
		mat4x4_translate_in_place(model, light.position.x, light.position.y, light.position.z);
		mat4x4_scale_aniso(model, model, 0.2f, 0.2f, 0.2f);
		glUniform4f(glGetUniformLocation(shaderProgramDiffuse, "overrideColor"), 1.0f, 1.0f, 1.0f, 0.3f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//rysowanie specular
		modelLoc = glGetUniformLocation(shaderProgramSpecular, "model");
		projectionLoc = glGetUniformLocation(shaderProgramSpecular, "projection");
		viewLoc = glGetUniformLocation(shaderProgramSpecular, "view");

		glUseProgram(shaderProgramSpecular);

		glUniform3fv(glGetUniformLocation(shaderProgramSpecular, "light.position"), 1, glm::value_ptr(light.position));
		glUniform3fv(glGetUniformLocation(shaderProgramSpecular, "light.color"), 1, glm::value_ptr(light.color));
		glUniform1f(glGetUniformLocation(shaderProgramSpecular, "shininess"), 50.0f);
		glUniform1f(glGetUniformLocation(shaderProgramSpecular, "dophong"), 0.0f);
		glm::vec3 cameraPositionGLM = glm::vec3(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
		glUniform3fv(glGetUniformLocation(shaderProgramSpecular, "ViewPos"), 1, glm::value_ptr(cameraPositionGLM));

		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);

		mat4x4_identity(model);
		mat4x4_translate_in_place(model, cubePositions[1][0], cubePositions[1][1], cubePositions[1][2]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		//rysowanie phong(ambient+specular+diffuse)
		mat4x4_identity(model);
		mat4x4_translate_in_place(model, cubePositions[2][0], cubePositions[2][1], cubePositions[2][2]);
		glUniform1f(glGetUniformLocation(shaderProgramSpecular, "dophong"), 1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//rysowanie texture
		modelLoc = glGetUniformLocation(shaderProgramTexture, "model");
		projectionLoc = glGetUniformLocation(shaderProgramTexture, "projection");
		viewLoc = glGetUniformLocation(shaderProgramTexture, "view");

		glUseProgram(shaderProgramTexture);
		glActiveTexture(GL_TEXTURE0); // Aktywacja jednostki tekstur
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shaderProgramTexture, "ourTexture"), 0);
		

		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);

		mat4x4_identity(model);
		mat4x4_translate_in_place(model, cubePositions[3][0], cubePositions[3][1], cubePositions[3][2]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//rysowanie lambert
		modelLoc = glGetUniformLocation(shaderProgramLambert, "model");
		projectionLoc = glGetUniformLocation(shaderProgramLambert, "projection");
		viewLoc = glGetUniformLocation(shaderProgramLambert, "view");

		glUseProgram(shaderProgramLambert);

		glUniform3fv(glGetUniformLocation(shaderProgramLambert, "light.position"), 1, glm::value_ptr(light.position));
		glUniform3fv(glGetUniformLocation(shaderProgramLambert, "light.color"), 1, glm::value_ptr(light.color));

		glActiveTexture(GL_TEXTURE0); // Aktywacja jednostki tekstur
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shaderProgramTexture, "ourTexture"), 0);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);
		mat4x4_identity(model);
		mat4x4_translate_in_place(model, cubePositions[4][0], cubePositions[4][1], cubePositions[4][2]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgramDiffuse);
	glDeleteProgram(shaderProgramSpecular);
	glDeleteProgram(shaderProgramTexture);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}