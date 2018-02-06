#ifndef TERRAINTINTIN_TESSTESTSCENE_H
#define TERRAINTINTIN_TESSTESTSCENE_H

#include "Scene.h"

class TessTestScene : public Scene {
public:
  virtual void initialize() {
    GLfloat vertices[] = {
       // positions       // colors
       0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top right
       0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
      -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f  // top left
    };

    GLuint indices[] = {
      0, 1, 3,
      1, 2, 3
    };

    _funcs->glGenVertexArrays(1, &vao);
    _funcs->glGenBuffers(1, &vbo);
    _funcs->glGenBuffers(1, &ebo);

    _funcs->glBindVertexArray(vao);

    _funcs->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    _funcs->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    _funcs->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    _funcs->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    _funcs->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    _funcs->glEnableVertexAttribArray(0);

    _funcs->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
    _funcs->glEnableVertexAttribArray(1);

    _funcs->glBindVertexArray(0);

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/testTess.vert")) {
      qDebug() << program->log();
    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/testTess.frag")) {
      qDebug() << program->log();
    }
//    if (!program->addShaderFromSourceFile(QOpenGLShader::TessellationControl, "../data/shaders/testTess.tesc")) {
//      qDebug() << program->log();
//    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, "../data/shaders/testTess.tese")) {
      qDebug() << program->log();
    }
    if (!program->link()) {
      qDebug() << program->log();
    }

    _funcs->glPatchParameteri(GL_PATCH_VERTICES, 3);
    const GLfloat innerLevel[] = { 2 };
    const GLfloat outerLevel[] = { 1, 1, 1 };
    _funcs->glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, innerLevel);
    _funcs->glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outerLevel);
  }

  virtual void render() {
    _funcs->glClearColor(0.2, 0.2, 0.2, 1.0);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//    _funcs->glCullFace(GL_FRONT);
//    _funcs->glEnable(GL_CULL_FACE);
    _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _funcs->glUseProgram(program->programId());
    _funcs->glBindVertexArray(vao);
    _funcs->glDrawElements(GL_PATCHES, 6, GL_UNSIGNED_INT, 0);
  }

  virtual void update(float dt) {

  }

  virtual void clean() {
    _funcs->glDeleteVertexArrays(1, &vao);
    _funcs->glDeleteBuffers(1, &vbo);
    _funcs->glDeleteBuffers(1, &ebo);
  }

private:
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  QOpenGLShaderProgram *program;
};

#endif //TERRAINTINTIN_TESSTESTSCENE_H
