#ifndef TERRAINTINTIN_TESSTESTSCENE_H
#define TERRAINTINTIN_TESSTESTSCENE_H

#include "Scene.h"

class TessTestScene : public Scene {
public:
  virtual void initialize() {
    GLfloat vertices[] = {
       // positions       // colors
       0.f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top right
       0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
//      -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f  // top left
    };

    GLuint indices[] = {
      0, 1, 2
//      0, 1, 3,
//      1, 2, 3
    };

    _f->glGenVertexArrays(1, &vao);
    _f->glGenBuffers(1, &vbo);
    _f->glGenBuffers(1, &ebo);

    _f->glBindVertexArray(vao);

    _f->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    _f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    _f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    _f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    _f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    _f->glEnableVertexAttribArray(0);

    _f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
    _f->glEnableVertexAttribArray(1);

    _f->glBindVertexArray(0);

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/testTess.vert")) {
      qDebug() << program->log();
    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/testTess.frag")) {
      qDebug() << program->log();
    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::TessellationControl, "../data/shaders/testTess.tesc")) {
      qDebug() << program->log();
    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, "../data/shaders/testTess.tese")) {
      qDebug() << program->log();
    }
    if (!program->link()) {
      qDebug() << program->log();
    }

    _innerTessIndex = _f->glGetProgramResourceLocation(program->programId(), GL_UNIFORM, "TessLevelInner");
    _outerTessIndex = _f->glGetProgramResourceLocation(program->programId(), GL_UNIFORM, "TessLevelOuter");

    _f->glPatchParameteri(GL_PATCH_VERTICES, 3);
  }

  virtual void render() {
    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//    _f->glCullFace(GL_FRONT);
//    _f->glEnable(GL_CULL_FACE);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _f->glUseProgram(program->programId());

    _f->glUniform1f(_innerTessIndex, TessLevelInner);
    _f->glUniform3fv(_outerTessIndex, 1, TessLevelOuter);

    _f->glBindVertexArray(vao);
    _f->glDrawElements(GL_PATCHES, 6, GL_UNSIGNED_INT, 0);
  }

  virtual void update(float dt) {
    _f->glUseProgram(program->programId());


  }

  virtual void clean() {
    _f->glDeleteVertexArrays(1, &vao);
    _f->glDeleteBuffers(1, &vbo);
    _f->glDeleteBuffers(1, &ebo);
  }

  virtual QDockWidget *createDock() {
    QDockWidget *dock = new QDockWidget("Options");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QFrame *frame = new QFrame(dock);
    QFormLayout *form = new QFormLayout;

    auto *innerSB = new QDoubleSpinBox(frame);
    auto *outer1SB = new QDoubleSpinBox(frame);
    auto *outer2SB = new QDoubleSpinBox(frame);
    auto *outer3SB = new QDoubleSpinBox(frame);

    innerSB->setValue(TessLevelInner);
    outer1SB->setValue(TessLevelOuter[0]);
    outer2SB->setValue(TessLevelOuter[1]);
    outer3SB->setValue(TessLevelOuter[2]);

    frame->connect(innerSB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this](double v) { TessLevelInner = v; });
    frame->connect(outer1SB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this](double v) { TessLevelOuter[0] = v; });
    frame->connect(outer2SB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this](double v) { TessLevelOuter[1] = v; });
    frame->connect(outer3SB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this](double v) { TessLevelOuter[2] = v; });

    form->addRow("Inner", innerSB);
    form->addRow("Outer1", outer1SB);
    form->addRow("Outer2", outer2SB);
    form->addRow("Outer3", outer3SB);

    frame->setLayout(form);
    dock->setWidget(frame);
    return dock;
  }

protected slots:
  void setTessLevelInner(int level) {
    TessLevelInner = level;
  }

private:
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  QOpenGLShaderProgram *program;

  GLuint _innerTessIndex;
  GLuint _outerTessIndex;

  float TessLevelInner = 1.0f;
  float TessLevelOuter[3] = { 1.0f, 1.0f, 1.0f };
};

#endif //TERRAINTINTIN_TESSTESTSCENE_H
