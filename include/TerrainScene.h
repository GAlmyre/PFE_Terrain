#ifndef TERRAINTINTIN_TERRAINSCENE_H
#define TERRAINTINTIN_TERRAINSCENE_H

#include "Scene.h"
#include "Terrain.h"

class TerrainScene : public Scene {
 public:

  enum TexturingMode {NONE=0, TEXTURE=1, HEIGHTMAP=2, NORMALS=3, TEXCOORDS=4};//if these values are changed make sure to change them in simple.frag the same way
  enum DrawMode {FILL, WIREFRAME, FILL_AND_WIREFRAME};
  
  void initialize() override {
    //shader init
    _simplePrg = new QOpenGLShaderProgram();
    _simplePrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
    _simplePrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
    _simplePrg->link();

    _camera->setPosition(Eigen::Vector3f(0,10,-5));
    _camera->setDirection(-Eigen::Vector3f(0,10,-5));
    _camera->setViewport(600, 400);

    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glEnable(GL_BLEND);
    _f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  void render() override {
    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _simplePrg->bind();

    _f->glUniformMatrix4fv(_simplePrg->uniformLocation("view_mat"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_simplePrg->uniformLocation("proj_mat"), 1, GL_FALSE, _camera->projectionMatrix().data());

    if(_drawMode == DrawMode::FILL || _drawMode == DrawMode::FILL_AND_WIREFRAME){
      _f->glDepthFunc(GL_LESS);
      _simplePrg->setUniformValue(_simplePrg->uniformLocation("wireframe"), false);
      _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(1,0,0));
      _simplePrg->setUniformValue(_simplePrg->uniformLocation("texturing_mode"), _texMode);
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      _terrain.draw(*_simplePrg);
    }

    if(_drawMode == DrawMode::WIREFRAME || _drawMode == DrawMode::FILL_AND_WIREFRAME){
      _f->glDepthFunc(GL_LEQUAL);
      _simplePrg->setUniformValue(_simplePrg->uniformLocation("wireframe"), true);
      _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(0,1,0));
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      _terrain.draw(*_simplePrg);
    }
    _simplePrg->release();
  }

  void update(float dt) override {
    _camera->update(dt);
  }

  void clean() override {
    delete _simplePrg;
    _terrain.clean();
  }

  virtual QDockWidget *createDock() {
    QDockWidget *dock = new QDockWidget("Options");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QFrame *frame = new QFrame(dock);
    QVBoxLayout * VLayout = new QVBoxLayout;
    VLayout->setAlignment(Qt::AlignTop);

    QGroupBox * displayGroupBox = new QGroupBox("Display", frame);
    QVBoxLayout * displayLayout = new QVBoxLayout;
    displayGroupBox->setLayout(displayLayout);
    
    QLabel * drawModeLabel = new QLabel(frame);
    drawModeLabel->setText("Drawing mode :");
    displayLayout->addWidget(drawModeLabel);
    
    QComboBox * drawModeCB = new QComboBox(frame);
    drawModeCB->setEditable(false);
    drawModeCB->addItem("Fill", DrawMode::FILL);
    drawModeCB->addItem("Wire-frame", DrawMode::WIREFRAME);
    drawModeCB->addItem("Fill + wire-frame", DrawMode::FILL_AND_WIREFRAME);
    
    QObject::connect(drawModeCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		     [this, drawModeCB](int ind) {
		       int data = drawModeCB->itemData(ind).toInt();
		       _drawMode = (DrawMode)data;
		     });

    displayLayout->addWidget(drawModeCB);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    displayLayout->addWidget(line);

    QLabel * texturingLabel = new QLabel(frame);
    texturingLabel->setText("Texturing mode :");
    displayLayout->addWidget(texturingLabel);
    
    QComboBox * texturingCB = new QComboBox(frame);
    texturingCB->setEditable(false);
    texturingCB->addItem("No texture", TexturingMode::NONE);
    texturingCB->addItem("Texture", TexturingMode::TEXTURE);
    texturingCB->addItem("Height map", TexturingMode::HEIGHTMAP);
    texturingCB->addItem("Normal map", TexturingMode::NORMALS);
    texturingCB->addItem("Texture coordinates", TexturingMode::TEXCOORDS);
    
    QObject::connect(texturingCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		     [this, texturingCB](int ind) {
		       int data = texturingCB->itemData(ind).toInt();
		       _texMode = (TexturingMode)data;
		     });

    displayLayout->addWidget(texturingCB);
    
    VLayout->addWidget(displayGroupBox);
    
    QLabel * cameraSpeedLabel = new QLabel(frame);
    cameraSpeedLabel->setText("Camera speed :");
    VLayout->addWidget(cameraSpeedLabel);

    QHBoxLayout * cameraSpeedLayout = new QHBoxLayout;
    QSpinBox * cameraSpeedSB = new QSpinBox(frame);
    cameraSpeedSB->setMinimum(0);
    cameraSpeedSB->setSingleStep(1);
    cameraSpeedSB->setValue(5);
    QSlider * cameraSpeedSlider = new QSlider(frame);
    cameraSpeedSlider->setOrientation(Qt::Horizontal);
    cameraSpeedSlider->setMinimum(0);
    cameraSpeedSlider->setMaximum(20);
    cameraSpeedSlider->setValue(5);
    
    QObject::connect(cameraSpeedSB, SIGNAL(valueChanged(int)),
		     cameraSpeedSlider, SLOT(setValue(int)));
    QObject::connect(cameraSpeedSlider, SIGNAL(valueChanged(int)),
		     cameraSpeedSB, SLOT(setValue(int)));
    QObject::connect(cameraSpeedSB, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		     [this](int val){
		       this->_camera->setSpeed((float)val/100);
		     });
    cameraSpeedLayout->addWidget(cameraSpeedSB);
    cameraSpeedLayout->addWidget(cameraSpeedSlider);

    VLayout->addLayout(cameraSpeedLayout);
    
    frame->setLayout(VLayout);
    dock->setWidget(frame);
    return dock;
  }
    
  virtual void connectToMainWindow(const MainWindow& mw){
    QObject::connect(&mw, static_cast<void (MainWindow::*)(const QImage&)>(&MainWindow::loadedHeightMap),
    [this](const QImage& im) {
    this->_terrain.setHeightMap(im);
    });
  }

 private:
  QOpenGLShaderProgram * _simplePrg;
  Terrain _terrain;
  
  TexturingMode _texMode = TexturingMode::NONE;
  DrawMode _drawMode = DrawMode::FILL;
  
};

#endif //TERRAINTINTIN_TERRAINSCENE_H
