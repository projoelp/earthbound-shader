#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

char* read_file(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) { fprintf(stderr, "ERROR: Could not open file: %s\n", filepath); return NULL; }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, file);
    fclose(file);
    buffer[length] = '\0';
    return buffer;
}

GLuint create_shader_program(const char* frag_path) {
    const char* vertex_shader_source = read_file("shader.vert");
    const char* fragment_shader_source = read_file(frag_path);
    if (!vertex_shader_source || !fragment_shader_source) {
        free((void*)vertex_shader_source);
        free((void*)fragment_shader_source);
        return 0;
    }
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    int success; char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog); fprintf(stderr, "ERROR: Vertex shader compilation failed\n%s\n", infoLog); }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog); fprintf(stderr, "ERROR: Fragment shader compilation failed\n%s\n", infoLog); }
    
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) { glGetProgramInfoLog(shader_program, 512, NULL, infoLog); fprintf(stderr, "ERROR: Shader program linking failed\n%s\n", infoLog); }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    free((void*)vertex_shader_source);
    free((void*)fragment_shader_source);

    if (!success) { glDeleteProgram(shader_program); return 0; }

    printf("Successfully compiled and linked shader: %s\n", frag_path);
    return shader_program;
}

int main(int argc, char* args[]) {
    if (argc < 2) { fprintf(stderr, "Usage: %s <fragment_shader.frag>\n", args[0]); return 1; }
    const char* frag_shader_path = args[1];
    time_t last_mod_time = 0;
    struct stat file_stat;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    SDL_Window* window = SDL_CreateWindow("GLSL Live Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    float vertices[] = { 1.0f,1.0f,0.0f, 1.0f,-1.0f,0.0f, -1.0f,-1.0f,0.0f, -1.0f,1.0f,0.0f };
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint shader_program = create_shader_program(frag_shader_path);
    if (shader_program == 0) { fprintf(stderr, "Failed to load initial shader. Exiting.\n"); return -1; }
    stat(frag_shader_path, &file_stat);
    last_mod_time = file_stat.st_mtime;

    int quit = 0;
    SDL_Event e;
    Uint32 start_time = SDL_GetTicks();

    while (!quit) {
        if (stat(frag_shader_path, &file_stat) == 0 && file_stat.st_mtime > last_mod_time) {
            last_mod_time = file_stat.st_mtime;
            printf("Shader file changed, reloading...\n");
            glDeleteProgram(shader_program);
            shader_program = create_shader_program(frag_shader_path);
        }
        
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) { quit = 1; }
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);

        if (shader_program > 0) {
            glUseProgram(shader_program);
            float time_sec = (SDL_GetTicks() - start_time) / 1000.0f;
            glUniform1f(glGetUniformLocation(shader_program, "u_time"), time_sec);
            glUniform2f(glGetUniformLocation(shader_program, "u_resolution"), (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        
        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }

    glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader_program);
    SDL_GL_DeleteContext(gl_context); SDL_DestroyWindow(window); SDL_Quit();
    return 0;
}