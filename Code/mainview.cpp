#include "mainview.h"
#include "model.h"
#include "vertex.h"

#include <math.h>
#include <QDateTime>

/**
 * @brief MainView::MainView
 *
 * Constructor of MainView
 *
 * @param parent
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
    qDebug() << "MainView constructor";

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView() {
    debugLogger->stopLogging();

    qDebug() << "MainView destructor";

    glDeleteTextures(1, &(models[0].texturePtr));
    glDeleteTextures(1, &(models[1].texturePtr));
    glDeleteTextures(1, &(models[2].texturePtr));
    glDeleteTextures(1, &(models[3].texturePtr));

    destroyModelBuffers();
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL
 *
 * Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions
 */
void MainView::initializeGL() {
    qDebug() << ":: Initializing OpenGL";
    initializeOpenGLFunctions();

    debugLogger = new QOpenGLDebugLogger();
    connect( debugLogger, SIGNAL( messageLogged( QOpenGLDebugMessage ) ),
             this, SLOT( onMessageLogged( QOpenGLDebugMessage ) ), Qt::DirectConnection );

    if ( debugLogger->initialize() ) {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging( QOpenGLDebugLogger::SynchronousLogging );
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0, 0.5, 0.0, 0.0);

    createShaderProgram();
    loadMesh();
    loadTextures();

    // Initialize transformations
    updateProjectionTransform();
    for(int i=0; i<4; i++){
        updateModelTransforms(i);
    }
    // Initialise animation
    rotationAmount = 1;
    timer.start(1000.0 / 60.0);
}

void MainView::createShaderProgram()
{
    // Create Normal Shader program
    normalShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_normal.glsl");
    normalShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_normal.glsl");
    normalShaderProgram.link();

    // Create Gouraud Shader program
    gouraudShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_gouraud.glsl");
    gouraudShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_gouraud.glsl");
    gouraudShaderProgram.link();

    // Create Phong Shader program
    phongShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_phong.glsl");
    phongShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_phong.glsl");
    phongShaderProgram.link();

    // Get the uniforms for the normal shader.
    uniformModelViewTransformNormal  = normalShaderProgram.uniformLocation("modelViewTransform");
    uniformProjectionTransformNormal = normalShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformNormal     = normalShaderProgram.uniformLocation("normalTransform");

    // Get the uniforms for the gouraud shader.
    uniformModelViewTransformGouraud  = gouraudShaderProgram.uniformLocation("modelViewTransform");
    uniformProjectionTransformGouraud = gouraudShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformGouraud     = gouraudShaderProgram.uniformLocation("normalTransform");
    uniformMaterialGouraud            = gouraudShaderProgram.uniformLocation("material");
    uniformLightPositionGouraud       = gouraudShaderProgram.uniformLocation("lightPosition");
    uniformLightColourGouraud         = gouraudShaderProgram.uniformLocation("lightColour");
    uniformTextureSamplerGouraud      = gouraudShaderProgram.uniformLocation("textureSampler");

    // Get the uniforms for the phong shader.
    uniformModelViewTransformPhong  = phongShaderProgram.uniformLocation("modelViewTransform");
    uniformProjectionTransformPhong = phongShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformPhong     = phongShaderProgram.uniformLocation("normalTransform");
    uniformMaterialPhong            = phongShaderProgram.uniformLocation("material");
    uniformLightPositionPhong       = phongShaderProgram.uniformLocation("lightPosition");
    uniformLightColourPhong         = phongShaderProgram.uniformLocation("lightColour");
    uniformTextureSamplerPhong      = phongShaderProgram.uniformLocation("textureSampler");
}

void MainView::loadMesh()
{
    models[0] = Model(":/models/sickleobj.obj");
    models[2] = Model(":/models/sickleobj.obj");
    models[1] = Model(":/models/cat.obj");
    models[3] = Model(":/models/cat.obj");

    models[0].position = QVector3D(0,0.8,-4);
    models[1].position = QVector3D(-1.5,-0.8,-4);
    models[2].position = QVector3D(0,-0.8,-4);
    models[3].position = QVector3D(1.5,-0.8,-4);

    models[0].animation = QVector3D(0,1,0);
    models[1].animation = QVector3D(1,0,0);
    models[2].animation = QVector3D(1,0,0);
    models[3].animation = QVector3D(-1,0,0);



    for(int i=0; i<4; i++){

    models[i].unitize();
    models[i].meshData = models[i].getVNTInterleaved(); //QVector<float> meshData = model.getVNTInterleaved();
    models[i].meshSize = models[i].getVertices().size();//this->meshSize = model.getVertices().size();

    // Generate VAO
    glGenVertexArrays(1, &(models[i].meshVAO));
    glBindVertexArray(models[i].meshVAO);

    // Generate VBO
    glGenBuffers(1, &(models[i].meshVBO));
    glBindBuffer(GL_ARRAY_BUFFER, models[i].meshVBO);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, models[i].meshData.size() * sizeof(float), models[i].meshData.data(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Set vertex normals to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set vertex texture coordinates to location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    }

}

void MainView::loadTextures()
{
      glGenTextures(1, &(models[0].texturePtr));
      glGenTextures(1, &(models[1].texturePtr));
      glGenTextures(1, &(models[2].texturePtr));
      glGenTextures(1, &(models[3].texturePtr));
      loadTexture(":/textures/initialShadingGroup_Base_Color.png", models[0].texturePtr);
      loadTexture(":/textures/initialShadingGroup_Base_Color.png", models[2].texturePtr);
      loadTexture(":/textures/cat_diff.png", models[1].texturePtr);
      loadTexture(":/textures/cat_diff.png", models[3].texturePtr);
}

void MainView::loadTexture(QString file, GLuint texturePtr)
{
    // Set texture parameters.
    glBindTexture(GL_TEXTURE_2D, texturePtr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Push image data to texture.
    QImage image(file);
    QVector<quint8> imageData = imageToBytes(image);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
}

// --- OpenGL drawing

/**
 * @brief MainView::paintGL
 *
 * Actual function used for drawing to the screen
 *
 */
void MainView::paintGL() {
    // Clear the screen before rendering
    glClearColor(0.5f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Choose the selected shader.
    QOpenGLShaderProgram *shaderProgram;
    for(int i=0; i<4; i++) {

        switch (currentShader) {
        case NORMAL:
            shaderProgram = &normalShaderProgram;
            shaderProgram->bind();
            updateNormalUniforms(i);
            break;
        case GOURAUD:
            shaderProgram = &gouraudShaderProgram;
            shaderProgram->bind();
            updateGouraudUniforms(i);
            break;
        case PHONG:
            shaderProgram = &phongShaderProgram;
            shaderProgram->bind();
            updatePhongUniforms(i);
            break;
        }

        if(initialised) {
            updateModelTransforms(i);
        }

        // Set the texture and draw the mesh.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, models[i].texturePtr);
        glBindVertexArray(models[i].meshVAO);
        glDrawArrays(GL_TRIANGLES, 0, models[i].meshSize);
    }
    initialised = true;
    shaderProgram->release();
}

/**
 * @brief MainView::resizeGL
 *
 * Called upon resizing of the screen
 *
 * @param newWidth
 * @param newHeight
 */
void MainView::resizeGL(int newWidth, int newHeight)
{
    Q_UNUSED(newWidth)
    Q_UNUSED(newHeight)
    updateProjectionTransform();
}

void MainView::updateNormalUniforms(int i)
{
    glUniformMatrix4fv(uniformProjectionTransformNormal, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformModelViewTransformNormal, 1, GL_FALSE, models[i].meshTransform.data());
    glUniformMatrix3fv(uniformNormalTransformNormal, 1, GL_FALSE, models[i].meshNormalTransform.data());
}

void MainView::updateGouraudUniforms(int i)
{
    glUniformMatrix4fv(uniformProjectionTransformGouraud, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformModelViewTransformGouraud, 1, GL_FALSE, models[i].meshTransform.data());
    glUniformMatrix3fv(uniformNormalTransformGouraud, 1, GL_FALSE, models[i].meshNormalTransform.data());

    glUniform4fv(uniformMaterialGouraud, 1, &material[0]);
    glUniform3fv(uniformLightPositionGouraud, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColourGouraud, 1, &lightColour[0]);

    glUniform1i(uniformTextureSamplerGouraud, 0); // Redundant now, but useful when you have multiple textures.
}

void MainView::updatePhongUniforms(int i)
{
    glUniformMatrix4fv(uniformProjectionTransformPhong, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformModelViewTransformPhong, 1, GL_FALSE, models[i].meshTransform.data());
    glUniformMatrix3fv(uniformNormalTransformPhong, 1, GL_FALSE, models[i].meshNormalTransform.data());

    glUniform4fv(uniformMaterialPhong, 1, &material[0]);
    glUniform3fv(uniformLightPositionPhong, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColourPhong, 1, &lightColour[0]);

    glUniform1i(uniformTextureSamplerGouraud, 0);
}

void MainView::updateProjectionTransform()
{
    float aspect_ratio = static_cast<float>(width()) / static_cast<float>(height());
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60, aspect_ratio, 0.2, 20);
    projectionTransform.translate(0,0,-4);
    projectionTransform.rotate(QQuaternion::fromEulerAngles(rotation));
    projectionTransform.translate(0,0,4);
}

void MainView::updateModelTransforms(int i)
{

    models[i].meshTransform.setToIdentity();
    models[i].meshTransform.translate(models[i].position);
    models[i].meshTransform.scale(scale);
    rotationAmount++;
    if(i==1 || i==3) {
        models[i].meshTransform.rotate(90, QVector3D(0,-1,0));
    }
    models[i].meshTransform.rotate(rotationAmount, models[i].animation);
    models[i].meshNormalTransform = models[i].meshTransform.normalMatrix();

    update();
}

// --- OpenGL cleanup helpers

void MainView::destroyModelBuffers()
{
    for (int i=0; i<4; i++) {
        glDeleteBuffers(1, &(models[i].meshVBO));
        glDeleteVertexArrays(1, &(models[i].meshVAO));
    }
}

// --- Public interface

void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
    rotation = { static_cast<float>(rotateX), static_cast<float>(rotateY), static_cast<float>(rotateZ) };
    for(int i=0; i<4; i++){
    updateProjectionTransform();
    }
}

void MainView::setScale(int newScale)
{
    scale = static_cast<float>(newScale) / 100.f;
    for(int i=0; i<4; i++){
    updateModelTransforms(i);
    }
}

void MainView::setShadingMode(ShadingMode shading)
{
    qDebug() << "Changed shading to" << shading;
    currentShader = shading;
}

// --- Private helpers

/**
 * @brief MainView::onMessageLogged
 *
 * OpenGL logging function, do not change
 *
 * @param Message
 */
void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
    qDebug() << " → Log:" << Message;
}

