#include "shader_h.h"
#include "stb_image_loader.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int w, int h);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int orthogonal_mode = 0;
int show_triangles = 0;
float scaleFactor = 0.4;
float x_angle = 0.0;
float y_angle = 0.0;
float z_angle = 0.0;
float cam_x = 0.0;
float cam_y = 0.0;
float cam_z = -3.0;
float cam_angle_x = 0.0;
float cam_angle_y = 0.0;

int main(int argc, char** argv){
    // Set up GLFW window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // Create window object
    GLFWwindow* window = glfwCreateWindow(800, 600, "Enter to spin", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport size and callback function
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);

    // these here are the coords for each vertex of a regular dodecahedron
    // with unit side length.
    const float phi = (1 + sqrtf(5)) / 2;
    float vertices[] = {
        // cube
        -1, -1, -1,
        -1, -1, 1,
        -1, 1, -1,
        -1, 1, 1,
        1, -1, -1,
        1, -1, 1,
        1, 1, -1,
        1, 1, 1,
        // yz-plane
        0, -phi, -(1/phi),
        0, -phi, (1/phi),
        0, phi, -(1/phi),
        0, phi, (1/phi),
        // xz-plane
        -(1/phi), 0, -phi,
        -(1/phi), 0, phi,
        (1/phi), 0, -phi,
        (1/phi), 0, phi,
        // xy-plane
        -phi, -(1/phi), 0,
        -phi, (1/phi), 0,
        phi, -(1/phi), 0,
        phi, (1/phi), 0,
    };
    // these are the indices for coords to triangulate the dodecahedron
    // each pentagon is made into 3 triangles, so 3 * 12 = 36 triangles total
    // a "triplet" is 3 pentagons created around a single vertex
    u_int indices[] = {
        // Triplet 1
        5, 9, 8,
        5, 18, 4,
        5, 8, 4,

        5, 15, 13,
        5, 9, 1,
        5, 1, 13,

        5, 18, 19,
        5, 15, 7,
        5, 19, 7,

        // Triplet 2
        0, 12, 14,
        0, 8, 4,
        0, 14, 4,

        0, 16, 17,
        0, 12, 2,
        0, 17, 2,

        0, 8, 9,
        0, 16, 1,
        0, 9, 1,

        // Triplet 3
        6, 19, 18,
        6, 14, 4,
        6, 18, 4,

        6, 14, 12,
        6, 10, 2,
        6, 12, 2,

        6, 10, 11,
        6, 19, 7,
        6, 11, 7,

        // Triplet 4
        3, 17, 16,
        3, 13, 1,
        3, 16, 1,

        3, 13, 15,
        3, 11, 7,
        3, 15, 7,

        3, 11, 10,
        3, 17, 2,
        3, 10, 2,
    };
    // these are indices to create lines around the perimeter of each pentagon
    u_int line_indices[] = {
        // triplet 1
        5, 9, 9, 8, 8, 4, 4, 18, 18, 5,
        5, 15, 15, 13, 13, 1, 1, 9, 9, 5,
        5, 18, 18, 19, 19, 7, 7, 15, 15, 5,
        // triplet 2
        0, 12, 12, 14, 14, 4, 4, 8, 8, 0,
        0, 16, 16, 17, 17, 2, 2, 12, 12, 0,
        0, 8, 8, 9, 9, 1, 1, 16, 16, 0,
        // triplet 3
        6, 19, 19, 18, 18, 4, 4, 14, 14, 6,
        6, 14, 14, 12, 12, 2, 2, 10, 10, 6,
        6, 10, 10, 11, 11, 7, 7, 19, 19, 6,
        // triplet 4
        3, 17, 17, 16, 16, 1, 1, 13, 13, 3,
        3, 13, 13, 15, 15, 7, 7, 11, 11, 3,
        3, 11, 11, 10, 10, 2, 2, 17, 17, 3,
    };

    // Vertex Buffer Object
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    // Element Buffer Object
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    unsigned int EBO2;
    glGenBuffers(1, &EBO2);

    // Vertex Array Object
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    // Bind stuff
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof line_indices, line_indices, GL_STATIC_DRAW);

    // Link vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Load shaders
    Shader sovietShader("./vertex_s.glsl", "./frag_s.glsl");

    // Use program
    sovietShader.use();

    // Create model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, glm::radians(cam_angle_x), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(cam_angle_y), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::translate(view, glm::vec3(cam_x, cam_y, cam_z));
        // Get aspect ratio
        GLint m_viewport[4];
        glGetIntegerv(GL_VIEWPORT, m_viewport);
        float ratio = (float) m_viewport[2] / m_viewport[3];
        // Projection
        glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)ratio, 0.1f, 100.0f);

        // Funky colors
        float timeVal = glfwGetTime() / 2;
        float greenVal = 0 / 255.0f;
        float redVal = 100 / 255.0f;
        float blueVal = 33.3 / 255.0f;
        sovietShader.setFloat("rval", redVal);
        sovietShader.setFloat("gval", greenVal);
        sovietShader.setFloat("bval", blueVal);

        // Fix w  i  d  e
        glm::mat4 trans = glm::mat4(1.0f);
        if (orthogonal_mode)
        {
            trans = glm::scale(trans, glm::vec3(scaleFactor / ratio, scaleFactor, scaleFactor));
        }
        else
        {
            trans = glm::scale(trans, glm::vec3(scaleFactor));
        }
        trans = glm::rotate(trans, glm::radians(x_angle), glm::vec3(1.0f, 0.0f, 0.0f));
        trans = glm::rotate(trans, glm::radians(y_angle), glm::vec3(0.0f, 1.0f, 0.0f));
        trans = glm::rotate(trans, glm::radians(z_angle), glm::vec3(0.0f, 0.0f, 1.0f));

        // Apply transformations
        if (!orthogonal_mode)
        {
            trans = projection * view * model * trans;
        }

        glUniformMatrix4fv(glGetUniformLocation(sovietShader.ID, "trans"), 1, GL_FALSE, glm::value_ptr(trans));

        // TRIANGLE(S) !!!!!111!1o 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, sizeof indices / sizeof (u_int), GL_UNSIGNED_INT, 0);

        sovietShader.setFloat("rval", 0);
        sovietShader.setFloat("gval", 0);
        sovietShader.setFloat("bval", 0);
        glLineWidth(7.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if (show_triangles)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glDrawElements(GL_TRIANGLES, sizeof line_indices / sizeof (u_int), GL_UNSIGNED_INT, 0);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
            glDrawElements(GL_LINES, sizeof line_indices / sizeof (u_int), GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate program
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    glViewport(0, 0, w, h);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        orthogonal_mode = !orthogonal_mode;
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        show_triangles= !show_triangles;
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto current = scaleFactor;
    scaleFactor += yoffset * 0.02f;
    if (scaleFactor < 0)
    {
        scaleFactor = current;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // get change in mouse position
    static double lastX = 0;
    static double lastY = 0;
}

void processInput(GLFWwindow *window)
{
    // Rotation
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        z_angle += 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        z_angle -= 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        y_angle += 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        y_angle -= 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        x_angle += 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        x_angle -= 3.0f;
    }
    // Translation (camera)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        cam_x -= 0.1f * sin(glm::radians(cam_angle_x));
        cam_z += 0.1f * cos(glm::radians(cam_angle_x));
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        cam_x += 0.1f * sin(glm::radians(cam_angle_x));
        cam_z -= 0.1f * cos(glm::radians(cam_angle_x));
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        cam_x -= 0.1f * cos(glm::radians(cam_angle_x));
        cam_z -= 0.1f * sin(glm::radians(cam_angle_x));
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        cam_x += 0.1f * cos(glm::radians(cam_angle_x));
        cam_z += 0.1f * sin(glm::radians(cam_angle_x));
    }
    // Look up/down
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        float current = cam_angle_y;
        cam_angle_y += 3.0f;
        if (cam_angle_y > 90.0f)
        {
            cam_angle_y = current;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        float current = cam_angle_y;
        cam_angle_y -= 3.0f;
        if (cam_angle_y < -90.0f)
        {
            cam_angle_y = current;
        }
    }
    // Look left/right
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        cam_angle_x += 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        cam_angle_x -= 5.0f;
    }
}