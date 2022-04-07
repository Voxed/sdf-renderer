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

float rot;
float rot2;

int main() {
    if (glfwInit() != GLFW_TRUE) {
        spdlog::error("Failed to initialize GLFW!");
        return EXIT_FAILURE;
    }

    auto window = glfwCreateWindow(480, 480, "Vx.Luminence", nullptr, nullptr);
    if (!window) {
        spdlog::error("Failed to create GLFW window!");
        glfwTerminate();
        return EXIT_FAILURE;
    }

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

    printf("Shader: %s\n", source.c_str());


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

    SDFInstance s[24];
    printf("SIZES: %lu, %lu\n", s, sizeof(SDFInstance));

    glUseProgram(program);

    GLuint ssbo;
    glGenBuffers(1, &ssbo);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 24 * sizeof(SDFInstance), &s,
                 GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    float data[20 * 10 * 10 * 3];

    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                data[x + y * 20 + z * 20 * 10] = sqrt(pow(5 - z, 2) + pow(5 - x, 2) + pow(5 - y, 2)) - 3.0f;
                data[x + y * 20 + z * 20 * 10] = fmin(sqrt(pow(5 - z, 2) + pow(14 - x, 2) + pow(5 - y, 2)) - 3.0f,
                                                      data[x + y * 20 + z * 20 * 10]);
                data[x + y * 20 + z * 20 * 10] = fmin(sqrt(pow(5 - z, 2) + pow(5 - y, 2)) - 2.0f,
                                                      data[x + y * 20 + z * 20 * 10]);
            }
        }
    }
    int off = 20 * 10 * 10;
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                data[x + y * 20 + z * 20 * 10 + off] = sqrt(pow(5 - z, 2) + pow(5 - x, 2) + pow(5 - y, 2)) - 2.0f;
                data[x + y * 20 + z * 20 * 10 + off] = fmin(sqrt(pow(5 - z, 2) + pow(14 - x, 2) + pow(5 - y, 2)) - 3.0f,
                                                      data[x + y * 20 + z * 20 * 10 + off]);
                data[x + y * 20 + z * 20 * 10 + off] = fmin(sqrt(pow(5 - z, 2) + pow(5 - y, 2)) - 1.0f,
                                                      data[x + y * 20 + z * 20 * 10 + off]);
            }
        }
    }
    off = 20 * 10 * 10*2;
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                data[x + y * 20 + z * 20 * 10 + off] = sqrt(pow(5 - z, 2) + pow(5 - x, 2) + pow(5 - y, 2)) - 3.0f;
                data[x + y * 20 + z * 20 * 10 + off] = fmin(sqrt(pow(5 - z, 2) + pow(14 - x, 2) + pow(5 - y, 2)) - 1.0f,
                                                            data[x + y * 20 + z * 20 * 10 + off]);
                data[x + y * 20 + z * 20 * 10 + off] = fmin(sqrt(pow(5 - z, 2) + pow(5 - y, 2)) - 0.25f,
                                                            data[x + y * 20 + z * 20 * 10 + off]);
            }
        }
    }
    GLuint ssbo2;
    glGenBuffers(1, &ssbo2);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), data,
                 GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    int tex_w = 256, tex_h = 256;
    GLuint tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
                 nullptr);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glClearColor(255, 0, 0, 255);

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
    while (!glfwWindowShouldClose(window)) {
        for (int i = 0; i < 24; i++) {
            s[i] = SDFInstance{
                    glm::inverse(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0),
                                                                        glm::vec3(-16.0f + (i % 8) * 6.75f - 8.0f,
                                                                                  floor(i / 8) * 20 - 20, 0.0)),
                                                         rot, glm::vec3(0, 1, 0)), rot2,
                                             glm::vec3(0, 0, 1)

                                 )

                    ),
                    glm::vec4(20, 10, 10, 0),
                    glm::vec4(20 * 10 * 10 * floor(i/8), 20 * 10 * 10, 0, 0),
            };
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, 24 * sizeof(SDFInstance), &s,
                     GL_DYNAMIC_DRAW);
        glUseProgram(program);

        for (int i = 0; i < 1; i++) {
            glDispatchCompute((GLuint) tex_w / 16, (GLuint) tex_h / 16, 1);
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
        ImGui::SliderFloat("Rotate Y", &rot, 0, 3.14*2);
        ImGui::SliderFloat("Rotate Z", &rot2, 0, 3.14*2);
        ImGui::End();

        ImGui::Render();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program2);

        glActiveTexture(0);
        glBindTexture(GL_TEXTURE_2D, tex_output);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
