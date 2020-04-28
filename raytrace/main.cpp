#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <linmath.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>

#include <entt/entt.hpp>

#include "opencl_raytracer.hpp"
#include "metal_raytracer.hpp"
#include "entities.hpp"


using namespace std;

static const struct
{
    float x, y;
    float r, g, b;
    float tx, ty;
} vertices[] =
{
    {-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
    {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
    {0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
};


static const char* vertex_shader_text =
        "#version 110\n"
        "attribute vec3 vCol;\n"
        "attribute vec2 vPos;\n"
        "varying vec3 color;\n"
        "attribute vec2 vTextCord;\n"
        "varying vec2 textCord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vPos, 0.0, 0.5);\n"
        "    color = vCol;\n"
        "    textCord = vTextCord;\n"
        "}\n";

static const char* fragment_shader_text =
        "#version 110\n"
        "varying vec3 color;\n"
        "varying vec2 textCord;\n"
        "uniform sampler2D renderTexture;\n"
        "void main()\n"
        "{\n"
//        "    gl_FragColor = vec4(color, 1.0);\n"
//        "    gl_FragColor = texture(renderTexture, textCord).rgb;\n"
        "      vec4 tex = texture2D ( renderTexture, textCord );\n"
        "      gl_FragColor = vec4(tex.r, tex.g, tex.b, 1);\n"
        "}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

inline float GetRandom() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}



const int WIDTH = 1280;
const int HEIGHT = 1024;
const float SCALE = 0.01;


class Physics {
public:
    Physics(entt::registry& registry)
        : Registry(registry)
    {}
    void Update() {
        auto view = Registry.view<Transform, RigidBody>();
        for(auto entity: view) {
            RigidBody& rigidBody = view.get<RigidBody>(entity);
            Transform& transform = view.get<Transform>(entity);
            transform.Position += rigidBody.Velocity;

            if (transform.Position.X < -4.0 || transform.Position.X > 4.0) {
                rigidBody.Velocity.X = -rigidBody.Velocity.X;
            }
            if (transform.Position.Y < -5.0 || transform.Position.Y > 3.0) {
                rigidBody.Velocity.Y = -rigidBody.Velocity.Y;
            }
            if (transform.Position.Z < -2.1 || transform.Position.Z > 2.1) {
                rigidBody.Velocity.Z = -rigidBody.Velocity.Z;
            }
        }
    }
private:
    entt::registry& Registry;
};


int main(void)
{
    //RunLinmathTests();

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location,vTextCord_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    //gladLoadGL(glfwGetProcAddress);
    gladLoadGL();
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
    vTextCord_location = glGetAttribLocation(program, "vTextCord");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);

    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));

    glEnableVertexAttribArray(vTextCord_location);
    glVertexAttribPointer(vTextCord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 5));


    entt::registry registry;

//    OCLRaytracer oclRaytracer(registry, WIDTH, HEIGHT);
    MetalRaytracer metalRaytracer(registry, WIDTH, HEIGHT);
    Physics physics(registry);


    {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(0.0f, -2.0f, -20.0f);
        Camera& camera = registry.assign<Camera>(entity);
        camera.Direction = Vector3(0, 0, 20.0f);
        camera.FocusDistance = 20.0f;
    }

    {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(23.0f, 30.0f, -80.0f);
        LightSource& light = registry.assign<LightSource>(entity);
        light.Power = 0.9f;
    }


    /*

    {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(-1, 0, 3);

        SphereRenderer& sphere = registry.assign<SphereRenderer>(entity);
        sphere.Radius = 2;

        Material& material = registry.assign<Material>(entity);
        material.Color = Color(0.0, 0.0, 1.0);
        material.DiffuseCF = 0.95f;
        material.AlbedoCF = Vector3(20.0f, 1.4f, 0.1f);
        material.RefractCF =  Vector2(0.0f, 0.0f);
        material.ShadowQuality = 2;

        RigidBody& rigidBody = registry.assign<RigidBody>(entity);
        rigidBody.Velocity = Vector3(0.04f, -0.02f, 0.01);
    }

    {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(2, 2, 0);

        SphereRenderer& sphere = registry.assign<SphereRenderer>(entity);
        sphere.Radius = 1.5;

        Material& material = registry.assign<Material>(entity);
        material.Color = Color(1.0, 1.0, 0.0);
        material.DiffuseCF = 0.95f;
        material.AlbedoCF = Vector3(20.0f, 1.4f, 0.1f);
        material.RefractCF =  Vector2(0.0f, 0.0f);
        material.ShadowQuality = 2;

        RigidBody& rigidBody = registry.assign<RigidBody>(entity);
        rigidBody.Velocity = Vector3(-0.03f, 0.03f, -0.02);
    }

    {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(2, 2, 0);

        SphereRenderer& sphere = registry.assign<SphereRenderer>(entity);
        sphere.Radius = 1.2;

        Material& material = registry.assign<Material>(entity);
        material.Color = Color(1.0, 0.3, 0.4);
        material.DiffuseCF = 0.7f;
        material.AlbedoCF = Vector3(1.0f, 0.1f, 0.0f);
        material.RefractCF =  Vector2(0.0f, 0.0f);
        material.ShadowQuality = 2;

        RigidBody& rigidBody = registry.assign<RigidBody>(entity);
        rigidBody.Velocity = Vector3(-0.02f, 0.02f, 0.04);
    }

    {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(-1, -3, 3);

        SphereRenderer& sphere = registry.assign<SphereRenderer>(entity);
        sphere.Radius = 1.7;

        Material& material = registry.assign<Material>(entity);
        material.Color = Color(0.5, 1.0, 0.5);
        material.DiffuseCF = 0.9f;
        material.AlbedoCF = Vector3(20.0f, 1.1f, 0.1f);
        material.RefractCF =  Vector2(0.0f, 0.0f);
        material.ShadowQuality = 2;

        RigidBody& rigidBody = registry.assign<RigidBody>(entity);
        rigidBody.Velocity = Vector3(0.05f, -0.03f, -0.01);
    }

    {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(-2, -2, 0);

        SphereRenderer& sphere = registry.assign<SphereRenderer>(entity);
        sphere.Radius = 1.2;

        Material& material = registry.assign<Material>(entity);
        material.Color = Color(0.5, 0.5, 1.0);
        material.DiffuseCF = 0.1f;
        material.AlbedoCF = Vector3(20.0f, 1.4f, 0.4f);
        material.RefractCF =  Vector2(0.4f, 0.0f);
        material.ShadowQuality = 2;

        RigidBody& rigidBody = registry.assign<RigidBody>(entity);
        rigidBody.Velocity = Vector3(-0.07f, 0.08f, 0.04);
    }
    */

    for (int i = 0; i < 50; ++i) {
        auto entity = registry.create();
        Transform& transform = registry.assign<Transform>(entity);
        transform.Position = Vector3(GetRandom() * 4 - 2, GetRandom() * 4 - 2, GetRandom() * 4 - 2);

        SphereRenderer& sphere = registry.assign<SphereRenderer>(entity);
        sphere.Radius = 0.3f + GetRandom() * 0.6f;

        Material& material = registry.assign<Material>(entity);
        material.Color = Color(GetRandom(), GetRandom(), GetRandom());
        material.DiffuseCF = 0.1f + GetRandom() * 0.8f;
        material.AlbedoCF = Vector3(20.0f, 1.4f, GetRandom() * 0.4f);
        material.RefractCF =  Vector2(GetRandom() * 0.2f, 0.0f);
        material.ShadowQuality = 2;

        RigidBody& rigidBody = registry.assign<RigidBody>(entity);
        rigidBody.Velocity = Vector3(GetRandom() * 0.16 - 0.08, GetRandom() * 0.16 - 0.08, GetRandom() * 0.16 - 0.08);
    }


    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, oclRaytracer.RawData());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, metalRaytracer.RawData());

    clock_t prevTime = clock();
    int frames = 0;

    while (!glfwWindowShouldClose(window))
    {
        physics.Update();
//        oclRaytracer.Update();
        metalRaytracer.Update();

        float ratio;
        int width, height;

        glBindTexture(GL_TEXTURE_2D, tex);
//        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, oclRaytracer.RawData());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, metalRaytracer.RawData());

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
        frames += 1;

        clock_t currTime = clock();
        double elapsed = (double) (currTime - prevTime) / CLOCKS_PER_SEC;
        if (elapsed >= 1.0f) {
            cout << "FPS: " << frames << "\n";
            frames = 0;
            prevTime = currTime;
        }
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
