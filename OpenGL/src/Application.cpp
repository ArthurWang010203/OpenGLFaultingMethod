#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Vertex.h"
#include <cmath>
#include "Renderer.h"
#include <omp.h>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"

#include "VertexArray.h"

#include "Shader.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -3.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
size_t screenWidth = 960;
size_t screenHeight = 540;
float lastX = screenHeight / 2;
float lastY = screenWidth / 2;
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
float cameraSpeed = 0.05f;
float fov = 45.0f;
float cellWidth = 0.25f;
unsigned int meshWidth = 100;
unsigned int meshHeight = 100;
unsigned int faults = 100;

struct Geometry {
    float* vertices = nullptr;
    size_t numVertices = 0;
    unsigned int* indices = nullptr;
    size_t numIndices = 0;
};

auto processInput(GLFWwindow* window) -> void {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
}

auto mouse_callback(GLFWwindow* window, double xpos, double ypos) -> void {
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sens = 0.1f;
    xoffset *= sens;
    yoffset *= sens;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

auto scroll_callback(GLFWwindow* window, double xoffset, double yoffset) -> void {
    fov -= (float)yoffset*2.0f;
    if (fov < 1.0f) {
        fov = 1.0f;
    }
    if (fov > 90.0f) {
        fov = 90.0f;
    }
}

auto printFloatArray(float* arr, size_t size) {
    std::cout << "printing float array:\n";
    for (int i = 0; i < size; i+=6) {
        std::cout << arr[i] << "," << arr[i + 1] << "," << arr[i + 2] << "->" << arr[i+3] << "," << arr[i+4] << "," << arr[i+5] << std::endl;
    }
    std::cout << "\n\n";
}
auto printIntArray(unsigned int* arr, size_t size) {
    std::cout << "printing int array:\n";
    for (int i = 0; i < size; i += 3) {
        std::cout << arr[i] << "," << arr[i + 1] << "," << arr[i + 2] << "," << std::endl;
    }
    std::cout << "\n\n";
}

auto addFault(unsigned int vertexWidth, unsigned int vertexHeight, Geometry* geometry) -> void {
    unsigned int row = rand() % vertexHeight;
    unsigned int col = rand() % vertexWidth;
    glm::vec3 point = glm::vec3(col, row, 0);

    float degree = (float)(rand() % 360); // pick direction
    glm::vec3 normal_vector = glm::vec3(cos(glm::radians(degree)), sin(glm::radians(degree)), 0);

    float distance = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    int vertexIdx = 0;
    for (int i = 0; i < vertexHeight; i++) {
        for (int j = 0; j < vertexWidth; j++) {
            int side = glm::dot(glm::vec3(j, i, 0) - point,normal_vector);
            if (side >= 0) {
                geometry->vertices[vertexIdx + 1] += distance;
            }
            else {
                geometry->vertices[vertexIdx + 1] -= distance;
            }
            vertexIdx+=6;
        }
    }
}

auto faultingMethod(unsigned int vertexWidth, unsigned int vertexHeight, Geometry* geometry, size_t times) {
#pragma omp parallel for reduction(+:geometry->vertices)
    for (int i = 0; i < times; i++) {
        addFault(vertexHeight,vertexHeight,geometry);
    }
}

auto createGeometry(unsigned int width, unsigned int height, unsigned int cuts, unsigned int weathering) -> Geometry* {
    Geometry* rt = new Geometry();
    // width = number of cells for the width
    // height = number of cells for the height
    
    size_t vertexWidth = width + 1;
    size_t vertexHeight = height + 1;
    size_t size = (vertexWidth) * (vertexHeight) * (3*2); // n+1 vertices x n+1 vertices x 3*2 floats per vertex (position, normal)
    float* vertices = new float[size];
    memset(vertices, 0, sizeof(float) * size);
    int vertexIdx = 0;
    int idx = 0;
    float w_bound = (width / 2) * cellWidth;
    float h_bound = (height / 2) * cellWidth;
    for (int i = 0; i < size; i+=6) {
        vertices[i] = static_cast<float>((vertexIdx % vertexWidth) * cellWidth) - w_bound; // x coord
        //vertices[i+1] = rand() % 3; // y coord
        vertices[i+2] = static_cast<float>((vertexIdx / vertexWidth) * cellWidth) - h_bound; // z coord
        vertexIdx++;
    }

    rt->vertices = vertices;
    rt->numVertices = size;
   
    size_t numTriangles = height * width * 2;
    unsigned int* indices = new unsigned int[numTriangles * 3]; // n cells x n cells x 2 triangles per cell x 3 indices per triangle
    
    idx = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            indices[idx] = i * vertexHeight + j; // upper left corner
            indices[idx + 1] = i * vertexHeight + j + 1; // right one
            indices[idx + 2] = (i+1) * vertexHeight + j + 1; // right one, down one
            idx += 3;
            indices[idx] = i * vertexHeight + j; // upper left corner
            indices[idx + 1] = (i + 1) * vertexHeight + j + 1; // right one, down one
            indices[idx + 2] = (i + 1) * vertexHeight + j; // down one
            idx += 3;

        }
    }
    rt->indices = indices;
    rt->numIndices = numTriangles*3;

    faultingMethod(vertexWidth, vertexHeight, rt, cuts);

    for (int i = 0; i < numTriangles; i++) {
        unsigned int triangleIdx = i * 3;
        float* A = &vertices[indices[triangleIdx]*6];
        float* B = &vertices[indices[triangleIdx + 1]*6];
        float* C = &vertices[indices[triangleIdx + 2]*6];
        glm::vec3 _A = glm::vec3(A[0], A[1], A[2]);
        glm::vec3 _B = glm::vec3(B[0], B[1], B[2]);
        glm::vec3 _C = glm::vec3(C[0], C[1], C[2]);
        glm::vec3 L1 = _B - _A;
        glm::vec3 L2 = _C - _A;
        glm::vec3 Norm = glm::normalize(glm::cross(L1,L2));
        for (int j = 3; j < 6; j++) {
            A[j] = -Norm[j-3];
            B[j] = -Norm[j-3];
            C[j] = -Norm[j-3];
        }
    }

    return rt;
}

auto runGLFW() -> int {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screenWidth, screenHeight, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cout << "glew init error" << std::endl;
    }
    {
        /* // 1x1x1 cube
        float positions[48] = {
            -0.5f, -0.5f, -0.5f,0.0f,1.0f,0.0f,
            -0.5f, -0.5f, 0.5f,0.0f,1.0f,0.0f,
            -0.5f, 0.5f, -0.5f,1.0f,0.0f,0.0f,
            -0.5f,0.5f, 0.5f,1.0f,0.0f,0.0f,
            0.5f,-0.5f,-0.5f,-1.0f,0.0f,0.0f,
            0.5f,-0.5f,0.5f,-1.0f,0.0f,0.0f,
            0.5f,0.5f,-0.5f,0.0f,0.0f,1.0f,
            0.5f,0.5f,0.5f,0.0f,0.0f,1.0f,
        };*/
        
        Geometry* myGeom = createGeometry(meshWidth, meshHeight, faults, 0);
        //printFloatArray(myGeom->vertices, myGeom->numVertices);
        //printIntArray(myGeom->indices, myGeom->numIndices);
        /* // 1x1x1 cube
        unsigned int indices[] = {
            0,1,2,
            2,3,1,
            0,1,5,
            4,5,1,
            4,5,6,
            5,6,7,
            7,3,2,
            7,2,6,
            1,3,7,
            5,1,7,
            0,2,4,
            2,4,6
        };*/

        

        unsigned int vao;
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));

        VertexArray va;
        VertexBuffer vb(myGeom->vertices, myGeom->numVertices * sizeof(float));
        //VertexBuffer vb(positions, 48 * sizeof(float)); // 1x1x1 cube
        VertexBufferLayout layout;
        layout.Push<float>(3); // positions
        layout.Push<float>(3); // normals
        va.AddBuffer(vb, layout); 

        IndexBuffer ib(myGeom->indices, myGeom->numIndices); // inits and binds 36 indices
        //IndexBuffer ib(indices, 36); // inits and binds 36 indices // 1x1x1 cube

        Shader shader("res/shaders/Basic.shader");

        shader.Bind();
        shader.SetUniform4f("u_Color", 1.0, 0.0, 0.0, 0.0);

        float r = 0.0f;
        float increment = 0.01f;

        va.Unbind();
        shader.Unbind();
        vb.Unbind();
        ib.Unbind();

        Renderer renderer;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            renderer.Clear();

            processInput(window);

            glm::mat4 view;
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 0.0f);

            glm::mat4 proj = glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
            glm::mat4 mvp = proj * view;
            shader.SetUniformMat4f("u_MVP", mvp);

            renderer.Draw(va, ib, shader);

            if (r > 1.0f) {
                increment = -0.01f;
            }
            else if (r < 0.0f) {
                increment = 0.01f;
            }

            r += increment;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

    }
    glfwTerminate();
    return 0;
}

int main(int argc, char** argv)
{
    srand((unsigned)time(0));
    if (argc < 3) {
        std::cout << "Usage: ./main [MESH_WIDTH] [MESH_HEIGHT] [# of faults]" << std::endl;
        return -1;
    }
    meshWidth = std::atoi(argv[1]);
    meshHeight = std::atoi(argv[2]);
    faults = std::atoi(argv[3]);
    std::cout << "Constructing a terrain with dimensions " << meshWidth << " x " << meshHeight << " with " << faults << " faults" << std::endl;

    runGLFW();
    return 0;
}