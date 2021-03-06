#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "model.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QVector3D>
#include <QImage>
#include <QVector>
#include <memory>
#include <QMatrix4x4>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

    QOpenGLDebugLogger *debugLogger;
    QTimer timer; // timer used for animation

    QOpenGLShaderProgram shaderProgram;

    // Uniforms for the phong shader.
    GLint uniformModelViewTransform;
    GLint uniformProjectionTransform;
    GLint uniformNormalTransform;

    GLint uniformMaterial;
    GLint uniformLightPosition;
    GLint uniformLightColour;

    GLint uniformTextureSampler;

    GLint uniformTime;
    GLfloat t = 0;

    // Buffers
    GLuint meshVAO;
    GLuint meshVBO;
    GLuint meshSize;

    // Texture
    GLuint texturePtr;

    // Transforms
    float scale = 2.f;
    QVector3D rotation;
    QMatrix4x4 projectionTransform;
    QMatrix3x3 meshNormalTransform;
    QMatrix4x4 meshTransform;

    // Phong model constants.
    QVector4D material = {0.4, 0.6, 1, 10};
    QVector3D lightPosition = {0, 3, 3};
    QVector3D lightColour = {1, 1, 1};

    // Waves
    QVector<GLfloat> amp = {0.02, 0.05, 0.03, 0.01};
    QVector<GLfloat> freq = {5.0, 2.0, 1.0, 10.0};
    QVector<GLfloat> phase = {0.2, 0.0, 0.5, 0.0};

    /*
     * Square waves:
    QVector<GLfloat> amp = {
        0.2/(2*1 - 1),
        0.2/(2*2 - 1),
        0.2/(2*3 - 1),
        0.2/(2*4 - 1),
        0.2/(2*5 - 1),
        0.2/(2*6 - 1),
        0.2/(2*7 - 1),
        0.2/(2*8 - 1),
    };
    QVector<GLfloat> freq = {
        2*1 - 1,
        2*2 - 1,
        2*3 - 1,
        2*4 - 1,
        2*5 - 1,
        2*6 - 1,
        2*7 - 1,
        2*8 - 1
    };
    QVector<GLfloat> phase = {0.0, 0.0, 0.0, 0.0};
    */

public:
    enum ShadingMode : GLuint
    {
        PHONG = 0, NORMAL, GOURAUD
    };

    MainView(QWidget *parent = 0);
    ~MainView();

    // Functions for widget input events
    void setRotation(int rotateX, int rotateY, int rotateZ);
    void setScale(int scale);
    void setShadingMode(ShadingMode shading);

protected:
    void initializeGL();
    void resizeGL(int newWidth, int newHeight);
    void paintGL();

    // Functions for keyboard input events
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

    // Function for mouse input events
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);

private slots:
    void onMessageLogged( QOpenGLDebugMessage Message );

private:
    void createShaderProgram();
    void loadMesh();

    // Loads texture data into the buffer of texturePtr.
    void loadTextures();
    void loadTexture(QString file, GLuint texturePtr);

    void destroyModelBuffers();

    void updateProjectionTransform();
    void updateModelTransforms();

    void updateUniforms();

    // Useful utility method to convert image to bytes.
    QVector<quint8> imageToBytes(QImage image);

    // The current shader to use.
    ShadingMode currentShader = PHONG;
};

#endif // MAINVIEW_H
