#ifndef TERRAINTINTIN_PATCHTESSTESTSCENE_H
#define TERRAINTINTIN_PATCHTESSTESTSCENE_H

#include "Scene.h"

#include "VariableOption.h"
#include "ComboBoxOption.h"

#define NB_TESS_LEVELS 1

using namespace Eigen;
using namespace surface_mesh;

class PatchTessTestScene : public Scene {
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
    
    generateTessellatedPatches();

    _displayedPatch = 0;
  }

  void generateTessellatedPatches(){

    surface_mesh::Surface_mesh mesh;
    
    Surface_mesh::Vertex_property<Vector3f> points = mesh.get_vertex_property<Vector3f>("v:point");
    for(int i=0; i<NB_TESS_LEVELS; ++i){
      if(i==0){
	Surace_mesh::Vertex v1, v2, v3;
	v1 = mesh.add_vertex(Vector3f(1, 0, 0));
	v2 = mesh.add_vertex(Vector3f(0, 1, 0));
	v3 = mesh.add_vertex(Vector3f(0, 0, 1));
	mesh.add_triangle(v1, v2, v3);
      }

      std::vector<Vector3f> vertices;
      std::vector<unsigned int> indices;
      Surface_mesh::Vertex_iterator vit;
  
      Vector3f pos;
      for(vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
	{
	  pos = points[*vit];
	  vertices.push_back(pos);
	}

      // face iterator
      Surface_mesh::Face_iterator fit, fend = mesh.faces_end();
      // vertex circulator
      Surface_mesh::Vertex_around_face_circulator fvit, fvend;
      Surface_mesh::Vertex v0, v1, v2;
      for (fit = mesh.faces_begin(); fit != fend; ++fit)
	{
	  fvit = fvend = mesh.vertices(*fit);
	  v0 = *fvit;
	  ++fvit;
	  v2 = *fvit;

	  do{
	    v1 = v2;
	    ++fvit;
	    v2 = *fvit;
	    indices.push_back(v0.idx());
	    indices.push_back(v1.idx());
	    indices.push_back(v2.idx());
	  } while (++fvit != fvend);
	}

      _vertexBuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
      _vertexBuffer[i]->create();
      _vertexBuffer[i]->bind();
      _vertexBuffer[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
      _vertexBuffer[i]->allocate(&(vertices[0]), sizeof(Vector3f)*vertices.size());
      _vertexBuffer[i]->release();
    
      _indexBuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
      _indexBuffer[i]->create();
      _indexBuffer[i]->bind();
      _indexBuffer[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
      _indexBuffer[i]->allocate(&(indices[0]), sizeof(unsigned int)*indices.size());
      _indexBuffer[i]->release();

      _vertexArray[i].create();
      _vertexBuffer[i]->release();
      _indexBuffer[i]->release();

      _nbElements[i] = indices.size();
    }
  }

  void loadShaders(){
    if(_shader)
      delete _shader;
    //shader init
    _shader = new QOpenGLShaderProgram();
    _shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/patchTessTest.vert");
    _shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/patchTessTest.frag");
    _shader->link();

  }

  void render() override {
    if (_needShaderReloading) {
      loadShaders();
      _needShaderReloading = false;
    }

    _shader->bind();

    
    
    _vertexArray[displayedPatch].bind();
    _vertexBuffer[displayedPatch]->bind();
    _indexBuffer[displayedPatch]->bind();
	
    int vertex_loc = _shader->attributeLocation("vtx_position");
    if(vertex_loc>=0) {
      _shader->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(Vector3f));
      _shader->enableAttributeArray(vertex_loc);
    }

    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _f->glUniformMatrix4fv(_shader->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_shader->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());
    
    
    _f->glDrawElements(GL_TRIANGLES, _nbElements[_displayedPatch], GL_UNSIGNED_INT, 0);

    if(vertex_loc)
      _shader->disableAttributeArray(vertex_loc);
    _indexBuffer[_displayedPatch]->release();
    _vertexBuffer[_displayedPatch]->release();
    _vertexArray[_displayedPatch].release();
    
    _shader->release();
    
    
  }

  void update(float dt) override {
    _camera->update(dt);
  }
  
  void clean() override {
    delete _shader;
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

    VariableOption* displayedPatch = new VariableOption("Displayed patch :", 1, 1, NB_TESS_LEVELS, 1);
    QObject::connect(lightAzimuth, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _displayedPatch = (unsigned int)val - 1;

		       std::cout << "displayed patch : " << _displayedPatch << std::endl;
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

  unsigned int _displayedPatch;
  
  QOpenGLShaderProgram * _shader;

  unsigned int _nbElements[NB_TESS_LEVELS];
  QOpenGLVertexArrayObject _vertexArray[NB_TESS_LEVELS];
  QOpenGLBuffer* _vertexBuffer[NB_TESS_LEVELS];
  QOpenGLBuffer* _indexBuffer[NB_TESS_LEVELS];
  
  bool _needShaderReloading;
};

#endif //TERRAINTINTIN_PATCHTESSTESTSCENE_H
