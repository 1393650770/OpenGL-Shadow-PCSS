

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Material.h"
#include "LightDirectional.h"
#include "LightPoint.h"
#include "LightSpot.h"
#include "ParticleSystem.h"
#include <iostream>
#include <math.h>
#include<algorithm>
#include <stb_image.h>

//ShadowMapType=0为CommonShadowMap，=1为CSM，=2为PCSS
#define ShadowMapType 1
#define CSMAreaNum 3

using namespace std;
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(char const* path);
void ShowFrameRate();
void renderScene(const Shader& shader);
void renderCube();
void renderQuad();
float Z_Partitioning(int AreaNum, float Far, float Near);
void Calc_LightCameraFrustum(int SplitAreaCount, float LogarithmDistance, glm::mat4 lightViewProjMat, vector<vector<glm::vec3>>& LightFrustumVector, vector<float>& cascadedDistanceVector);
void Calc_CSMLightTransMat4(glm::vec3 LightDir, int SplitAreaCount, float LogarithmDistance, glm::mat4 lightViewProjMat, vector<glm::mat4>& CSMLightTransMat4Vector, vector<float>& cascadedDistanceVector);


Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

bool firstMouse = true;


unsigned int planeVAO;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

#pragma region Light Declare
    //position angle color
    LightDirectional lightD(
        glm::vec3(135.0f, 0, 0),
        glm::vec3(0.5f, 0.5f, 0.5f));

    LightPoint lightP0(glm::vec3(1.0f, 0, 0),
        glm::vec3(1.0f, 1.0f, 1.0f));

    LightPoint lightP1(glm::vec3(0, 1.0f, 0),
        glm::vec3(1.0f, 1.0f, 1.0f));

    LightPoint lightP2(glm::vec3(0, 0, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));

    LightPoint lightP3(glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));

    LightSpot lightS(glm::vec3(0, 8.0f, 0.0f),
        glm::vec3(135.0f, 0, 0),
        glm::vec3(1.0f, 1.0f, 1.0f));
#pragma endregion

    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//mac下使用                
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        cout << " Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glEnable(GL_DEPTH_TEST);


#if ShadowMapType==1
    Shader depthShader("D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\shadow_mapping_depth.vs", "D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\shadow_mapping_depth.fs", "D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\shadow_mapping_depth.gs");
#else
    Shader depthShader("D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\shadow_mapping_depth.vs", "D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\shadow_mapping_depth.fs");
#endif
    
    Shader renderShader("D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\shadow_mapping.vs", "D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\shadow_mapping.fs");
    Shader debugDepthQuad("D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\debug_quad.vs", "D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Shader\\debug_quad_depth.fs");



    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
    
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    const GLuint SHADOW_WIDTH = 1920, SHADOW_HEIGHT = 1080;
    GLuint depthMap;

#if ShadowMapType==1

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_DEPTH_COMPONENT32F,
        4096,
        4096,
        CSMAreaNum+1,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
        throw 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


#else
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    
    unsigned int shadowTypeUBO;
    int ShadowType = ShadowMapType;
    glGenBuffers(1, &shadowTypeUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, shadowTypeUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(int), &ShadowType, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, shadowTypeUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    unsigned int woodTexture = loadTexture("D:\\LearnOpenGL\\OpenGLProject\\Learning1\\Texture\\container.jpg");
   

#if ShadowMapType==1

    unsigned int matricesUBO;
    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    float zPationNum = Z_Partitioning(CSMAreaNum,camera.Far ,camera.Near );
    vector< glm::mat4> CSMLightTransMat4Vector;
    vector<float> cascadedDistanceVector;

#endif


    renderShader.use();
    renderShader.setInt("diffuseTexture", 0);
    renderShader.setInt("shadowMap", 1);
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 0);
    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
    float lightRadius = 100.f;
    float timeValue = 0;

    float startTime = static_cast<float>(glfwGetTime());

    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 7.5f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::vec3 lightDir = glm::vec3(0.0f) - lightPos;
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        //ShowFrameRate();
        processInput(window);
        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //渲染光源深度图
#if ShadowMapType==1
        
        Calc_CSMLightTransMat4(lightDir, CSMAreaNum, zPationNum, lightSpaceMatrix, CSMLightTransMat4Vector, cascadedDistanceVector);
        
        glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);

        glBufferData(GL_UNIFORM_BUFFER, CSMLightTransMat4Vector.size() *sizeof(glm::mat4), &CSMLightTransMat4Vector[0], GL_STATIC_DRAW);
        
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, depthMap,0);
        glViewport(0, 0, 4096, 4096);

        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        renderScene(depthShader);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
 

        

#else
        glCullFace(GL_FRONT);
        
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        renderScene(depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glCullFace(GL_BACK); 
#endif

        // 2. 渲染常规场景，并且生成阴影 
       // --------------------------------------------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, camera.Near, camera.Far);
        glm::mat4 view = camera.GetViewMatrix();
        renderShader.setMat4("projection", projection);
        renderShader.setMat4("view", view);
        // set light uniforms
        renderShader.setVec3("viewPos", camera.Position);
        renderShader.setVec3("lightPos", lightPos);
        renderShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        renderShader.SetUniform1f("lightRadius", lightRadius);


#if ShadowMapType==1
        renderShader.setInt("CSMAreaNum", CSMAreaNum);
        for (size_t i = 0; i < cascadedDistanceVector.size(); ++i)
        {
            //cout << "i: " << i << "  cascadedDistanceVector:" << cascadedDistanceVector[i] << endl;
            renderShader.setFloat("CSMPlaneDistances[" + std::to_string(i) + "]", cascadedDistanceVector[i]);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);



#else
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
#endif


        renderScene(renderShader);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    //glDeleteVertexArrays(1, &skyboxVAO);

    //glDeleteBuffers(1, &skyboxVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(const Shader& shader)
{
    // floor
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.25));
    shader.setMat4("model", model);
    renderCube();
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

//计算CSM的区域划分，采用对数的分割方法
float Z_Partitioning(int SplitAreaNum, float Far, float Near)
{

    return powf(Far / Near, 1.0 / float(SplitAreaNum));
}

//计算灯光空间的包围盒
void Calc_LightCameraFrustum(int SplitAreaCount,float LogarithmDistance, glm::mat4 lightViewProjMat, vector<vector<glm::vec3>>& LightFrustumVector,vector<float>& cascadedDistanceVector)
{
    //计算切分之后的顶点视锥
    cascadedDistanceVector.clear();
    vector<vector<glm::vec4>> cameraFovFrustum;
    float NearLightFrustumDistance = camera.Near;
    float aspect = camera.Width / camera.Height;
    glm::vec4 CameraPosition = glm::vec4(camera.Position.x, camera.Position.y, camera.Position.z, 1.f);
    glm::vec4 CameraFront =glm::normalize( glm::vec4(camera.Front.x, camera.Front.y, camera.Front.z, 0.f));
    glm::vec4 CameraUp = glm::vec4(camera.Up.x, camera.Up.y, camera.Up.z, 0.f);
    glm::vec4 CameraRight = glm::vec4(camera.Right.x, camera.Right.y, camera.Right.z, 0.f);
    for (int i = 0; i < SplitAreaCount-1; i++)
    {
        float FarLightFrustumDistance = camera.Near + powf(LogarithmDistance, i + 1);
        float nearHeight = tanf(camera.Fov / 2.0f) * NearLightFrustumDistance;
        float nearWidth = nearHeight * aspect;
        float farHeight= tanf(camera.Fov / 2.0f) * FarLightFrustumDistance;
        float farWidth = farHeight * aspect;
        glm::vec4 nearCenter = CameraPosition + CameraFront * NearLightFrustumDistance;
        glm::vec4 farCenter = CameraPosition + CameraFront * FarLightFrustumDistance;

        vector<glm::vec4> cameraFovFrustumSingleVector;
        //近平面左下点
        cameraFovFrustumSingleVector.push_back(nearCenter - CameraUp * nearHeight - CameraRight * nearWidth);
        //近平面左上点
        cameraFovFrustumSingleVector.push_back(nearCenter + CameraUp * nearHeight - CameraRight * nearWidth);
        //近平面右上点
        cameraFovFrustumSingleVector.push_back(nearCenter + CameraUp * nearHeight + CameraRight * nearWidth);
        //近平面右下点
        cameraFovFrustumSingleVector.push_back(nearCenter - CameraUp * nearHeight + CameraRight * nearWidth);
        
        //远平面左下点
        cameraFovFrustumSingleVector.push_back(farCenter - CameraUp * farHeight - CameraRight * farWidth);
        //远平面左下点
        cameraFovFrustumSingleVector.push_back(farCenter + CameraUp * farHeight - CameraRight * farWidth);
        //远平面左下点
        cameraFovFrustumSingleVector.push_back(farCenter + CameraUp * farHeight + CameraRight * farWidth);
        //远平面左下点
        cameraFovFrustumSingleVector.push_back(farCenter - CameraUp * farHeight + CameraRight * farWidth);

        cameraFovFrustum.push_back(cameraFovFrustumSingleVector);

        NearLightFrustumDistance = FarLightFrustumDistance;
        cascadedDistanceVector.push_back(NearLightFrustumDistance);
    }

    //转换到灯光空间
    for (int i = 0; i < cameraFovFrustum.size(); i++)
    {
        for (int j = 0; j < 8; j++)
        {
            cameraFovFrustum[i][j] = lightViewProjMat * cameraFovFrustum[i][j];
        }
    }

    //计算灯光空间下的包围盒
    vector<float> XVector;
    vector<float> YVector;
    vector<float> ZVector;
    LightFrustumVector.clear();
    for (int i = 0; i < cameraFovFrustum.size(); i++)
    {
        XVector.clear();
        YVector.clear();
        ZVector.clear();
        for (int j = 0; j < 8; j++)
        {
            XVector.push_back(cameraFovFrustum[i][j].x);
            YVector.push_back(cameraFovFrustum[i][j].y);
            ZVector.push_back(cameraFovFrustum[i][j].z);
        }

        float minX = *min_element(XVector.begin(), XVector.end());
        float minY = *min_element(YVector.begin(), YVector.end());
        float minZ = *min_element(ZVector.begin(), ZVector.end());

        float maxX = *max_element(XVector.begin(), XVector.end());
        float maxY = *max_element(YVector.begin(), YVector.end());
        float maxZ = *max_element(ZVector.begin(), ZVector.end());

        vector<glm::vec3> LightFrustumSingleVector;
        LightFrustumSingleVector.push_back(glm::vec3(minX, minY, minZ));
        LightFrustumSingleVector.push_back(glm::vec3(minX, maxY, minZ));
        LightFrustumSingleVector.push_back(glm::vec3(maxX, maxY, minZ));
        LightFrustumSingleVector.push_back(glm::vec3(maxX, minY, minZ));

        LightFrustumSingleVector.push_back(glm::vec3(minX, minY, maxZ + 10.0f));
        LightFrustumSingleVector.push_back(glm::vec3(minX, maxY, maxZ + 10.0f));
        LightFrustumSingleVector.push_back(glm::vec3(maxX, maxY, maxZ + 10.0f));
        LightFrustumSingleVector.push_back(glm::vec3(maxX, minY, maxZ + 10.0f));

        LightFrustumVector.push_back(LightFrustumSingleVector);
    }
}

//根据灯光包围盒生成新的灯光空间，并返回转换矩阵
void Calc_CSMLightTransMat4(glm::vec3 LightDir,int SplitAreaCount, float LogarithmDistance, glm::mat4 lightViewProjMat, vector<glm::mat4>& CSMLightTransMat4Vector, vector<float>& cascadedDistanceVector)
{

    vector<vector<glm::vec3>> LightFrustumVector;
    glm::mat4 LightSpaceToWorldSpaceMat4 =glm::inverse(lightViewProjMat);

    
    //计算灯光空间的包围盒

    LightFrustumVector.clear();
    
    Calc_LightCameraFrustum(SplitAreaCount, LogarithmDistance, lightViewProjMat, LightFrustumVector, cascadedDistanceVector);

    //计算包围盒灯光空间的转换矩阵
    CSMLightTransMat4Vector.clear();

    for (int i = 0; i < LightFrustumVector.size(); i++)
    {
        glm::vec3 NearCenterPosition = LightFrustumVector[i][0] + (LightFrustumVector[i][2] - LightFrustumVector[i][0]) * 0.5f;
        glm::vec3 FarCenterPosition = LightFrustumVector[i][4] + (LightFrustumVector[i][6] - LightFrustumVector[i][4]) * 0.5f;
        glm::vec4 NearCenterPosition_WorldSpace = glm::vec4( LightSpaceToWorldSpaceMat4 * glm::vec4(NearCenterPosition, 1.0f));
        NearCenterPosition_WorldSpace = NearCenterPosition_WorldSpace / NearCenterPosition_WorldSpace.w;
        glm::vec4 FarCenterPosition_WorldSpace = glm::vec4(LightSpaceToWorldSpaceMat4 * glm::vec4(FarCenterPosition, 1.0f));
        FarCenterPosition_WorldSpace = FarCenterPosition_WorldSpace / FarCenterPosition_WorldSpace.w;
        glm::mat4 lightProjection, lightView;
        LightDir = FarCenterPosition_WorldSpace - NearCenterPosition_WorldSpace;

        lightView = glm::lookAt(glm::vec3(NearCenterPosition_WorldSpace) , glm::vec3(NearCenterPosition_WorldSpace)+LightDir, glm::vec3(0.0, 1.0, 0.0));

        float minX = INT_MAX;
        float maxX = INT_MIN;
        float minY = INT_MAX;
        float maxY = INT_MIN;
        float minZ = INT_MAX;
        float maxZ = INT_MIN;

        for (int j = 0; j < LightFrustumVector[i].size(); j++)
        {

        }

        for (int j = 0; j < LightFrustumVector[i].size(); j++)
        {
            glm::vec4 LightSpacePoint_WorldSpace = glm::vec4(LightSpaceToWorldSpaceMat4 * glm::vec4(LightFrustumVector[i][j], 1.0f));
            LightSpacePoint_WorldSpace = LightSpacePoint_WorldSpace / LightSpacePoint_WorldSpace.w;
            glm::vec3 LightSpacePoint = lightView * LightSpacePoint_WorldSpace;
            
            minX = std::min(minX, LightSpacePoint.x);
            maxX = std::max(maxX, LightSpacePoint.x);
            minY = std::min(minY, LightSpacePoint.y);
            maxY = std::max(maxY, LightSpacePoint.y);
            minZ = std::min(minZ, LightSpacePoint.z);
            maxZ = std::max(maxZ, LightSpacePoint.z);
        }
 


        cout << minZ << " " << maxZ << endl;
        constexpr float zMult = 10.0f;
        if (minZ < 0)
        {
            minZ *= zMult;
        }
        else
        {
            minZ /= zMult;
        }
        if (maxZ < 0)
        {
            maxZ /= zMult;
        }
        else
        {
            maxZ *= zMult;
        }

        glm::vec2 tscale(2.0f / (maxX - minX), 2.0f / (maxY - minY));
        glm::vec2 toffset(-0.5f * (maxX + minX) * tscale.x, -0.5f * (maxY + minY) * tscale.y);

        glm::mat4 t_shad_crop = glm::mat4(1.0);
        t_shad_crop[0][0] = tscale.x;
        t_shad_crop[1][1] = tscale.y;
        t_shad_crop[0][3] = toffset.x;
        t_shad_crop[1][3] = toffset.y;
        t_shad_crop = glm::transpose(t_shad_crop);//注意glm按列储存，实际矩阵要转置

        glm::mat4 t_ortho = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -maxZ, -minZ);
        glm::mat4 t_projection = t_shad_crop * t_ortho;
        //lightProjection= glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

        CSMLightTransMat4Vector.push_back(t_projection * lightView);

    }
   
    return;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{    
    if (glfwGetMouseButton(window, 0))
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = lastX - xpos;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        // camera.updateCameraVectorsByQuat1(-xoffset, -yoffset);
        camera.updateCameraVectorsByQuat2(-xoffset, yoffset);
    }
    else
    {
        lastX = SCR_WIDTH / 2.0f;
        lastY = SCR_HEIGHT / 2.0f;
        firstMouse = true;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void ShowFrameRate()
{
    float fps = 1 / deltaTime;
    cout << fps << endl;
}


