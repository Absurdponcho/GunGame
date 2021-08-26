#include "SpriteRendererComponent.h"
#include "../WindowManager.h"
#include "../ECS/GameEntity.h"
#include "../Logging/Logging.h"
#include "../Utility/Utility.h"
#include "../GameManager.h"
#include "../Rendering/Camera.h"
#include <glm/glm.hpp>
#include <gl/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include "../Assets/GameAssetSoftPointer.h"
#include "../Assets/GameAsset.h"
#include "../Assets/TextAsset.h"

bool SpriteRendererComponent::bGLInitialized = false;
unsigned int SpriteRendererComponent::VertexBufferObject = 0;
unsigned int SpriteRendererComponent::VertexArrayObject = 0;
unsigned int SpriteRendererComponent::ElementBufferObject = 0;
unsigned int SpriteRendererComponent::ShaderProgram = 0;

// Verts for a square of size 1x1
float Vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};

unsigned int Indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

void SpriteRendererComponent::OnSpawn()
{
    GameRendererComponent::OnSpawn();

    if (!bGLInitialized)
    {
        bGLInitialized = true;

        // Vertex Shader =========================================
            TextAsset* VertexShaderSource = GameAssetSoftPointer<TextAsset>("GameAssetFiles/Shaders/SpriteVertex.glsl").LoadSynchronous();
            Check(VertexShaderSource);
            const char* VertexShaderCString = VertexShaderSource->GetString().c_str();

            unsigned int VertexShader;
            VertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(VertexShader, 1, &VertexShaderCString, NULL);
            glCompileShader(VertexShader);

            int Success;
            char InfoLog[512];
            glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);

            if (!Success)
            {
                glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
                Logging::LogError("SpriteRendererComponent::OnSpawn()", "Vertex Shader: " + std::string(InfoLog));
                return;
            }
        //!Vertex Shader =========================================

        // Fragment Shader =======================================
            TextAsset* FragmentShaderSource = GameAssetSoftPointer<TextAsset>("GameAssetFiles/Shaders/SpriteFragment.glsl").LoadSynchronous();
            Check(FragmentShaderSource);
            const char* FragmentShaderCString = FragmentShaderSource->GetString().c_str();

            unsigned int FragmentShader;
            FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(FragmentShader, 1, &FragmentShaderCString, NULL);
            glCompileShader(FragmentShader);

            glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);

            if (!Success)
            {
                glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
                Logging::LogError("SpriteRendererComponent::OnSpawn()", "Fragment Shader: " + std::string(InfoLog));
                return;
            }
        //!Fragment Shader =======================================

        // Shader Program ========================================
            ShaderProgram = glCreateProgram();

            glAttachShader(ShaderProgram, VertexShader);
            glAttachShader(ShaderProgram, FragmentShader);
            glLinkProgram(ShaderProgram);

            glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
            if (!Success) {
                glGetProgramInfoLog(ShaderProgram, 512, NULL, InfoLog);
                Logging::LogError("SpriteRendererComponent::OnSpawn()", "Shader Program: " + std::string(InfoLog));
            }

            glDeleteShader(VertexShader); // delete shaders after shader program is created
            glDeleteShader(FragmentShader);
        //!Shader Program ========================================


        // Vertex Array Object ===================================

            // Generate and bind VAO, which stores the VBO and EBO
            glGenVertexArrays(1, &VertexArrayObject);
            glBindVertexArray(VertexArrayObject);

            // Generate and bind VBO, which stores vertex information
            glGenBuffers(1, &VertexBufferObject);
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

            // Generate and bind VBO, which stores element information
            glGenBuffers(1, &ElementBufferObject);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

        //!Vertex Array Object ===================================

    }
}

void SpriteRendererComponent::Render(CameraComponent& Camera)
{
    glm::mat4x4 ModelMatrix = GetParentEntity().GetModelMatrix();
    glm::mat4 ViewMatrix = Camera.GetViewMatrix();
    glm::mat4x4 ProjectionMatrix = Camera.GetProjectionMatrix();

    glm::mat4x4 MVPMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Bind shader & VAO so opengl knows what to draw
    glUseProgram(ShaderProgram);
    glBindVertexArray(VertexArrayObject);

    // Upload matrix to shader
    int MVPMatrixLocation = glGetUniformLocation(ShaderProgram, "MVP");
    glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVPMatrix));

    // draw elements, we have 6 elements so specify 6. For meshes we would
    // need a dynamic element count but sprites will always have 6 elements
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}