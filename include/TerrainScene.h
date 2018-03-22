#ifndef TERRAINTINTIN_TERRAINSCENE_H
#define TERRAINTINTIN_TERRAINSCENE_H

#include "Scene.h"
#include "Terrain.h"
#include "DirectionalLight.h"
#include "Sphere.h"
#include "BenchmarkWindow.h"

#include "VariableOption.h"
#include "ComboBoxOption.h"

#include <chrono>

using namespace Eigen;

class TerrainScene : public Scene {
 public:
  enum TexturingMode {CONST_COLOR=0, TEXTURE=1, HEIGHTMAP=2, NORMALS=3, TEXCOORDS=4, TESSLEVEL=5};//if these values are changed make sure to change them in simple.frag the same way
  enum DrawMode {FILL, WIREFRAME, FILL_AND_WIREFRAME};
  enum CameraMode {FREE_FLY};
  enum TessellationMethod {NO_TESSELLATION, HARDWARE, INSTANCIATION};
  enum TessellationMode {CONSTANT = 0, ADAPTATIVE_FROM_POV, ADAPTATIVE_FROM_FIXED_POINT};
  enum AdaptativeMode {DISTANCE = 0, VIEWSPACE = 1, CONTENT = 2};

public:
  TerrainScene() : Scene(), _camera(new FreeFlyCamera) {}

  void initialize() override {
    _terrain.init();
    _mainWindow->loadHeightMap("../data/heightmaps/semnoz.png");
    _terrain.setTexture(QImage("../data/textures/sol.jpg"));
    _colormap = new QOpenGLTexture(QImage("../data/textures/parula_colormap.png").mirrored());
    _colormap->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
    _colormap->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);

    loadShaders();

    _needShaderReloading = false;

    _defaultCamSpeed = _terrain.getSize().norm() / 6000.f;
    _camera->setPosition(Eigen::Vector3f(10, 100, 10));
    _camera->setDirection(-Eigen::Vector3f(-10,10,-10));
    _camera->setViewport(600, 400);
    _camera->setSpeed(_defaultCamSpeed);

    _testSphere.generate(1.f);
    _testSphere.init(_f);
    _testSphere._transformation = Translation3f(200.f, 100.f, 200.f);

    _f->glEnable(GL_DEPTH_TEST);
    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _f->glPatchParameteri(GL_PATCH_VERTICES, 3);

    // Benchmarking queries
    _f->glGenQueries(1, &_primGenQuery);
    _f->glGenQueries(1, &_gpuTimeQuery);
  }

  void loadShaders(){
    if(_simplePrg)
      delete _simplePrg;
    if(_simpleTessPrg)
      delete _simpleTessPrg;
    if(_fillPrg)
      delete _fillPrg;
    if(_simpleInstTessPrg)
      delete _simpleInstTessPrg;

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

    _fillPrg = new QOpenGLShaderProgram();
    _fillPrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/fill.vert");
    _fillPrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/fill.frag");
    _fillPrg->link();

    _normalPrg = new QOpenGLShaderProgram();
    _normalPrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
    _normalPrg->addShaderFromSourceFile(QOpenGLShader::TessellationControl, "../data/shaders/simpleTess.tesc");
    _normalPrg->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, "../data/shaders/simpleTess.tese");
    _normalPrg->addShaderFromSourceFile(QOpenGLShader::Geometry, "../data/shaders/normals.geom");
    _normalPrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/fill.frag");
    _normalPrg->link();

    _simpleInstTessPrg = new QOpenGLShaderProgram();
    _simpleInstTessPrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simpleInstTess.vert");
    _simpleInstTessPrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
    _simpleInstTessPrg->link();
  }

  void render() override {
    if (_needShaderReloading) {
      loadShaders();
      _needShaderReloading = false;
    }

    /* Benchmarking */
    _f->glGetQueryObjectuiv(_primGenQuery, GL_QUERY_RESULT, &_primGens);
    _f->glGetQueryObjectuiv(_gpuTimeQuery, GL_QUERY_RESULT, &_gpuTime); // GPU time in ns

    _bench->addNewData(BenchmarkWindow::Data(_primGens, _gpuTime / 1000000.0, _cpuTime.count(), _lodTime.count()));

    // Begin GPU Time count
    _f->glBeginQuery(GL_TIME_ELAPSED, _gpuTimeQuery);
    // Begin CPU Time count
    auto cpuTimeStart = std::chrono::high_resolution_clock::now();

    _f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Shader Program selection */
    QOpenGLShaderProgram *currentPrg = nullptr;
    switch (_tessellationMethod) {
      case TessellationMethod::NO_TESSELLATION:
        currentPrg = _simplePrg;
        break;
      case TessellationMethod::HARDWARE:
        currentPrg = _simpleTessPrg;
        break;
      case TessellationMethod::INSTANCIATION:
        currentPrg = _simpleInstTessPrg;
        break;
      default:
        return;
    }

    /* Set common uniforms */
    currentPrg->bind();

    currentPrg->setUniformValue(currentPrg->uniformLocation("Ka"), _ambientCoef);
    currentPrg->setUniformValue(currentPrg->uniformLocation("Kd"), _diffuseCoef);
    currentPrg->setUniformValue(currentPrg->uniformLocation("Ks"), _specularCoef);
    currentPrg->setUniformValue(currentPrg->uniformLocation("shininess"), _shininessCoef);
    currentPrg->setUniformValue(currentPrg->uniformLocation("distanceFog"), _distFog);
    _f->glUniform3fv(currentPrg->uniformLocation("fogColor"), 1, _fogColor.data());
    _f->glUniform3fv(currentPrg->uniformLocation("lightDirection"), 1, _light.getDirection().data());
    _f->glUniform3fv(currentPrg->uniformLocation("lightColor"), 1, _light.getColor().data());

    _f->glUniformMatrix4fv(currentPrg->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(currentPrg->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());

    _colormap->bind(3);
    currentPrg->setUniformValue(currentPrg->uniformLocation("colormap"), 3);

    if (_tessellationMethod == HARDWARE || _tessellationMethod == INSTANCIATION) {
      _f->glPatchParameteri(GL_PATCH_VERTICES, 3);

      _f->glUniform1f(currentPrg->uniformLocation("TessLevelInner"), _constantInnerTessellationLevel);
      Vector3f outerLvl = Vector3f::Constant(_constantOuterTessellationLevel);
      _f->glUniform3fv(currentPrg->uniformLocation("TessLevelOuter"), 1, outerLvl.data());
      _f->glUniform1f(currentPrg->uniformLocation("triEdgeSize"), _terrain.getTriEdgeSize());
      _f->glUniform1f(currentPrg->uniformLocation("heightScale"), _terrain.heightScale());
      _f->glUniform2fv(currentPrg->uniformLocation("viewport"), 1, _camera->viewport().data());
      _f->glUniform1f(currentPrg->uniformLocation("adaptativeFactor"), _adaptativeFactor);
    }

    _f->glUniform1i(currentPrg->uniformLocation("tessMethod"),
                    (_tessellationMode == CONSTANT) ? CONSTANT : _adaptativeTessellationMode + 1);

    if (_tessellationMode == ADAPTATIVE_FROM_POV) {
      _f->glUniform3fv(currentPrg->uniformLocation("TessDistRefPos"), 1, _camera->position().data());
    } else if (_tessellationMode == ADAPTATIVE_FROM_FIXED_POINT) {
      /* TODO : Implement Placement of point on scene */
      _f->glUniform3fv(currentPrg->uniformLocation("TessDistRefPos"), 1, _camera->position().data());
    }

    /* LOD Computation */
    auto lodTimeStart = std::chrono::high_resolution_clock::now();
    if(_tessellationMethod == INSTANCIATION) {
      Matrix4f VPMat = _camera->projectionMatrix()*_camera->viewMatrix().matrix();
      _terrain.computeTessellationLevels(VPMat, _camera->viewport(), _adaptativeFactor);
    }
    _lodTime = std::chrono::high_resolution_clock::now() - lodTimeStart;


    /* Draw */
    if(_drawMode == DrawMode::FILL || _drawMode == DrawMode::FILL_AND_WIREFRAME){
      _f->glDepthFunc(GL_LESS);
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      currentPrg->setUniformValue(currentPrg->uniformLocation("wireframe"), false);
      currentPrg->setUniformValue(currentPrg->uniformLocation("v_color"), QVector3D(1,0,0));
      currentPrg->setUniformValue(currentPrg->uniformLocation("texturing_mode"), _texMode);

      _f->glBeginQuery(GL_PRIMITIVES_GENERATED, _primGenQuery);

      switch(_tessellationMethod) {
        case NO_TESSELLATION:
          _terrain.draw(*currentPrg);
          break;
        case HARDWARE:
          _terrain.drawHardwareTessellation(*currentPrg);
          break;
        case INSTANCIATION:
          _terrain.drawPatchInstanciation(*currentPrg);
          break;
      }

      _f->glEndQuery(GL_PRIMITIVES_GENERATED);
    }

    if(_drawMode == DrawMode::WIREFRAME || _drawMode == DrawMode::FILL_AND_WIREFRAME){
      _f->glDepthFunc(GL_LEQUAL);
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      _simplePrg->setUniformValue(currentPrg->uniformLocation("wireframe"), true);
      _simplePrg->setUniformValue(currentPrg->uniformLocation("v_color"), QVector3D(0,1,0));

      switch(_tessellationMethod) {
        case NO_TESSELLATION:
          _terrain.draw(*currentPrg);
          break;
        case HARDWARE:
          _terrain.drawHardwareTessellation(*currentPrg);
          break;
        case INSTANCIATION:
          _terrain.drawPatchInstanciation(*currentPrg);
          break;
      }
    }

    // End GPU Time count
    _f->glEndQuery(GL_TIME_ELAPSED);
    // End CPU Time count
    _cpuTime = std::chrono::high_resolution_clock::now() - cpuTimeStart;

    if (_showNormals) drawNormals();
    if (intersectionFound) drawIntersection();
  }

  void drawNormals() {
    // Show normals
    _normalPrg->bind();

    _f->glUniformMatrix4fv(_normalPrg->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_normalPrg->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());

    _f->glPatchParameteri(GL_PATCH_VERTICES, 3);

    _f->glUniform1f(_normalPrg->uniformLocation("TessLevelInner"), _constantInnerTessellationLevel);
    Vector3f outerLvl = Vector3f::Constant(_constantOuterTessellationLevel);
    _f->glUniform3fv(_normalPrg->uniformLocation("TessLevelOuter"), 1, outerLvl.data());
    _f->glUniform1f(_normalPrg->uniformLocation("triEdgeSize"), _terrain.getTriEdgeSize());
    _f->glUniform1f(_normalPrg->uniformLocation("heightScale"), _terrain.heightScale());
    _f->glUniform2fv(_normalPrg->uniformLocation("viewport"), 1, _camera->viewport().data());
    _f->glUniform1f(_normalPrg->uniformLocation("adaptativeFactor"), _adaptativeFactor);

    if (_tessellationMode == TessellationMode::CONSTANT)
      _f->glUniform1i(_normalPrg->uniformLocation("tessMethod"), TessellationMode::CONSTANT);
    else
      _f->glUniform1i(_normalPrg->uniformLocation("tessMethod"), _adaptativeTessellationMode+1);
    if (_tessellationMode == ADAPTATIVE_FROM_POV) {
      _f->glUniform3fv(_normalPrg->uniformLocation("TessDistRefPos"), 1, _camera->position().data());
    } else if (_tessellationMode == ADAPTATIVE_FROM_FIXED_POINT) {
      /* TODO : Implement Placement of point on scene */
      _f->glUniform3fv(_normalPrg->uniformLocation("TessDistRefPos"), 1, _camera->position().data());
    }

    _f->glUniform4fv(_normalPrg->uniformLocation("color"), 1, Vector4f(1.f, 1.f, 0.f, 1.f).data());

    _terrain.drawHardwareTessellation(*_normalPrg);
  }

  void drawIntersection() {
    _fillPrg->bind();

    _f->glUniformMatrix4fv(_fillPrg->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_fillPrg->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());
    _f->glUniformMatrix4fv(_fillPrg->uniformLocation("model"), 1, GL_FALSE, Eigen::Affine3f::Identity().data());

    _fillPrg->setUniformValue(_fillPrg->uniformLocation("color"), QVector4D(1,1,1,1));
    Line::draw(_f, _fillPrg, lineOrig, lineInterPoint); // Rayon

    _fillPrg->setUniformValue(_fillPrg->uniformLocation("color"), QVector4D(1,0.5,0.1,1));
    Point::draw(_f, _fillPrg, lineInterPoint);
  }

  void update(float dt) override {
    if (_camera->grabedToGround()) {
      Eigen::Vector3f camPos = _camera->position();
      float h = 0;
      if (_terrain.coordsInTerrain(camPos.x(), camPos.z()))
        h = _terrain.getHeight(camPos.x(), camPos.z());
      _camera->setUpOffset(h);
    }
    _camera->update(dt);
  }

  void clean() override {
    delete _simplePrg;
    delete _colormap;
    _terrain.clean();

    _f->glDeleteQueries(1, &_primGenQuery);
    _f->glDeleteQueries(1, &_gpuTimeQuery);
  }

  QGroupBox* createDisplayGroupBox(){
    QGroupBox * displayGroupBox = new QGroupBox("Display");
    QVBoxLayout * displayLayout = new QVBoxLayout;
    displayGroupBox->setLayout(displayLayout);

    //Draw mode selection
    ComboBoxOption * drawMode = new ComboBoxOption("Drawing mode : ");
    drawMode->addItem("Fill", DrawMode::FILL);
    drawMode->addItem("Wire-frame", DrawMode::WIREFRAME);
    drawMode->addItem("Fill + wire-frame", DrawMode::FILL_AND_WIREFRAME);
    drawMode->setCurrentIndex(_drawMode);
    QObject::connect(drawMode, static_cast<void (ComboBoxOption::*)(int)>(&ComboBoxOption::activated),
		     [this](int data) {
		       _drawMode = (DrawMode)data;
		     });
    displayLayout->addWidget(drawMode);

    //Separator
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    displayLayout->addWidget(line);

    //Texturing mode selection
    ComboBoxOption * textureMode = new ComboBoxOption("Texturing mode : ");
    textureMode->addItem("Constant color", TexturingMode::CONST_COLOR);
    textureMode->addItem("Texture", TexturingMode::TEXTURE);
    textureMode->addItem("Height map", TexturingMode::HEIGHTMAP);
    textureMode->addItem("Normal map", TexturingMode::NORMALS);
    textureMode->addItem("Texture coordinates", TexturingMode::TEXCOORDS);
    textureMode->addItem("Tessellation level", TexturingMode::TESSLEVEL);
    textureMode->setCurrentIndex(_texMode);
    QObject::connect(textureMode, static_cast<void (ComboBoxOption::*)(int)>(&ComboBoxOption::activated),
		     [this](int data) {
		       _texMode = (TexturingMode)data;
		     });
    displayLayout->addWidget(textureMode);

    return displayGroupBox;
  }

  QGroupBox* createLightingGroupBox(){
    QGroupBox * lightingGroupBox = new QGroupBox("Lighting");
    QVBoxLayout * lightingLayout = new QVBoxLayout;
    lightingGroupBox->setLayout(lightingLayout);

    VariableOption * lightAzimuth = new VariableOption("Light azimuth :", 20, 0, 360, 0.5);
    QObject::connect(lightAzimuth, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _light.setAzimuth(val*M_PI/180.);
		     });

    lightingLayout->addWidget(lightAzimuth);



    VariableOption * lightAltitude = new VariableOption("Light altitude :", 20, 0, 180, 0.5);
    QObject::connect(lightAltitude, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _light.setAltitude(val*M_PI/180.);
		     });

    lightingLayout->addWidget(lightAltitude);


    //Light color selection button
    QHBoxLayout * lightColor = new QHBoxLayout;
    lightingLayout->addLayout(lightColor);

    QLabel * lColorLabel = new QLabel("Light color :");
    lightColor->addWidget(lColorLabel);

    QPushButton * lColorPicker = new QPushButton;
    lightColor->addWidget(lColorPicker);

    Vector3f lc = _light.getColor();
    QString qss = QString("background-color: %1").arg(QColor(lc.x()*255., lc.y()*255., lc.z()*255.).name());
    lColorPicker->setStyleSheet(qss);

    QObject::connect(lColorPicker, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this, lColorPicker](){
		       Vector3f lc = _light.getColor();
		       QColor c = QColorDialog::getColor(QColor(lc.x()*255., lc.y()*255., lc.z()*255.), lColorPicker);
		       if(c.isValid()) {
			 QString qss = QString("background-color: %1").arg(c.name());
			 lColorPicker->setStyleSheet(qss);
			 _light.setColor(Vector3f(c.red()/255., c.green()/255., c.blue()/255.));
		       }
		     });

    //Separator
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    lightingLayout->addWidget(line);

    //Material label
    QLabel * materialLabel = new QLabel("Material :");
    lightingLayout->addWidget(materialLabel);

    //ambient coef
    VariableOption * ambientCoef = new VariableOption("Ka :", _ambientCoef, 0, 1, 0.01);
    QObject::connect(ambientCoef, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _ambientCoef = val;
		     });

    lightingLayout->addWidget(ambientCoef);

    //diffuse coef
    VariableOption * diffuseCoef = new VariableOption("Kd :", _diffuseCoef, 0, 1, 0.01);
    QObject::connect(diffuseCoef, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _diffuseCoef = val;
		     });

    lightingLayout->addWidget(diffuseCoef);

    //specular coef
    VariableOption * specularCoef = new VariableOption("Ks :", _specularCoef, 0, 1, 0.01);
    QObject::connect(specularCoef, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _specularCoef = val;
		     });

    lightingLayout->addWidget(specularCoef);

    //shininess coef
    VariableOption * shininessCoef = new VariableOption("shininess :", _shininessCoef, 0, 100, 0.01);
    QObject::connect(shininessCoef, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _shininessCoef = val;
		     });

    lightingLayout->addWidget(shininessCoef);

    //Separator
    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    lightingLayout->addWidget(line2);

    VariableOption * distFog = new VariableOption("Distance fog :", _distFog, 0, 5000, 1);
    QObject::connect(distFog, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _distFog = val;
		     });

    lightingLayout->addWidget(distFog);

    //Light color selection button
    QHBoxLayout * fogColor = new QHBoxLayout;
    lightingLayout->addLayout(fogColor);

    QLabel * fogColorLabel = new QLabel("Fog color :");
    fogColor->addWidget(fogColorLabel);

    QPushButton * fogColorPicker = new QPushButton;
    fogColor->addWidget(fogColorPicker);

    Vector3f fc = _light.getColor();
    QString qssf = QString("background-color: %1").arg(QColor(fc.x()*255., fc.y()*255., fc.z()*255.).name());
    fogColorPicker->setStyleSheet(qssf);

    QObject::connect(fogColorPicker, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this, fogColorPicker](){
		       Vector3f fc = _fogColor;
		       QColor c = QColorDialog::getColor(QColor(fc.x()*255., fc.y()*255., fc.z()*255.), fogColorPicker);
		       if(c.isValid()) {
			 QString qss = QString("background-color: %1").arg(c.name());
			 fogColorPicker->setStyleSheet(qss);
			 _fogColor = Vector3f(c.red()/255., c.green()/255., c.blue()/255.);
		       }
		     });


    return lightingGroupBox;
  }

  QGroupBox * createTessellationGroupBox(){
    QGroupBox * tessellationGroupBox = new QGroupBox("Tessellation");
    QVBoxLayout * tessellationLayout = new QVBoxLayout;
    tessellationGroupBox->setLayout(tessellationLayout);

    //Tessellation method selection
    ComboBoxOption * tessMethod = new ComboBoxOption("Tessellation method : ");
    tessMethod->addItem("No tessellation", TessellationMethod::NO_TESSELLATION);
    tessMethod->addItem("Hardware tessellation", TessellationMethod::HARDWARE);
    tessMethod->addItem("Patch instanciation", TessellationMethod::INSTANCIATION);
    tessMethod->setCurrentIndex(_tessellationMethod);
    QObject::connect(tessMethod, static_cast<void (ComboBoxOption::*)(int)>(&ComboBoxOption::activated),
		     [this](int data) {
		       _tessellationMethod = (TessellationMethod)data;
		     });

    tessellationLayout->addWidget(tessMethod);

    //Separator
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    tessellationLayout->addWidget(line);

    //Tessellation mode selection
    ComboBoxOption * tessMode = new ComboBoxOption("Tesselation mode : ");
    tessMode->addItem("Constant", TessellationMode::CONSTANT);
    tessMode->addItem("Adaptative from POV", TessellationMode::ADAPTATIVE_FROM_POV);
    tessMode->addItem("Adaptative from marker", TessellationMode::ADAPTATIVE_FROM_FIXED_POINT);
    tessMode->setCurrentIndex(_tessellationMode);
    QFrame * constantModeSubMenu = new QFrame();
    constantModeSubMenu->setVisible((_tessellationMode == TessellationMode::CONSTANT)? true : false);
    QFrame * adaptativeModeSubMenu = new QFrame();
    adaptativeModeSubMenu->setVisible((_tessellationMode == TessellationMode::CONSTANT)? false : true);

    QObject::connect(tessMode, static_cast<void (ComboBoxOption::*)(int)>(&ComboBoxOption::activated),
		     [this, constantModeSubMenu, adaptativeModeSubMenu](int data) {
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

    tessellationLayout->addWidget(tessMode);
    tessellationLayout->addWidget(constantModeSubMenu);
    tessellationLayout->addWidget(adaptativeModeSubMenu);

    QVBoxLayout * constantModeLayout = new QVBoxLayout;
    constantModeLayout->setContentsMargins(0,0,0,0);
    constantModeSubMenu->setLayout(constantModeLayout);

    VariableOption * innerLvl = new VariableOption("Inner level", 1, 1, 64, 0.1);
    QObject::connect(innerLvl, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _constantInnerTessellationLevel = val;
		     });
    VariableOption * outerLvl = new VariableOption("Outer level", 1, 1, 64, 0.1);
    QObject::connect(outerLvl, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _constantOuterTessellationLevel = val;
		     });
    constantModeLayout->addWidget(innerLvl);
    constantModeLayout->addWidget(outerLvl);

    QVBoxLayout * adaptativeModeLayout = new QVBoxLayout;
    adaptativeModeLayout->setContentsMargins(0,0,0,0);
    adaptativeModeSubMenu->setLayout(adaptativeModeLayout);

    ComboBoxOption * adaptativeTessMode = new ComboBoxOption("LOD method");
    adaptativeTessMode->addItem("Distance", AdaptativeMode::DISTANCE);
    adaptativeTessMode->addItem("Viewspace", AdaptativeMode::VIEWSPACE);
    adaptativeTessMode->addItem("Content Aware", AdaptativeMode::CONTENT);
    adaptativeTessMode->setCurrentIndex(_adaptativeTessellationMode);
    QObject::connect(adaptativeTessMode, static_cast<void (ComboBoxOption::*)(int)>(&ComboBoxOption::activated),
		     [this](int data) {
		       _adaptativeTessellationMode = (AdaptativeMode)data;
		     });

    VariableOption * adaptativeFactor = new VariableOption("Adaptative LOD factor", 1, 0, 1, 0.01);
    QObject::connect(adaptativeFactor, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _adaptativeFactor = val;
		     });
    adaptativeModeLayout->addWidget(adaptativeTessMode);
    adaptativeModeLayout->addWidget(adaptativeFactor);

    //Separator
    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    tessellationLayout->addWidget(line2);

    VariableOption * heightScaleFactor = new VariableOption("Height scale factor :", _terrain.heightScale(), 0, 200, 1);
    tessellationLayout->addWidget(heightScaleFactor);
    QObject::connect(heightScaleFactor, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
     [this](double val){
       _terrain.setHeightScale(val);
     });
    return tessellationGroupBox;
  }

  QGroupBox* createCameraGroupBox(){
    QGroupBox *cameraGroupBox = new QGroupBox("Camera");
    QVBoxLayout *cameraLayout = new QVBoxLayout;
    cameraGroupBox->setLayout(cameraLayout);

    ComboBoxOption * cameraMode = new ComboBoxOption("Camera mode :");
    cameraMode->addItem("FreeFly", CameraMode::FREE_FLY);
    QObject::connect(cameraMode, static_cast<void (ComboBoxOption::*)(int)>(&ComboBoxOption::activated),
		     [this](int data) {
		       _cameraMode = (CameraMode)data;
		     });

    cameraLayout->addWidget(cameraMode);

    VariableOption * cameraSpeed = new VariableOption("Camera speed : ", 15, 1, 30, 1);
    QObject::connect(cameraSpeed, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _camera->setSpeed(_defaultCamSpeed * val / 15.f);
		     });
    cameraLayout->addWidget(cameraSpeed);

    return cameraGroupBox;
  }

  virtual QDockWidget *createDock() {
    QDockWidget *dock = new QDockWidget("Options");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    QScrollArea *scrollArea = new QScrollArea(dock);
    scrollArea->setWidgetResizable(true);
    QFrame *frame = new QFrame;
    //frame->setWidgetResizable(false);
    QVBoxLayout * VLayout = new QVBoxLayout;

    QPushButton * reloadShadersButton = new QPushButton;
    reloadShadersButton->setText("Reload shaders");
    VLayout->addWidget(reloadShadersButton);

    QObject::connect(reloadShadersButton, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this](){
		       _needShaderReloading = true;
		     });

    VLayout->setAlignment(Qt::AlignTop);
    VLayout->addWidget(createDisplayGroupBox());
    VLayout->addWidget(createCameraGroupBox());
    VLayout->addWidget(createTessellationGroupBox());
    VLayout->addWidget(createLightingGroupBox());
    frame->setLayout(VLayout);
    scrollArea->setWidget(frame);
    dock->setWidget(scrollArea);

    return dock;
  }

  virtual void connectToMainWindow(MainWindow *mw) {
    QObject::connect(mw, static_cast<void (MainWindow::*)(const QString &)>(&MainWindow::loadedHeightMap),
                     [this, mw](const QString &filename) {
                       try {
                         _terrain.setHeightMap(filename);
                         _defaultCamSpeed = _terrain.getSize().norm() / 6000.f;
                         _camera->setSpeed(_defaultCamSpeed);
                       } catch (const std::exception &e) {
                         QMessageBox::about(mw, "Error while loading heightmap",
                                            "Unable to load the image file located at " + filename);
                         return;
                       }
                    });
    QObject::connect(mw, static_cast<void (MainWindow::*)(const QImage &)>(&MainWindow::loadedTexture),
                     [this](const QImage &im) {
                       this->_terrain.setTexture(im);
                     });

    QObject::connect(mw, &MainWindow::toggledNormals,
                     [this](bool checked) {
                       _showNormals = checked;
                     });

    QObject::connect(mw, &MainWindow::toggledGrab,
                     [this](bool checked) {
                       _camera->gradToGround(checked);
                       if (!_camera->grabedToGround())
                         _camera->setUpOffset(0);
                     });

    _mainWindow = mw;

    _bench = new BenchmarkWindow(_mainWindow, _mainWindow);
  }

  virtual void resize(int width, int height) {
    _camera->setViewport(width, height);
    _f->glViewport( 0, 0, (GLint)width, (GLint)height );
  }

  virtual void mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() == Qt::LeftButton)
      _camera->processMouseMove(e->x(), e->y());
  }

  virtual void wheelEvent(QWheelEvent *e) {
  }

  virtual void mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
      if (e->modifiers() & Qt::ControlModifier) {
        // Throw a ray
        QPoint pos = e->pos();
        Eigen::Vector2i mousePos(pos.x(), pos.y());
        _camera->screenPosToRay(mousePos, lineOrig, lineDir);

        float t = 0;
        if (_terrain.intersect(lineOrig, lineDir, t)) {
          lineInterPoint = lineOrig + lineDir * t;
          intersectionFound = true;
        } else {
          intersectionFound = false;
        }
      } else {
        _camera->processMousePress(e->x(), e->y());
      }
    }
  }

  virtual void mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton)
      _camera->processMouseRelease();
  }

  virtual void keyPressEvent(QKeyEvent *e) {
    switch (e->key())
    {
      case Qt::Key_Up:
      case Qt::Key_Z:
        _camera->processKeyPress(FreeFlyCamera::KEY_FORWARD);
        break;
      case Qt::Key_Down:
      case Qt::Key_S:
        _camera->processKeyPress(FreeFlyCamera::KEY_BACKWARD);
        break;
      case Qt::Key_Right:
      case Qt::Key_D:
        _camera->processKeyPress(FreeFlyCamera::KEY_RIGHT);
        break;
      case Qt::Key_Left:
      case Qt::Key_Q:
        _camera->processKeyPress(FreeFlyCamera::KEY_LEFT);
        break;
      case Qt::Key_E:
        _camera->processKeyPress(FreeFlyCamera::KEY_UP);
        break;
      case Qt::Key_F:
        _camera->processKeyPress(FreeFlyCamera::KEY_DOWN);
        break;
      case Qt::Key_G:
        _mainWindow->_toggleGradAction->toggle();
        break;
      case Qt::Key_N:
        _mainWindow->_toggleNormalsAction->toggle();
        break;
      case Qt::Key_B:
        _mainWindow->_toggleBenchAction->toggle();
      default:break;
    }
  }

  virtual void keyReleaseEvent(QKeyEvent *e) {
    switch (e->key())
    {
      case Qt::Key_Up:
      case Qt::Key_Z:
        _camera->processKeyRelease(FreeFlyCamera::KEY_FORWARD);
        break;
      case Qt::Key_Down:
      case Qt::Key_S:
        _camera->processKeyRelease(FreeFlyCamera::KEY_BACKWARD);
        break;
      case Qt::Key_Right:
      case Qt::Key_D:
        _camera->processKeyRelease(FreeFlyCamera::KEY_RIGHT);
        break;
      case Qt::Key_Left:
      case Qt::Key_Q:
        _camera->processKeyRelease(FreeFlyCamera::KEY_LEFT);
        break;
      case Qt::Key_E:
        _camera->processKeyRelease(FreeFlyCamera::KEY_UP);
        break;
      case Qt::Key_F:
        _camera->processKeyRelease(FreeFlyCamera::KEY_DOWN);
        break;
      default:break;
    }
  }

  virtual void focusOutEvent(QFocusEvent *event) {
    _camera->stopMovement();
  }

private:
  QOpenGLShaderProgram *_simplePrg = nullptr;
  QOpenGLShaderProgram *_simpleTessPrg = nullptr;
  QOpenGLShaderProgram *_simpleInstTessPrg = nullptr;
  QOpenGLShaderProgram *_fillPrg = nullptr;
  QOpenGLShaderProgram *_normalPrg = nullptr;
  QOpenGLTexture *_colormap = nullptr;
  Terrain _terrain;
  DirectionalLight _light;
  Sphere _testSphere;

  std::shared_ptr<FreeFlyCamera> _camera;

  float _constantInnerTessellationLevel = 1.f;
  float _constantOuterTessellationLevel = 1.f;
  float _adaptativeFactor = 1.f;
  float _ambientCoef = 0.4f;
  float _diffuseCoef = 0.8f;
  float _specularCoef = 0.1f;
  float _shininessCoef = 2.f;

  float _distFog = 1400.f;
  Vector3f _fogColor = Vector3f(0.3058823529411765f,0.3058823529411765f,0.3058823529411765f);

  TexturingMode _texMode = TexturingMode::TEXTURE;
  DrawMode _drawMode = DrawMode::FILL;
  CameraMode _cameraMode = CameraMode::FREE_FLY;
  TessellationMethod _tessellationMethod = TessellationMethod::HARDWARE;
  TessellationMode _tessellationMode = TessellationMode::ADAPTATIVE_FROM_POV;
  AdaptativeMode _adaptativeTessellationMode = AdaptativeMode::CONTENT;

  float _defaultCamSpeed;
  bool _needShaderReloading;

  bool _showNormals = false;

  bool intersectionFound = false;
  Eigen::Vector3f lineOrig;
  Eigen::Vector3f lineDir;
  Eigen::Vector3f lineInterPoint;

  MainWindow *_mainWindow;

  /* Benchmarking */
  GLuint _primGenQuery, _gpuTimeQuery;
  struct Stats {
    Stats(unsigned int primitiveGenerated, double gpuTime, double cpuTime, double lodTime)
            : primitiveGenerated(primitiveGenerated), gpuTime(gpuTime), cpuTime(cpuTime), lodTime(lodTime) {}

    unsigned int primitiveGenerated;
    double gpuTime;
    double cpuTime;
    double lodTime;
  };
  unsigned int _primGens = 0;
  unsigned int _gpuTime = 0;
  std::chrono::duration<double, std::milli> _cpuTime;
  std::chrono::duration<double, std::milli> _lodTime;

  BenchmarkWindow *_bench;
};

#endif //TERRAINTINTIN_TERRAINSCENE_H
