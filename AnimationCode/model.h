#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>

/**
 * @brief The Model class
 *
 * Loads all data from a Wavefront .obj file
 * IMPORTANT! Current only supports TRIANGLE meshes!
 *
 * Support for other meshes can be implemented by students
 *
 */
class Model
{
public:
    Model(QString filename);
    Model();

    // Used for glDrawArrays()
    QVector<QVector3D> getVertices();
    QVector<QVector3D> getNormals();
    QVector<QVector2D> getTextureCoords();

    // Used for interleaving into one buffer for glDrawArrays()
    QVector<float> getVNInterleaved();
    QVector<float> getVNTInterleaved();

    // Used for glDrawElements()
    QVector<QVector3D> getVertices_indexed();
    QVector<QVector3D> getNormals_indexed();
    QVector<QVector2D> getTextureCoords_indexed();
    QVector<unsigned>  getIndices();

    // Used for interleaving into one buffer for glDrawElements()
    QVector<float> getVNInterleaved_indexed();
    QVector<float> getVNTInterleaved_indexed();

    bool hasNormals();
    bool hasTextureCoords();
    int getNumTriangles();

    void unitize();

    QVector<float> meshData;

    // Buffers
    GLuint meshVAO;
    GLuint meshVBO;
    GLuint meshSize;

    // Texture
    GLuint texturePtr;

    // Transforms
    QMatrix3x3 meshNormalTransform;
    QMatrix4x4 meshTransform;
    QVector3D position;
    QVector3D animation;

    //Animation
    float scale = 1.f;
    float scaleChange = 0.f;
    float scaleMax = 1.f;
    float scaleMin = 1.f;
    bool grow = true;
    void updateScale();

    // Phong model constants.
    QVector4D material = {0.5, 0.5, 1, 5};

private:

    // OBJ parsing
    void parseVertex(QStringList tokens);
    void parseNormal(QStringList tokens);
    void parseTexture(QStringList tokens);
    void parseFace(QStringList tokens);

    // Alignment of data
    void alignData();
    void unpackIndexes();

    // For unitizing.
    void getBounds(QVector3D &min, QVector3D &max);

    // Intermediate storage of values
    QVector<QVector3D> vertices_indexed;
    QVector<QVector3D> normals_indexed;
    QVector<QVector2D> textureCoords_indexed;
    QVector<unsigned> indices;

    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector2D> textureCoords;

    // Utility storage
    QVector<unsigned> normal_indices;
    QVector<unsigned> texcoord_indices;
    QVector<QVector3D> norm;
    QVector<QVector2D> tex;

    bool hNorms = false;
    bool hTexs = false;
};

#endif // MODEL_H
