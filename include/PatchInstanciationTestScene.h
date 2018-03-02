#ifndef TERRAINTINTIN_PATCHINSTANCIATIONTEST_H
#define TERRAINTINTIN_PATCHINSTANCIATIONTEST_H

#include "Scene.h"

#include "VariableOption.h"
#include "ComboBoxOption.h"

using namespace Eigen;

class PatchInstanciationTestScene : public Scene {
 public:

  void initialize() override {

    loadShaders();

    _needShaderReloading = false;

    _camera->setPosition(Eigen::Vector3f(0, 1, 1));
    _camera->setDirection(-Eigen::Vector3f(0,1,1));
    _camera->setViewport(600, 400);
    
    _f->glEnable(GL_DEPTH_TEST);
    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    /*
      p0-----p1
       |   _/|
       | _/  |
       |/    |
      p2-----p3
     */
    //p0
    _vertices.push_back(Vector3f(0,1,0));
    //p1
    _vertices.push_back(Vector3f(1,1,0));
    //p2
    _vertices.push_back(Vector3f(0,0,0));
    //p3
    _vertices.push_back(Vector3f(1,0,0));

    _indices.push_back(0);
    _indices.push_back(2);
    _indices.push_back(1);

    _indices.push_back(1);
    _indices.push_back(2);
    _indices.push_back(3);

    _nbInstances = 10;
    
    _vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _vertexBuffer->create();
    _vertexBuffer->bind();
    _vertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    _vertexBuffer->allocate(&(_vertices[0]), sizeof(Vector3f)*_vertices.size());
    _vertexBuffer->release();
    
    _indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    _indexBuffer->create();
    _indexBuffer->bind();
    _indexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    _indexBuffer->allocate(&(_indices[0]), sizeof(unsigned int)*_indices.size());
    _indexBuffer->release();
    
    
    _transformationBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _transformationBuffer->create();
    _transformationBuffer->bind();
    _transformationBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _transformationBuffer->allocate(&(_transformMatrices[0]), sizeof(Affine3f) * _nbInstances);
    _transformationBuffer->release();

    _colorBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _colorBuffer->create();
    _colorBuffer->bind();
    _colorBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _colorBuffer->allocate(&(_colors[0]), sizeof(Vector3f) * _nbInstances);
    _colorBuffer->release();
    

    _vertexArray.create();
    _vertexBuffer->release();
    _indexBuffer->release();
    _transformationBuffer->release();
    _colorBuffer->release();

    randomColors();
    randomTransform();
  }

  void loadShaders(){
    if(_simpleInstanciationPrg)
      delete _simpleInstanciationPrg;
    //shader init
    _simpleInstanciationPrg = new QOpenGLShaderProgram();
    _simpleInstanciationPrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/testInstanciation.vert");
    _simpleInstanciationPrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/testInstanciation.frag");
    _simpleInstanciationPrg->link();

  }

  void render() override {
    if (_needShaderReloading) {
      loadShaders();
      _needShaderReloading = false;
    }

    _simpleInstanciationPrg->bind();

    _vertexArray.bind();
    _vertexBuffer->bind();
    _indexBuffer->bind();
	
    int vertex_loc = _simpleInstanciationPrg->attributeLocation("vtx_position");
    if(vertex_loc>=0) {
      _simpleInstanciationPrg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(Vector3f));
      _simpleInstanciationPrg->enableAttributeArray(vertex_loc);
    }

    
    _transformationBuffer->bind();
    int transformation_loc = _simpleInstanciationPrg->attributeLocation("vtx_transformation");
    if(transformation_loc>=0) {
      _simpleInstanciationPrg->setAttributeBuffer(transformation_loc, GL_FLOAT, 0, 4, sizeof(Affine3f));
      _simpleInstanciationPrg->enableAttributeArray(transformation_loc);
      _simpleInstanciationPrg->setAttributeBuffer(transformation_loc+1, GL_FLOAT, 4 * sizeof(float), 4, sizeof(Affine3f));
      _simpleInstanciationPrg->enableAttributeArray(transformation_loc+1);
      _simpleInstanciationPrg->setAttributeBuffer(transformation_loc+2, GL_FLOAT, 8 * sizeof(float), 4, sizeof(Affine3f));
      _simpleInstanciationPrg->enableAttributeArray(transformation_loc+2);
      _simpleInstanciationPrg->setAttributeBuffer(transformation_loc+3, GL_FLOAT, 12 * sizeof(float), 4, sizeof(Affine3f));
      _simpleInstanciationPrg->enableAttributeArray(transformation_loc+3);
    }
    
    
    _colorBuffer->bind();
    int color_loc = _simpleInstanciationPrg->attributeLocation("vtx_color");
    if(color_loc>=0) {
      _simpleInstanciationPrg->setAttributeBuffer(color_loc, GL_FLOAT, 0, 3, sizeof(Vector3f));
      _simpleInstanciationPrg->enableAttributeArray(color_loc);
    }

    _f->glVertexAttribDivisor(vertex_loc, 0);
    _f->glVertexAttribDivisor(transformation_loc, 1);
    _f->glVertexAttribDivisor(transformation_loc+1, 1);
    _f->glVertexAttribDivisor(transformation_loc+2, 1);
    _f->glVertexAttribDivisor(transformation_loc+3, 1);
    _f->glVertexAttribDivisor(color_loc, 1);
    
    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _f->glUniformMatrix4fv(_simpleInstanciationPrg->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_simpleInstanciationPrg->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());
    
    QMatrix4x4 model;
    model.setToIdentity();

    _simpleInstanciationPrg->setUniformValue(_simpleInstanciationPrg->uniformLocation("model"), model);
    _simpleInstanciationPrg->setUniformValue(_simpleInstanciationPrg->uniformLocation("useModelMat"), false);

    _simpleInstanciationPrg->setUniformValue(_simpleInstanciationPrg->uniformLocation("color"), QVector3D(0,1,0));

    _f->glDrawElementsInstanced(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0, _nbInstances);

    if(vertex_loc)
      _simpleInstanciationPrg->disableAttributeArray(vertex_loc);
    _indexBuffer->release();
    _vertexBuffer->release();
    _vertexArray.release();
    
    _simpleInstanciationPrg->release();
    
    
  }

  void update(float dt) override {
    _camera->update(dt);
  }
  
  void clean() override {
    delete _simpleInstanciationPrg;
  }

  void randomColors() {
    _colors.clear();
    for(int i=0; i<_nbInstances; ++i){
      Vector3f c = Vector3f::Random();
      c = (c.array()+1)/2.;
      _colors.push_back(c);
    }
    _colorBuffer->bind();
    _colorBuffer->write(0, _colors.data(), sizeof(Vector3f) * _colors.size());
  }

  void randomTransform() {
    _transformMatrices.clear();
    for(int i=0; i<_nbInstances; ++i){
      Vector3f rot = Vector3f::Random();
      rot *= M_PI;

      Vector3f trans = Vector3f::Random();
      trans *= 3;

      Vector3f scale = Vector3f::Random();
      scale = (scale.array() + 2.) * 2;

      Affine3f t = Affine3f(Translation3f(trans))
	* AngleAxisf(rot.x(), Vector3f(1,0,0))
	* AngleAxisf(rot.y(), Vector3f(0,1,0))
	* AngleAxisf(rot.z(), Vector3f(0,0,1));
      _transformMatrices.push_back(t);
    }
    _transformationBuffer->bind();
    _transformationBuffer->write(0, _transformMatrices.data(), sizeof(Affine3f) * _transformMatrices.size());
  }
  
  virtual QDockWidget *createDock() {
    QDockWidget *dock = new QDockWidget("Options");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
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

    QPushButton * randomizeColorsButton = new QPushButton;
    randomizeColorsButton->setText("Randomize colors");
    VLayout->addWidget(randomizeColorsButton);

    QObject::connect(randomizeColorsButton, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this](){
		       randomColors();
		     });

    QPushButton * randomizePositionsButton = new QPushButton;
    randomizePositionsButton->setText("Randomize positions");
    VLayout->addWidget(randomizePositionsButton);

    QObject::connect(randomizePositionsButton, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this](){
		       randomTransform();
		     });
    
    VLayout->setAlignment(Qt::AlignTop);
    frame->setLayout(VLayout);
    scrollArea->setWidget(frame);
    dock->setWidget(scrollArea);
    return dock;
  }
    
  virtual void connectToMainWindow(const MainWindow& mw){
    
  }

 private:
  unsigned int _nbInstances;

  Terrain _terrain;
  
  QOpenGLShaderProgram * _shader;

  std::vector<Vector3f> _vertices;
  std::vector<unsigned int> _indices;
  std::vector<Affine3f> _transformMatrices;
  std::vector<Vector3f> _colors;
  
  QOpenGLVertexArrayObject _vertexArray;
  QOpenGLBuffer* _vertexBuffer;
  QOpenGLBuffer* _indexBuffer;
  QOpenGLBuffer* _transformationBuffer;
  QOpenGLBuffer* _colorBuffer;
  
  bool _needShaderReloading;
};

#endif //TERRAINTINTIN_PATCHINSTANCIATIONTEST_H
