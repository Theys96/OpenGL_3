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

    glDeleteTextures(1, &texturePtr);

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
    // glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0, 1.0, 0.0, 1.0);

    createShaderProgram();
    loadMesh();
    loadTextures();

    // Initialize transformations
    updateProjectionTransform();
    updateModelTransforms();

    timer.start(1000.0 / 60.0);
}

void MainView::createShaderProgram()
{
    // Create Shader program
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_water.glsl");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_water.glsl");
    shaderProgram.link();
    shaderProgram.bind();

    // Get the uniforms for the shader.
    uniformModelViewTransform  = shaderProgram.uniformLocation("modelViewTransform");
    uniformProjectionTransform = shaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransform     = shaderProgram.uniformLocation("normalTransform");
    uniformMaterial            = shaderProgram.uniformLocation("material");
    uniformLightPosition       = shaderProgram.uniformLocation("lightPosition");
    uniformLightColour         = shaderProgram.uniformLocation("lightColour");
    uniformTextureSampler      = shaderProgram.uniformLocation("textureSampler");
    uniformTime                = shaderProgram.uniformLocation("time");

    // Wave uniforms
    GLint uniformAmp = shaderProgram.uniformLocation("amp");
    GLint uniformFreq = shaderProgram.uniformLocation("freq");
    GLint uniformPhase = shaderProgram.uniformLocation("phase");
    glUniform1fv(uniformAmp, 2, amp.data());
    glUniform1fv(uniformFreq, 2, freq.data());
    glUniform1fv(uniformPhase, 2, phase.data());
}

void MainView::loadMesh()
{
    Model model(":/models/grid.obj");
    model.unitize();
    QVector<float> meshData = model.getVNTInterleaved();

    this->meshSize = model.getVertices().size();

    // Generate VAO
    glGenVertexArrays(1, &meshVAO);
    glBindVertexArray(meshVAO);

    // Generate VBO
    glGenBuffers(1, &meshVBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

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

void MainView::loadTextures()
{
    glGenTextures(1, &texturePtr);
    loadTexture(":/textures/sea-map.png", texturePtr);
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
    glClearColor(0.2f, 0.5f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateUniforms();

    // Set the texture and draw the mesh.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texturePtr);

    glBindVertexArray(meshVAO);
    glDrawArrays(GL_TRIANGLES, 0, meshSize);

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

void MainView::updateUniforms()
{
    glUniformMatrix4fv(uniformProjectionTransform, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformModelViewTransform, 1, GL_FALSE, meshTransform.data());
    glUniformMatrix3fv(uniformNormalTransform, 1, GL_FALSE, meshNormalTransform.data());

    glUniform4fv(uniformMaterial, 1, &material[0]);
    glUniform3fv(uniformLightPosition, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColour, 1, &lightColour[0]);

    glUniform1i(uniformTextureSampler, 0);
    glUniform1f(uniformTime, ++t);
}

void MainView::updateProjectionTransform()
{
    float aspect_ratio = static_cast<float>(width()) / static_cast<float>(height());
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60, aspect_ratio, 0.2, 20);
}

void MainView::updateModelTransforms()
{
    meshTransform.setToIdentity();
    meshTransform.translate(0, 0, -4);
    meshTransform.scale(scale);
    meshTransform.rotate(QQuaternion::fromEulerAngles(rotation));
    meshTransform.rotate(-60, QVector3D(1,0,0));
    meshNormalTransform = meshTransform.normalMatrix();

    update();
}

// --- OpenGL cleanup helpers

void MainView::destroyModelBuffers()
{
    glDeleteBuffers(1, &meshVBO);
    glDeleteVertexArrays(1, &meshVAO);
}

// --- Public interface

void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
    rotation = { static_cast<float>(rotateX), static_cast<float>(rotateY), static_cast<float>(rotateZ) };
    updateModelTransforms();
}

void MainView::setScale(int newScale)
{
    scale = static_cast<float>(newScale) / 100.f;
    updateModelTransforms();
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

