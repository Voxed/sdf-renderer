#include <iostream>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "shader.comp"
#include "shader.vert"
#include "shader.frag"
#include "sdfgen.comp"
#include "raster.vert"
#include "raster.frag"
#include "shadows.frag"
#include "position.frag"

#include "third_party/OBJ_Loader.h"
#include "Primitive.h"
#include "Mesh.h"


float rot;
float rot2;
float cut;
bool SHADOW = false;
bool normal;
bool light;
bool TRACE;
bool product;
bool USE_SHADOWS;

GLuint fboTex;
GLuint traceTex;
GLuint depthBuffer;
GLuint posTex;
GLuint normTex;

int width = 512;
int height = 512;

GLuint createComputeShader(std::string source) {
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char *sources[] = {
            source.c_str()
    };
    glShaderSource(computeShader, 1, sources, nullptr);
    glCompileShader(computeShader);
    GLint status;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED comp");

        char str[1024];
        glGetShaderInfoLog(computeShader, 1024, nullptr, &str[0]);

        printf("%s\n", str);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED");
    }

    return program;
}

GLuint createRasterShader(std::string vert, std::string frag) {
    GLuint fragSh = glCreateShader(GL_FRAGMENT_SHADER);
    const char *sources[] = {
            frag.c_str()
    };
    glShaderSource(fragSh, 1, sources, nullptr);
    glCompileShader(fragSh);
    GLint status;
    glGetShaderiv(fragSh, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED frag");

        char str[1024];
        glGetShaderInfoLog(fragSh, 1024, nullptr, &str[0]);

        printf("%s\n", str);
    }

    GLuint vertSh = glCreateShader(GL_VERTEX_SHADER);
    sources[0] =
            vert.c_str();
    glShaderSource(vertSh, 1, sources, nullptr);
    glCompileShader(vertSh);
    glGetShaderiv(vertSh, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED vert");

        char str[1024];
        glGetShaderInfoLog(vertSh, 1024, nullptr, &str[0]);

        printf("%s\n", str);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertSh);
    glAttachShader(program, fragSh);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED");
    }

    return program;
}

#define resol 64
#define SSBO_MAX 268435456

int main() {


    /*
    for(const auto& v : MESH->NormalizedVertices()) {
        printf("%f, %f, %f,\n", v.x, v.y, v.z);
    }
    */

    if (glfwInit() != GLFW_TRUE) {
        spdlog::error("Failed to initialize GLFW!");
        return EXIT_FAILURE;
    }



    auto window = glfwCreateWindow(width, height, "Vx.Luminence", nullptr, nullptr);
    if (!window) {
        spdlog::error("Failed to create GLFW window!");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int w, int h){
        glViewport(0,0,w,h);
        width = w;
        height = h;

        glBindTexture(GL_TEXTURE_2D, fboTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, traceTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, depthBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

        glBindTexture(GL_TEXTURE_2D, posTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, normTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    });

    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (glewInit() != GLEW_OK) {
        spdlog::error("Failed to initialize GLEW!");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }



    objl::Loader loader;

    bool loadout = loader.LoadFile("suzanne.obj");
    std::shared_ptr<Mesh> MESH;
    if (loadout) {
        objl::Mesh mesh = loader.LoadedMeshes[0];

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        for (auto &vertex: mesh.Vertices) {
            vertices.emplace_back(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);
            normals.emplace_back(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);
        }

        indices = mesh.Indices;

        Primitive primitive(vertices, indices, normals);
        std::vector<Primitive> primitives;
        primitives.push_back(primitive);

        MESH = std::make_shared<Mesh>(primitives);
    }

    objl::Loader loader2;
    loadout = loader2.LoadFile("mesh2.obj");
    std::shared_ptr<Mesh> MESH2;
    if (loadout) {

        objl::Mesh mesh = loader2.LoadedMeshes[0];

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        for (auto &vertex: mesh.Vertices) {
            vertices.emplace_back(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);
            normals.emplace_back(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);
        }

        indices = mesh.Indices;

        Primitive primitive(vertices, indices, normals);
        std::vector<Primitive> primitives;
        primitives.push_back(primitive);

        MESH2 = std::make_shared<Mesh>(primitives);
    }




    //printf("Shader: %s\n", source.c_str());


    GLuint program = createComputeShader(source);


    GLuint rastpr = createRasterShader(rastvertsource, rastfragsource);
    GLuint shadowProgram = createRasterShader(vertsource, shadfragsource);

    GLuint sdfGen = createComputeShader(sdfsource);

    glUseProgram(sdfGen);

    GLuint sdfIn;
    glGenBuffers(1, &sdfIn);

    GLuint sdfOut;
    glGenBuffers(1, &sdfOut);


    std::vector<glm::vec4> faces;
    for (const auto &v: MESH->NormalizedVertices()) {
        faces.push_back(glm::vec4(v, 0.0));
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdfIn);
    glNamedBufferStorage(sdfIn, // Name of the buffer
            //width*height*sizeof(glm::vec3), // Storage size in bytes
                         faces.size() * sizeof(glm::vec4), // Storage size in bytes
                         faces.data(),//&image[0], //  initial data
                         GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT); // Allow map for writing

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sdfIn);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdfOut);
    glNamedBufferStorage(sdfOut, // Name of the buffer
            //width*height*sizeof(glm::vec3), // Storage size in bytes
                         resol * resol * resol * sizeof(float), // Storage size in bytes
                         NULL,//&image[0], //  initial data
                         GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT); // Allow map for writing

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sdfOut);

    glDispatchCompute(resol / 8, resol / 8, resol / 8);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdfOut);
    float *sdfOutData = (float *) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    for (int x = 0; x < resol; x++)
        for (int y = 0; y < resol; y++)
            for (int z = 0; z < resol; z++) {
                float d = sdfOutData[x + y * resol + z * resol * resol];
                if (d < 0.0) {
                    printf("%f, %f, %f,\n", ((float) x) / resol * MESH->Size().x, ((float) y) / resol * MESH->Size().y,
                           ((float) z) / resol * MESH->Size().z);
                }
            }

    float* dataSuzanne = (float*)malloc(SSBO_MAX*sizeof(float));
    memcpy(dataSuzanne, sdfOutData, resol*resol*resol*sizeof(float));

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);





    faces = std::vector<glm::vec4>();
    for (const auto &v: MESH2->NormalizedVertices()) {
        faces.push_back(glm::vec4(v, 0.0));
    }

    glDeleteBuffers(1, &sdfIn);
    glGenBuffers(1, &sdfIn);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdfIn);
    glNamedBufferStorage(sdfIn, // Name of the buffer
            //width*height*sizeof(glm::vec3), // Storage size in bytes
                         faces.size() * sizeof(glm::vec4), // Storage size in bytes
                         faces.data(),//&image[0], //  initial data
                         GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT); // Allow map for writing

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sdfIn);


    glDispatchCompute(resol / 8, resol / 8, resol / 8);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdfOut);
    sdfOutData = (float *) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

    memcpy(dataSuzanne + resol*resol*resol, sdfOutData, resol*resol*resol*sizeof(float));

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


















    GLint status;

    const char *fragsrcs[] = {
            fragsource.c_str()
    };
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, fragsrcs, nullptr);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED frag");

        char str[1024];
        glGetShaderInfoLog(fragShader, 1024, nullptr, &str[0]);

        printf("%s\n", str);
    }

    const char *vertsrcs[] = {
            vertsource.c_str()
    };
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, vertsrcs, nullptr);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED vert");

        char str[1024];
        glGetShaderInfoLog(vertShader, 1024, nullptr, &str[0]);

        printf("%s\n", str);
    }

    GLuint program2 = glCreateProgram();
    glAttachShader(program2, fragShader);
    glAttachShader(program2, vertShader);
    glLinkProgram(program2);
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        spdlog::info("FAILED prog");
    }


    struct SDFInstance {
        glm::mat4 transform;
        glm::vec4 size;
        glm::vec4 dataLength;
    };

    SDFInstance s[2];
    printf("SIZES: %lu, %lu\n", s, sizeof(SDFInstance));

    glUseProgram(program);

    GLuint ssbo;
    glGenBuffers(1, &ssbo);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(s), &s,
                 GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    float data[20 * 10 * 10 * 3];

    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                data[x + y * 20 + z * 20 * 10] = sqrt(pow(4.5 - z, 2) + pow(4.5 - x, 2) + pow(4.5 - y, 2)) - 3.0f;
                data[x + y * 20 + z * 20 * 10] = fmin(sqrt(pow(4.5 - z, 2) + pow(14 - x, 2) + pow(4.5 - y, 2)) - 3.0f,
                                                      data[x + y * 20 + z * 20 * 10]);
                data[x + y * 20 + z * 20 * 10] = fmin(sqrt(pow(4.5 - z, 2) + pow(4.5 - y, 2)) - 2.0f,
                                                      data[x + y * 20 + z * 20 * 10]);
            }
        }
    }
    int off = 20 * 10 * 10;
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                data[x + y * 20 + z * 20 * 10 + off] = sqrt(pow(4.5 - z, 2) + pow(4.5 - x, 2) + pow(4.5 - y, 2)) - 2.0f;
                data[x + y * 20 + z * 20 * 10 + off] = fmin(
                        sqrt(pow(4.5 - z, 2) + pow(14 - x, 2) + pow(4.5 - y, 2)) - 3.0f,
                        data[x + y * 20 + z * 20 * 10 + off]);
                data[x + y * 20 + z * 20 * 10 + off] = fmin(sqrt(pow(4.5 - z, 2) + pow(4.5 - y, 2)) - 1.0f,
                                                            data[x + y * 20 + z * 20 * 10 + off]);
            }
        }
    }
    off = 20 * 10 * 10 * 2;
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                data[x + y * 20 + z * 20 * 10 + off] = sqrt(pow(4.5 - z, 2) + pow(4.5 - x, 2) + pow(4.5 - y, 2)) - 3.0f;
                data[x + y * 20 + z * 20 * 10 + off] = fmin(
                        sqrt(pow(4.5 - z, 2) + pow(14.5 - x, 2) + pow(4.5 - y, 2)) - 2.0f,
                        data[x + y * 20 + z * 20 * 10 + off]);
                data[x + y * 20 + z * 20 * 10 + off] = fmin(sqrt(pow(4.5 - z, 2) + pow(4.5 - y, 2)) - 0.75f,
                                                            data[x + y * 20 + z * 20 * 10 + off]);
            }
        }
    }
    GLuint ssbo2;
    glGenBuffers(1, &ssbo2);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_MAX*sizeof(float), dataSuzanne, //sizeof(data), data,
                 GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    int tex_w = 2048, tex_h = 2048;
    GLuint tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
                 nullptr);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glClearColor(0, 0, 0, 255);

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    float datadd[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,

            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(datadd), &datadd[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    double previousTime = glfwGetTime();
    int frameCount = 0;
    int fps = 0;









    unsigned int rastfbo;
    glGenFramebuffers(1, &rastfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, rastfbo);

    glGenTextures(1, &fboTex);
    glBindTexture(GL_TEXTURE_2D, fboTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int tracefbo;
    glGenFramebuffers(1, &tracefbo);
    glBindFramebuffer(GL_FRAMEBUFFER, tracefbo);

    glGenTextures(1, &traceTex);
    glBindTexture(GL_TEXTURE_2D, traceTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, traceTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    unsigned int posfbo;
    glGenFramebuffers(1, &posfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, posfbo);

    glGenTextures(1, &posTex);
    glBindTexture(GL_TEXTURE_2D, posTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);


    glGenTextures(1, &normTex);
    glBindTexture(GL_TEXTURE_2D, normTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normTex, 0);


    GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    GLuint posProgram = createRasterShader(rastvertsource, fragpos);



    while (!glfwWindowShouldClose(window)) {
        glm::mat4 PROJ = glm::perspective(45.0f, (float)width/(float)height, 0.1f, 200.0f);
        glm::mat4 V = glm::lookAt(glm::vec3(cos(rot)*10.0f,rot2,sin(rot)*10.0f), glm::vec3(0.0), glm::vec3(0,1.0,0));
        glm::mat4 MM = glm::scale(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(1.0,1.0,0.0)), glm::vec3(1.0,1.0 + 1.0 + cos(glfwGetTime()),1.0));

        glm::mat4 MM2 = glm::translate(glm::mat4(1.0), glm::vec3(0.0,-2.0,0.0));

        glUseProgram(rastpr);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(PROJ));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(MM));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(V));
        glUseProgram(0);


        glUseProgram(posProgram);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(PROJ));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(MM));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(V));
        glUseProgram(0);

        glUseProgram(program);
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(PROJ));
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(V));
        glUseProgram(0);

        for (int i2 = 0; i2 < 1; i2++) {
            int i = i2 % 24;
            int z = floor(i2 / 24) * 20;
            s[i2] = SDFInstance{
                    glm::inverse(glm::scale(glm::translate(MM, glm::vec3(MESH->Bounds()[0])), MESH->Size())
                    ),
                    glm::vec4(resol, resol, resol, 0),
                    glm::vec4(0, resol*resol*resol, 0, 0),
            };
        }

        s[1] = SDFInstance{
                glm::inverse(glm::scale(glm::translate(MM2, glm::vec3(MESH2->Bounds()[0])), MESH2->Size())
                ),
                glm::vec4(resol, resol, resol, 0),
                glm::vec4(resol*resol*resol, resol*resol*resol*2, 0, 0),
        };

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(s), &s,
                     GL_DYNAMIC_DRAW);
        glUseProgram(program);

        glUniform1f(0, normal ? 1.0f : 0.0f);
        glUniform1f(1, light ? 1.0f : 0.0f);
        glUniform1f(5, SHADOW ? 1.0f : 0.0f);
        glUniform1f(6, USE_SHADOWS ? 1.0f : 0.0f);
        glUniform1f(2, cut);

        for (int i = 0; i < 1; i++) {
            glDispatchCompute((GLuint) tex_w / 32, (GLuint) tex_h / 32, 1);
        }

        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0) {
            fps = frameCount;
            frameCount = 0;
            previousTime = currentTime;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::Begin("Test", nullptr,
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoInputs);
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %i", fps);
        ImGui::End();

        ImGui::Begin("SDF");
        ImGui::Text("SDF rotation control :)");
        ImGui::SliderFloat("Rotate Y", &rot, 0, 3.14 * 2);
        ImGui::SliderFloat("Rotate Z", &rot2, 0, 20.0);
        ImGui::SliderFloat("Cut", &cut, 0, 150);
        ImGui::Checkbox("Normals", &normal);
        ImGui::Checkbox("Lights", &light);
        ImGui::Checkbox("Use shadows", &USE_SHADOWS);
        ImGui::Checkbox("Shadows only", &SHADOW);
        ImGui::Checkbox("Product", &product);
        ImGui::Checkbox("Raytrace", &TRACE);
        ImGui::End();

        ImGui::Render();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, tracefbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program2);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_output);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);





        glBindFramebuffer(GL_FRAMEBUFFER, rastfbo);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(rastpr);

        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(MM));
        MESH->Render();

        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(MM2));
        MESH2->Render();


        glUseProgram(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);




        glBindFramebuffer(GL_FRAMEBUFFER, posfbo);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(posProgram);

        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(MM));
        MESH->Render();

        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(MM2));
        MESH2->Render();

        glUseProgram(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);




        if(product && !TRACE) {
            glUseProgram(shadowProgram);
        } else{
            glUseProgram(program2);
        }

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, TRACE ? traceTex : fboTex);
        //glBindTexture(GL_TEXTURE_2D, normTex);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, traceTex);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(0);





        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
