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

#include "gpu_raytracer.hpp"


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




struct Ray {
    Ray(Vector3 from, Vector3 dir): From(from), Dir(dir) {}
    Vector3 From;
    Vector3 Dir;
};


const int WIDTH = 1280;
const int HEIGHT = 1024;
const float SCALE = 0.01;

//const int WIDTH = 640;
//const int HEIGHT = 480;
//const float SCALE = 0.02;

//const int WIDTH = 800;
//const int HEIGHT = 600;
//const float SCALE = 0.015;

//class PixelRenderer {
//public:
//    PixelRenderer(int width, int height) {
//        Width = width;
//        Height = height;
//        Data.resize((width+1) * (height+1) * 3);
//    }
//    void SetPixel(int x, int y, Color c) {
//        int pos = (y * Width + x) * 3;
//        Data[pos] = c.R;
//        Data[pos + 1] = c.G;
//        Data[pos + 2] = c.B;
//    }
//    void* RawData() {
//        return &Data[0];
//    }
//private:
//    int Width;
//    int Height;
//    vector<float> Data;
//};

struct LightSource {
    LightSource(float x, float y, float z, float power) {
        Position.X = x;
        Position.Y = y;
        Position.Z = z;
        Power = power;
    }
    Vector3 Position;
    float Power;
};

//struct Material {
//    Material(Color color, float diffuseCF, Vector3 albedoCF, Vector2 refractCF, int shadowQaulity)
//        : Color(color), DiffuseCF(diffuseCF), AlbedoCF(albedoCF), RefractCF(refractCF), ShadowQuality(shadowQaulity)
//    {}
//    Material(): DiffuseCF(0.0) {}
//    Color Color;
//    float DiffuseCF;
//    Vector3 AlbedoCF;
//    Vector2 RefractCF;
//    int ShadowQuality;
//};

struct Sphere {
    Sphere(Vector3 position, float radius, Material material, Vector3 velocity)
        : Position(position), Radius(radius), Material(material), Velocity(velocity)
    {
        RadiusSqr = radius * radius;
    }
    float Intersect(const Ray& ray, Material& material, Vector3& normal) const {
        Vector3 k = ray.From - Position;

        float b = k.Dot(ray.Dir);
        float kLen = k.Magnitude();
        float c = kLen*kLen - RadiusSqr;


        float d = b*b - c;
        if (d < 0) {
            return numeric_limits<float>::max();
        }

        float sqrtfd = sqrt(d);
        float t1 = -b + sqrtfd;
        float t2 = -b - sqrtfd;

        float min_t  = min(t1,t2);
        float max_t = max(t1,t2);

        float t = (min_t >= 0) ? min_t : max_t;
        if (t <= 0) {
            return numeric_limits<float>::max();
        }
        material = Material;
        normal = (ray.From + ray.Dir * t - Position).Normalized();
        return t;
    }
public:
    Vector3 Position;
    Material Material;
    float Radius;
    float RadiusSqr;
    Vector3 Velocity;
};

struct Board {
    Board() {}
    Board(float y, Material matA, Material matB) : MaterialA(matA), MaterialB(matB) {}
    float Intersect(const Ray& ray, Material& material, Vector3& normal) const {
        float dist = Position.Y - ray.From.Y;
        if ((dist < 0 && ray.Dir.Y >= 0) || (dist > 0 && ray.Dir.Y <= 0)) {
            return numeric_limits<float>::max();
        }

        float cf = dist / ray.Dir.Y;
        Vector3 rayToPoint = ray.Dir * cf;

        Vector3 point = ray.From + rayToPoint;
        if (point.X < Position.X -Size.X / 2 || point.X > Position.X + Size.X / 2 || point.Z < Position.Z -Size.Y / 2 || point.Z > Position.Z + Size.Y / 2) {
            return numeric_limits<float>::max();
        }

        float cx = fmod(point.X + 100000.0f, 8.0f);
        float cz = fmod(point.Z + 100000.0f, 8.0f);

        if ((cx < 4.0 && cz < 4.0) || (cx > 4.0 && cz > 4.0)) {
            material = MaterialA;
        } else {
            material = MaterialB;
        }

        if (ray.From.Y > Position.Y) {
            normal = Vector3(0, 1, 0);
        } else {
            normal = Vector3(0, -1, 0);
        }
        float resDist = rayToPoint.Magnitude();
//        cout << resDist << "\n";
        return resDist;
    }
public:
    Vector3 Position;
    Vector2 Size;
    Material MaterialA;
    Material MaterialB;
};




// todo: read https://habr.com/ru/post/436790/

class RaycastRenderer {
public:
    RaycastRenderer(PixelRenderer& renderer, GPURaytracer& gpuRaytracer)
        : Renderer(renderer)
        , GPURaytracer(gpuRaytracer)
    {
        Renderer = renderer;
//        Lights.push_back(LightSource(-6.5f, 13.0f, -15.5f, 0.1f));
//        Lights.push_back(LightSource(10.0f, 40.0f, -45.0f, 0.1f));

//        Lights.push_back(LightSource(0, 100.0f, 0, 0.3f));
        Lights.push_back(LightSource(23.0f, 30.0f, -80.0f, 0.9f));
//        Lights.push_back(LightSource(0.0f, 5.0f, -5.0f, 0.9f));


//        Spheres.push_back(Sphere(
//            Vector3(2, -1, -2), 1,
//            Material(Color(0.7, 0.7, 1.0), 0.4, Vector3(30.0f, 1.4f, 0.3f), Vector2(0.4f, 1.4f), 2),
//            Vector3(0.01f, 0.03f, 0.02f)
//        ));

        Spheres.push_back(Sphere(
            Vector3(-1, 0, 3), 2,
            Material(Color(0.0, 0.0, 1.0), 0.95f, Vector3(20.0f, 1.4f, 0.1f), Vector2(0.0f, 0.0f), 2),
            Vector3(0.04f, -0.02f, -0.01f)
        ));

//        Spheres.push_back(Sphere(
//            Vector3(1, 1.4, 0), 1,
//            Material(Color(1.0, 1.0, 0.0), 0.95f, Vector3(20.0f, 1.4f, 0.1f), Vector2(0.0f, 0.0f), 2),
//            Vector3(-0.02f, -0.04f, 0.03f)
//         ));
//
//        Spheres.push_back(Sphere(
//                Vector3(2, 2, 0), 1.8,
//                Material(Color(1.0, 0.3, 0.4), 0.95f, Vector3(20.0f, 0.3f, 0.0f), Vector2(0.0f, 0.0f), 2),
//                Vector3(-0.03f, 0.03f, 0.015f)
//        ));

        Board.Position = Vector3(0.0, -5.0f, 5.0);
        Board.Size = Vector2(24.0, 32.0);
        Board.MaterialA = Material(Color(1.0, 1.0, 0.8), 3.3f, Vector3(20.0f, 2.4f, 0.2f), Vector2(0.0f, 0.0f), 3);
        Board.MaterialB = Material(Color(0.4, 0.3, 0.2), 3.3f, Vector3(20.0f, 2.4f, 0.2f), Vector2(0.0f, 0.0f), 3);
    }

    void Update() {
        for (auto& s: Spheres) {
            s.Position += s.Velocity * 2.0f;
            if (s.Position.X < -4.0 || s.Position.X > 4.0) {
                s.Velocity.X = -s.Velocity.X;
            }
            if (s.Position.Y < -3.0 || s.Position.Y > 3.0) {
                s.Velocity.Y = -s.Velocity.Y;
            }
            if (s.Position.Z < -2.0 || s.Position.Z > 2.0) {
                s.Velocity.Z = -s.Velocity.Z;
            }
        }

        GPURaytracer.Update();

//        GPURaytracer.Process(
//                Lights[0].Position,
//                CameraPos,
//                Spheres[0].Position,
//                Spheres[0].Radius
//        );

//        clock_t prevTime = clock();
//        for (int j = 0; j < HEIGHT; ++j) {
//            for (int i = 0; i < WIDTH; ++i) {
//                Vector3 posFrom = CameraPos;
//                Vector3 dir = Vector3((i - WIDTH / 2)*SCALE, (j - HEIGHT / 2)*SCALE, 20.0f).Normalized();
//                Vector3 focusPoint = posFrom + dir * FocusDistance;
////                Color color;
////                int total = 0;
////                for (int sx = -5; sx <= 5; ++sx) {
////                    for (int sy = -5; sy <= 5; ++sy) {
////                        total += 1;
////                        Vector3 currPos = posFrom;
////                        currPos.X += (float)sx * 0.06f;
////                        currPos.Y += (float)sy * 0.06f;
////                        Vector3 currDir = (focusPoint - currPos).Normalized();
////                        Color currColor = TraceColored(Ray(currPos, currDir), 2);
////                        color.R += currColor.R;
////                        color.G += currColor.G;
////                        color.B += currColor.B;
////                    }
////                }
////
////                color.R /= (float)total;
////                color.G /= (float)total;
////                color.B /= (float)total;
//
////                Ray ray(posFrom, dir);
////                Color color = TraceColored(ray, 2);
////                Color color = {1.0, 1.0, 1.0};
//
////                if (i < -100 || j < -200 || i > 100 || j > 200) {
////                    color = {0, 0, 0};
////                }
//
////                Renderer.SetPixel(i, j, color);
//
//
//                float r, g, b;
//                GPURaytracer.GetColor(i, j, r, g, b);
//                Renderer.SetPixel(i, j, Color(r, g, b));
//            }
//
//            clock_t currTime = clock();
//            double elapsed = (double) (currTime - prevTime) / CLOCKS_PER_SEC;
//            if (elapsed > 5.0) {
//                cout << "processed " << 100.0 * (float)j / (float)HEIGHT << "%" << endl;
//                prevTime = currTime;
//            }
//        }
    }

    Color TraceColored(const Ray& ray, int depth) {
        Color res = BackgroundColor;
        float distance;
        Material material;
        Vector3 normal;
        Intersect(ray, distance, material, normal);
        if (distance == numeric_limits<float>::max()) {
            return res;
        }
        return GetColor(ray, distance, material, normal, depth);
    }

    void Intersect(const Ray& ray, float& distance, Material& material, Vector3& normal) {
        distance = numeric_limits<float>::max();
        for (size_t i = 0; i < Spheres.size(); ++i) {
            Material currMaterial;
            Vector3 currNormal;
            float dist = Spheres[i].Intersect(ray, currMaterial, currNormal);
            if (dist < distance) {
                distance = dist;
                material = currMaterial;
                normal = currNormal;
            }
        }
//        {
//            Material currMaterial;
//            Vector3 currNormal;
//            float dist = Board.Intersect(ray, currMaterial, currNormal);
//            if (dist < distance) {
//                distance = dist;
//                material = currMaterial;
//                normal = currNormal;
//            }
//        }
    }

    Color GetColor(const Ray& ray, float distance, const Material& material, const Vector3& normal, int depth) {

        Vector3 point = ray.From + ray.Dir * distance;
        Vector3 dirToCamera = ray.Dir * -1.0f;

        Color color(minLight, minLight, minLight);

        for (const auto& light: Lights) {
            Vector3 dirToLight = (light.Position - point).Normalized();

//            if (depth > 0) {
//                if (material.AlbedoCF.Z > 0) {
//                    Vector3 reflDir = dirToCamera.Reflect(normal);
//                    Color reflectedColor = TraceColored(Ray(point + reflDir * 0.01f, reflDir), depth-1);
//                    color.R += reflectedColor.R * material.AlbedoCF.Z;
//                    color.G += reflectedColor.G * material.AlbedoCF.Z;
//                    color.B += reflectedColor.B * material.AlbedoCF.Z;
//                }
//
//                if (material.RefractCF.X > 0) {
//                    Vector3 refrDir = ray.Dir.Refract(normal, material.RefractCF.Y).Normalized();
//                    Color refrColor = TraceColored(Ray(point + refrDir * 0.001f, refrDir), depth-1);
//                    color.R += refrColor.R * material.RefractCF.X;
//                    color.G += refrColor.G * material.RefractCF.X;
//                    color.B += refrColor.B * material.RefractCF.X;
//                }
//            }

//            float shadow = GetShadow(Ray(point + dirToLight * 0.5f, dirToLight), material.ShadowQuality);
//            if (shadow == 0) {
//                continue;
//            }
            float shadow = 1.0;

//            if (IntersectAnything(Ray(point + dirToLight * 0.001f, dirToLight))) {
//                continue;
//            }

            float dp = dirToLight.Dot(normal);
            if (dp > 0) {
                color.R += dp * light.Power * material.Color.R * material.DiffuseCF * shadow;
                color.G += dp * light.Power * material.Color.G * material.DiffuseCF * shadow;
                color.B += dp * light.Power * material.Color.B * material.DiffuseCF * shadow;
            }

            dp = dirToLight.Reflect(normal).Dot(dirToCamera);
            if (dp > 0) {
                dp = pow(dp, material.AlbedoCF.X);
                color.R += dp * light.Power * material.AlbedoCF.Y * shadow;
                color.G += dp * light.Power * material.AlbedoCF.Y * shadow;
                color.B += dp * light.Power * material.AlbedoCF.Y * shadow;
            }
        }
        return color;
    }

    float GetShadow(const Ray& ray, int shadowQuality) {
        if (shadowQuality == 0) {
            return (float)(!IntersectAnything(ray));
        }
        int num = 0;
        int total = 0;
        for (int i = -shadowQuality; i <= shadowQuality; ++i) {
            for (int j = -shadowQuality; j <= shadowQuality; ++j) {
                for (int k = -shadowQuality; k <= shadowQuality; ++k) {
                    Ray currRay = ray;
                    currRay.From.X += 0.1f * i;
                    currRay.From.Y += 0.1f * j;
                    if (IntersectAnything(currRay)) {
                        ++num;
                    }
                    ++total;
                }
            }
        }
//        for (int i = 0; i < total; ++i) {
//                Ray currRay = ray;
//                currRay.From.X += (GetRandom() - 0.5f) * 0.3f;
//                currRay.From.Y += (GetRandom() - 0.5f) * 0.3f;
//                currRay.From.Z += (GetRandom() - 0.5f) * 0.3f;
//                if (IntersectAnything(currRay)) {
//                    ++num;
//                }
//        }
        return 1.0f - (float(num) / total);
    }

    bool IntersectAnything(const Ray& ray) {
        float distance;
        Material material;
        Vector3 normal;
        Intersect(ray, distance, material, normal);
        return distance != numeric_limits<float>::max();
    }

private:
    Vector3 CameraPos = {0.0f, -2.0f, -20.0f};
    vector<LightSource> Lights;
    vector<Sphere> Spheres;
    Board Board;
    float minLight = 0.0f;
//    Color BackgroundColor = {0.8f, 0.8f, 1.0f};
    Color BackgroundColor = {1, 1, 1};
    PixelRenderer& Renderer;
    GPURaytracer& GPURaytracer;
    float FocusDistance = 20.0f;
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

    GPURaytracer gpuRaytracer(WIDTH, HEIGHT);

    PixelRenderer renderer(WIDTH, HEIGHT);
    RaycastRenderer raycastRenderer(renderer, gpuRaytracer);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, gpuRaytracer.RawData());

    clock_t prevTime = clock();
    int frames = 0;

    while (!glfwWindowShouldClose(window))
    {
        raycastRenderer.Update();
        float ratio;
        int width, height;

        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, gpuRaytracer.RawData());

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
