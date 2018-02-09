#ifndef TERRAINTINTIN_TERRAINSCENE_H
#define TERRAINTINTIN_TERRAINSCENE_H

#include "Scene.h"
#include "Terrain.h"

using namespace Eigen;

class TerrainScene : public Scene {
 public:

  enum TexturingMode {CONST_COLOR=0, TEXTURE=1, HEIGHTMAP=2, NORMALS=3, TEXCOORDS=4, TESSLEVEL=5};//if these values are changed make sure to change them in simple.frag the same way
  enum DrawMode {FILL, WIREFRAME, FILL_AND_WIREFRAME};
  enum CameraMode {FREE_FLY};
  enum TessellationMethod {NO_TESSELLATION, HARDWARE, INSTANCIATION};
  enum TessellationMode {CONSTANT = 0, ADAPTATIVE_FROM_POV, ADAPTATIVE_FROM_FIXED_POINT};
  enum AdaptativeMode {DISTANCE = 1, VIEWSPACE = 2};
  
  void initialize() override {
    _terrain.setHeightMap(QImage("../data/heightmaps/hm0_1024x1024.png"));
    _terrain.setTexture(QImage("../data/textures/rainbow.png"));

    loadShaders();
    
    _camera->setPosition(Eigen::Vector3f(10, 100, 10));
    _camera->setDirection(-Eigen::Vector3f(-10,10,-10));
    _camera->setViewport(600, 400);
    _camera->setSpeed(_terrain.getSize().norm() / 4000.f);

    _f->glEnable(GL_DEPTH_TEST);
    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _f->glPatchParameteri(GL_PATCH_VERTICES, 3);
  }

  void loadShaders(){
    if(_simplePrg)
      delete _simplePrg;
    if(_simpleTessPrg)
      delete _simpleTessPrg;
    //shader init
    _simplePrg = new QOpenGLShaderProgram();
    _simplePrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
    _simplePrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
    _simplePrg->link();

    _simpleTessPrg = new QOpenGLShaderProgram();
    _simpleTessPrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
    _simpleTessPrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
    _simpleTessPrg->addShaderFromSourceFile(QOpenGLShader::TessellationControl, "../data/shaders/simpleTess.tesc");
    _simpleTessPrg->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, "../data/shaders/simpleTess.tese");
    _simpleTessPrg->link();

    _defaultCamSpeed = _terrain.getSize().norm() / 6000.f;
    _camera->setPosition(Eigen::Vector3f(10, 100, 10));
    _camera->setDirection(-Eigen::Vector3f(-10,10,-10));
    _camera->setViewport(600, 400);
    _camera->setSpeed(_defaultCamSpeed);

    _f->glEnable(GL_DEPTH_TEST);
    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _f->glPatchParameteri(GL_PATCH_VERTICES, 3);
  }

  void render() override {
    
    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    if(_tessellationMethod == TessellationMethod::NO_TESSELLATION)
    {
      _simplePrg->bind();
      _f->glUniform1f(_simplePrg->uniformLocation("heightScale"), _heightScale);
      _f->glUniformMatrix4fv(_simplePrg->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
      _f->glUniformMatrix4fv(_simplePrg->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());
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
    } else if(_tessellationMethod == TessellationMethod::HARDWARE) {
      _simpleTessPrg->bind();

      _f->glUniformMatrix4fv(_simpleTessPrg->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
      _f->glUniformMatrix4fv(_simpleTessPrg->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());
      
      _f->glPatchParameteri(GL_PATCH_VERTICES, 3);
      
      _f->glUniform1f(_simpleTessPrg->uniformLocation("TessLevelInner"), _constantInnerTessellationLevel);
      Vector3f outerLvl = Vector3f::Constant(_constantOuterTessellationLevel);
      _f->glUniform3fv(_simpleTessPrg->uniformLocation("TessLevelOuter"), 1, outerLvl.data());
      _f->glUniform1f(_simpleTessPrg->uniformLocation("triEdgeSize"), _terrain.getTriEdgeSize());
      _f->glUniform1f(_simpleTessPrg->uniformLocation("heightScale"), _heightScale);

      _f->glUniform1i(_simpleTessPrg->uniformLocation("tessMethod"), _adaptativeTessellationMode);
      if (_tessellationMode == ADAPTATIVE_FROM_POV) {
        _f->glUniform3fv(_simpleTessPrg->uniformLocation("TessDistRefPos"), 1, _camera->position().data());
      } else if (_tessellationMode == ADAPTATIVE_FROM_FIXED_POINT) {
        /* TODO : Implement Placement of point on scene */
        _f->glUniform3fv(_simpleTessPrg->uniformLocation("TessDistRefPos"), 1, _camera->position().data());
      }
      
      if(_drawMode == DrawMode::FILL || _drawMode == DrawMode::FILL_AND_WIREFRAME){
      _f->glDepthFunc(GL_LESS);
      _simpleTessPrg->setUniformValue(_simpleTessPrg->uniformLocation("wireframe"), false);
      _simpleTessPrg->setUniformValue(_simpleTessPrg->uniformLocation("v_color"), QVector3D(1,0,0));
      _simpleTessPrg->setUniformValue(_simpleTessPrg->uniformLocation("texturing_mode"), _texMode);
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      _terrain.drawHardwareTessellation(*_simpleTessPrg);
      }
      
      if(_drawMode == DrawMode::WIREFRAME || _drawMode == DrawMode::FILL_AND_WIREFRAME){
      _f->glDepthFunc(GL_LEQUAL);
      _simpleTessPrg->setUniformValue(_simpleTessPrg->uniformLocation("wireframe"), true);
      _simpleTessPrg->setUniformValue(_simpleTessPrg->uniformLocation("v_color"), QVector3D(0,1,0));
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      _terrain.drawHardwareTessellation(*_simpleTessPrg);
      }
      
      _simpleTessPrg->release();
    }
    else{

    }
  }

  void update(float dt) override {
    _camera->update(dt);
  }

  void clean() override {
    delete _simplePrg;
    _terrain.clean();
  }

  QGroupBox* createDisplayGroupBox(){
    QGroupBox * displayGroupBox = new QGroupBox("Display");
    QVBoxLayout * displayLayout = new QVBoxLayout;
    displayGroupBox->setLayout(displayLayout);
    
    QLabel * drawModeLabel = new QLabel();
    drawModeLabel->setText("Drawing mode :");
    displayLayout->addWidget(drawModeLabel);
    
    QComboBox * drawModeCB = new QComboBox();
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
    
    QLabel * texturingLabel = new QLabel();
    texturingLabel->setText("Texturing mode :");
    displayLayout->addWidget(texturingLabel);
    
    QComboBox * texturingCB = new QComboBox();
    texturingCB->setEditable(false);
    texturingCB->addItem("Constant color", TexturingMode::CONST_COLOR);
    texturingCB->addItem("Texture", TexturingMode::TEXTURE);
    texturingCB->addItem("Height map", TexturingMode::HEIGHTMAP);
    texturingCB->addItem("Normal map", TexturingMode::NORMALS);
    texturingCB->addItem("Texture coordinates", TexturingMode::TEXCOORDS);
    texturingCB->addItem("Tessellation level", TexturingMode::TESSLEVEL);
    
    QObject::connect(texturingCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		     [this, texturingCB](int ind) {
		       int data = texturingCB->itemData(ind).toInt();
		       _texMode = (TexturingMode)data;
		     });
    
    displayLayout->addWidget(texturingCB);

    return displayGroupBox;
  }

  QGroupBox * createTessellationGroupBox(){
    QGroupBox * tessellationGroupBox = new QGroupBox("Tessellation");
    QVBoxLayout * tessellationLayout = new QVBoxLayout;
    tessellationGroupBox->setLayout(tessellationLayout);
    
    QLabel * tessellationMethodLabel = new QLabel();
    tessellationMethodLabel->setText("Tessellation method :");
    tessellationLayout->addWidget(tessellationMethodLabel);
    
    QComboBox * tessellationMethodCB = new QComboBox();
    tessellationMethodCB->setEditable(false);
    tessellationMethodCB->addItem("No tessellation", TessellationMethod::NO_TESSELLATION);
    tessellationMethodCB->addItem("Hardware tessellation", TessellationMethod::HARDWARE);
    tessellationMethodCB->addItem("Patch instanciation", TessellationMethod::INSTANCIATION);
    
    QObject::connect(tessellationMethodCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		     [this, tessellationMethodCB](int ind) {
		       int data = tessellationMethodCB->itemData(ind).toInt();
		       _tessellationMethod = (TessellationMethod)data;
		     });
    
    tessellationLayout->addWidget(tessellationMethodCB);
    
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    tessellationLayout->addWidget(line);
    
    QLabel * tessellationModeLabel = new QLabel();
    tessellationModeLabel->setText("Tessellation mode :");
    tessellationLayout->addWidget(tessellationModeLabel);
    
    QComboBox * tessellationModeCB = new QComboBox();
    tessellationModeCB->setEditable(false);
    tessellationModeCB->addItem("Constant", TessellationMode::CONSTANT);
    tessellationModeCB->addItem("Adaptative from POV", TessellationMode::ADAPTATIVE_FROM_POV);
    tessellationModeCB->addItem("Adaptative from marker", TessellationMode::ADAPTATIVE_FROM_FIXED_POINT);

    QFrame * constantModeSubMenu = new QFrame();
    constantModeSubMenu->setVisible(true);
    QFrame * adaptativeModeSubMenu = new QFrame();
    adaptativeModeSubMenu->setVisible(false);
    
    QObject::connect(tessellationModeCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		     [this, tessellationModeCB, constantModeSubMenu, adaptativeModeSubMenu](int ind) {
		       int data = tessellationModeCB->itemData(ind).toInt();
		       _tessellationMode = (TessellationMode)data;
		       switch(_tessellationMode){
		       case TessellationMode::CONSTANT:
			 constantModeSubMenu->setVisible(true);
			 adaptativeModeSubMenu->setVisible(false);
			 break;
		       case TessellationMode::ADAPTATIVE_FROM_POV:
		       case TessellationMode::ADAPTATIVE_FROM_FIXED_POINT:
			 constantModeSubMenu->setVisible(false);
			 adaptativeModeSubMenu->setVisible(true);
			 break;
		       }
		     });
    
    tessellationLayout->addWidget(tessellationModeCB);
    tessellationLayout->addWidget(constantModeSubMenu);
    tessellationLayout->addWidget(adaptativeModeSubMenu);

    QFormLayout * constantModeLayout = new QFormLayout;
    constantModeSubMenu->setLayout(constantModeLayout);
    QDoubleSpinBox * innerLvlSB = new QDoubleSpinBox;
    innerLvlSB->setSingleStep(0.1);
    innerLvlSB->setRange(1., 64.);
    QDoubleSpinBox * outerLvlSB = new QDoubleSpinBox;
    outerLvlSB->setSingleStep(0.1);
    outerLvlSB->setRange(1., 64.);

    QObject::connect(innerLvlSB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		     [this, outerLvlSB](double val){
		       _constantInnerTessellationLevel = val;
		     });
    QObject::connect(outerLvlSB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		     [this](double val){
		       _constantOuterTessellationLevel = val;
		     });
    constantModeLayout->addRow("Inner level", innerLvlSB);
    constantModeLayout->addRow("Outer level", outerLvlSB);


    QFormLayout * adaptativeModeLayout = new QFormLayout;
    adaptativeModeSubMenu->setLayout(adaptativeModeLayout);

    QComboBox * adaptativeTessModeCB = new QComboBox();
    adaptativeTessModeCB->setEditable(false);
    adaptativeTessModeCB->addItem("Distance", AdaptativeMode::DISTANCE);
    adaptativeTessModeCB->addItem("Viewspace", AdaptativeMode::VIEWSPACE);
    QObject::connect(adaptativeTessModeCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		     [this, adaptativeTessModeCB](int ind) {
		       int data = adaptativeTessModeCB->itemData(ind).toInt();
		       _adaptativeTessellationMode = (AdaptativeMode)data;
		     });
    
    QDoubleSpinBox * adaptativeFactorSB = new QDoubleSpinBox;
    innerLvlSB->setSingleStep(0.1);
    innerLvlSB->setRange(0., 100);

    QObject::connect(adaptativeFactorSB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		     [this](double val){
		       _adaptativeFactor = val;
		     });
    adaptativeModeLayout->addRow("LOD method", adaptativeTessModeCB);
    adaptativeModeLayout->addRow("Adaptative LOD factor", adaptativeFactorSB);
    

    tessellationLayout->addWidget(new QLabel("Height Scale Factor :"));

    QHBoxLayout * scaleLayout = new QHBoxLayout;
    QSpinBox * scaleSB = new QSpinBox();
    scaleSB->setMinimum(0);
    scaleSB->setMaximum(200);
    scaleSB->setSingleStep(1);
    scaleSB->setValue(50);
    QSlider * scaleSlider = new QSlider();
    scaleSlider->setOrientation(Qt::Horizontal);
    scaleSlider->setMinimum(0);
    scaleSlider->setMaximum(200);
    scaleSlider->setValue(50);

    QObject::connect(scaleSB, SIGNAL(valueChanged(int)),
                     scaleSlider, SLOT(setValue(int)));
    QObject::connect(scaleSlider, SIGNAL(valueChanged(int)),
                     scaleSB, SLOT(setValue(int)));
    QObject::connect(scaleSB, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
     [this](int val){
       _heightScale = val;
     });
    scaleLayout->addWidget(scaleSB);
    scaleLayout->addWidget(scaleSlider);

    tessellationLayout->addLayout(scaleLayout);

    return tessellationGroupBox;
  }

  QGroupBox* createCameraGroupBox(){
    QGroupBox *cameraGroupBox = new QGroupBox("Camera");
    QVBoxLayout *cameraLayout = new QVBoxLayout;
    cameraGroupBox->setLayout(cameraLayout);

    QLabel * cameraModeLabel = new QLabel();
    cameraModeLabel->setText("Camera mode :");

    QComboBox * cameraModeCB = new QComboBox();
    cameraModeCB->setEditable(false);
    cameraModeCB->addItem("FreeFly", CameraMode::FREE_FLY);
    
    QObject::connect(cameraModeCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		     [this, cameraModeCB](int ind) {
		       int data = cameraModeCB->itemData(ind).toInt();
		       _cameraMode = (CameraMode)data;
		     });
    
    cameraLayout->addWidget(cameraModeCB);
    
    QLabel * cameraSpeedLabel = new QLabel();
    cameraSpeedLabel->setText("Camera speed :");
    cameraLayout->addWidget(cameraSpeedLabel);

    QHBoxLayout * cameraSpeedLayout = new QHBoxLayout;
    QSpinBox * cameraSpeedSB = new QSpinBox();
    cameraSpeedSB->setMinimum(0);
    cameraSpeedSB->setMaximum(30);
    cameraSpeedSB->setSingleStep(1);
    cameraSpeedSB->setValue(15);
    QSlider * cameraSpeedSlider = new QSlider();
    cameraSpeedSlider->setOrientation(Qt::Horizontal);
    cameraSpeedSlider->setMinimum(0);
    cameraSpeedSlider->setMaximum(30);
    cameraSpeedSlider->setValue(15);
    
    QObject::connect(cameraSpeedSB, SIGNAL(valueChanged(int)),
		     cameraSpeedSlider, SLOT(setValue(int)));
    QObject::connect(cameraSpeedSlider, SIGNAL(valueChanged(int)),
		     cameraSpeedSB, SLOT(setValue(int)));
    QObject::connect(cameraSpeedSB, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		     [this](int val){
		       _camera->setSpeed(_defaultCamSpeed * val / 15.f);
		     });
    cameraSpeedLayout->addWidget(cameraSpeedSB);
    cameraSpeedLayout->addWidget(cameraSpeedSlider);

    cameraLayout->addLayout(cameraSpeedLayout);

    return cameraGroupBox;
  }
  
  virtual QDockWidget *createDock() {
    QDockWidget *dock = new QDockWidget("Options");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QFrame *frame = new QFrame(dock);
    QVBoxLayout * VLayout = new QVBoxLayout;

    QPushButton * reloadShadersButton = new QPushButton;
    reloadShadersButton->setText("Reload shaders");
    VLayout->addWidget(reloadShadersButton);

    QObject::connect(reloadShadersButton, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this](){
		       loadShaders();
		     });
    
    VLayout->setAlignment(Qt::AlignTop);
    VLayout->addWidget(createDisplayGroupBox());
    VLayout->addWidget(createCameraGroupBox());
    VLayout->addWidget(createTessellationGroupBox());
    frame->setLayout(VLayout);
    dock->setWidget(frame);
    return dock;
  }
    
  virtual void connectToMainWindow(const MainWindow& mw){
    QObject::connect(&mw, static_cast<void (MainWindow::*)(const QImage&)>(&MainWindow::loadedHeightMap),
		     [this](const QImage& im) {
		       this->_terrain.setHeightMap(im);
           _defaultCamSpeed = _terrain.getSize().norm() / 6000.f;
           _camera->setSpeed(_defaultCamSpeed);
		     });

    QObject::connect(&mw, static_cast<void (MainWindow::*)(const QImage&)>(&MainWindow::loadedTexture),
		     [this](const QImage& im) {
		       this->_terrain.setTexture(im);
		     });
  }

 private:
  QOpenGLShaderProgram * _simplePrg;
  QOpenGLShaderProgram * _simpleTessPrg;
  Terrain _terrain;

  float _constantInnerTessellationLevel = 1.;
  float _constantOuterTessellationLevel = 1.;
  float _adaptativeFactor = 1.;					    
  float _heightScale = 50.f;
  TexturingMode _texMode = TexturingMode::CONST_COLOR;
  DrawMode _drawMode = DrawMode::FILL;
  CameraMode _cameraMode = CameraMode::FREE_FLY;
  TessellationMethod _tessellationMethod = TessellationMethod::NO_TESSELLATION;
  TessellationMode _tessellationMode = TessellationMode::CONSTANT;
  AdaptativeMode _adaptativeTessellationMode = AdaptativeMode::DISTANCE;

  float _defaultCamSpeed;
};

#endif //TERRAINTINTIN_TERRAINSCENE_H