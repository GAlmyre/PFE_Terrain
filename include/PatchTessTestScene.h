#ifndef TERRAINTINTIN_PATCHTESSTESTSCENE_H
#define TERRAINTINTIN_PATCHTESSTESTSCENE_H

#include "Scene.h"

#include "VariableOption.h"
#include "ComboBoxOption.h"

#define NB_TESS_LEVELS 7

using namespace Eigen;
using namespace surface_mesh;

class PatchTessTestScene : public Scene {
 public:
  PatchTessTestScene() : Scene(), _camera(new FreeFlyCamera) {}
  void initialize() override {

    loadShaders();

    _needShaderReloading = false;

    _camera->setPosition(Eigen::Vector3f(1.5, 1.5, 1.5));
    _camera->setDirection(-Eigen::Vector3f(1,1,1));
    _camera->setViewport(600, 400);

    _f->glEnable(GL_DEPTH_TEST);
    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    generateTessellatedPatches();

    _f->glGenBuffers(1, &_vertexPositionSSBO);
    _f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _vertexPositionSSBO);
    _f->glBufferData(GL_SHADER_STORAGE_BUFFER, _vertexPosition.size()*sizeof(float), _vertexPosition.data(), GL_STATIC_COPY);
    _f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _vertexPositionSSBO);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    _f->glGenBuffers(1, &_vertexParentsSSBO);
    _f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _vertexParentsSSBO);
    _f->glBufferData(GL_SHADER_STORAGE_BUFFER, _vertexParents.size()*sizeof(float), _vertexParents.data(), GL_STATIC_COPY);
    _f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _vertexParentsSSBO);

    _displayedPatch = 0;

    _nbPatchs = 2;
    //creation of two patches
    /*
      p0-----p1
      |   _/|
      | _/  |
      |/    |
      p2-----p3
      p0(0,1)
      p1(1,1)
      p2(0,0)
      p3(1,0)
    */
    //p0
    _patchTransform.push_back(0);
    _patchTransform.push_back(1);
    //p2
    _patchTransform.push_back(0);
    _patchTransform.push_back(0);
    //p1
    _patchTransform.push_back(1);
    _patchTransform.push_back(1);

    //p1
    _patchTransform.push_back(1);
    _patchTransform.push_back(1);
    //p2
    _patchTransform.push_back(0);
    _patchTransform.push_back(0);
    //p3
    _patchTransform.push_back(1);
    _patchTransform.push_back(0);



    _patchTessLevel.push_back(2);
    _patchTessLevel.push_back(2);


    _f->glGenBuffers(1, &_patchTransformSSBO);
    _f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _patchTransformSSBO);
    _f->glBufferData(GL_SHADER_STORAGE_BUFFER, _patchTransform.size()*sizeof(float), _patchTransform.data(), GL_STATIC_COPY);
    _f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _patchTransformSSBO);

    _f->glGenBuffers(1, &_patchTessLevelsSSBO);

    //we allocate each patchIDBuffer with the max number of patchs
    for(int i=0; i<NB_TESS_LEVELS; ++i){
      _patchIDBuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
      _patchIDBuffer[i]->create();
      _patchIDBuffer[i]->bind();
      _patchIDBuffer[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
      _patchIDBuffer[i]->allocate(sizeof(unsigned int)*_nbPatchs);
      _patchIDBuffer[i]->release();
    }

    QImage heightMap("../data/heightmaps/gauss1_64x64.png");
    _heightMap = new QOpenGLTexture(heightMap.mirrored());
    _heightMap->setMinificationFilter(QOpenGLTexture::Linear);
    _heightMap->setMagnificationFilter(QOpenGLTexture::Linear);

    _vertexArray.create();

  }

  void generateTessellatedPatches(){

    Surface_mesh mesh;
    //mesh.reserve(1000,1000,1000);


    // ##### Properties #####
    Surface_mesh::Vertex_property<Vector3f> point = mesh.get_vertex_property<Vector3f>("v:point");
    //tessellation level
    Surface_mesh::Vertex_property<unsigned int> vertexTessLevel = mesh.vertex_property<unsigned int>("v:tessLevel");
    Surface_mesh::Edge_property<unsigned int> edgeTessLevel = mesh.edge_property<unsigned int>("e:tessLevel");
    Surface_mesh::Face_property<unsigned int> faceTessLevel = mesh.face_property<unsigned int>("f:tessLevel");
    //vertex child
    Surface_mesh::Vertex_property<Surface_mesh::Vertex> vertex_child = mesh.vertex_property<Surface_mesh::Vertex>("v:child");
    //edge child
    Surface_mesh::Edge_property<Surface_mesh::Vertex> edge_child = mesh.edge_property<Surface_mesh::Vertex>("e:child");
    //vertex parent1
    Surface_mesh::Vertex_property<Surface_mesh::Vertex> vertex_parent1 = mesh.vertex_property<Surface_mesh::Vertex>("v:parent1");
    //vertex parent2
    Surface_mesh::Vertex_property<Surface_mesh::Vertex> vertex_parent2 = mesh.vertex_property<Surface_mesh::Vertex>("v:parent2");

    // ##### Tessellation levels creation loop #####

    for(int i=0; i<NB_TESS_LEVELS; ++i){
      //creation of the first level
      if(i==0){
	Surface_mesh::Vertex v1, v2, v3;
	v1 = mesh.add_vertex(Vector3f(1, 0, 0));
	v2 = mesh.add_vertex(Vector3f(0, 1, 0));
	v3 = mesh.add_vertex(Vector3f(0, 0, 1));
	Surface_mesh::Face f = mesh.add_triangle(v1, v2, v3);

	vertexTessLevel[v1] = vertexTessLevel[v2] = vertexTessLevel[v3] = faceTessLevel[f] = i;
	for(Surface_mesh::Halfedge h : mesh.halfedges(f)){
	  Surface_mesh::Edge e = mesh.edge(h);
	  edgeTessLevel[e] = i;
	}
      }
      //creation of every other levels
      else{
	//we make a copy of each vertex of the previous level and mark it as its child
        for(Surface_mesh::Vertex v : mesh.vertices()){
	  if(vertexTessLevel[v] == i-1){
	    Surface_mesh::Vertex vChild = mesh.add_vertex(point[v]);
	    vertex_child[v] = vChild;

	    vertexTessLevel[vChild] = i;
	  }
	}

	//we create a vertex at the middle of each edge of the previous level and mark it as its child
	for(Surface_mesh::Edge e : mesh.edges()){
	  if(edgeTessLevel[e] == i-1){
	    Surface_mesh::Vertex ev0, ev1;
	    ev0 = mesh.vertex(e, 0);
	    ev1 = mesh.vertex(e, 1);
	    Surface_mesh::Vertex v = mesh.add_vertex((point[ev0] + point[ev1])/2);
	    edge_child[e] = v;
	    vertex_parent1[v] = ev0;
	    vertex_parent2[v] = ev1;

	    vertexTessLevel[v] = i;
	  }
	}

	//we create 4 triangles for each
	for(Surface_mesh::Face f : mesh.faces()){
	  if(faceTessLevel[f] == i-1){
	    Surface_mesh::Vertex v0, v1, v2, ev0, ev1, ev2;
	    Surface_mesh::Halfedge h = mesh.halfedge(f);
	    v0 = vertex_child[mesh.to_vertex(h)];
	    ev0 = edge_child[mesh.edge(h)];
	    h = mesh.next_halfedge(h);
	    v1 = vertex_child[mesh.to_vertex(h)];
	    ev1 = edge_child[mesh.edge(h)];
	    h = mesh.next_halfedge(h);
	    v2 = vertex_child[mesh.to_vertex(h)];
	    ev2 = edge_child[mesh.edge(h)];

	    Surface_mesh::Face f0, f1, f2, f3;
	    f0 = mesh.add_triangle(ev0, v0, ev1);
	    f1 = mesh.add_triangle(ev1, v1, ev2);
	    f2 = mesh.add_triangle(ev2, v2, ev0);
	    f3 = mesh.add_triangle(ev0, ev1, ev2);

	    faceTessLevel[f0] = i;
	    faceTessLevel[f1] = i;
	    faceTessLevel[f2] = i;
	    faceTessLevel[f3] = i;

	    for(Surface_mesh::Halfedge h : mesh.halfedges(f0)){
	      Surface_mesh::Edge e = mesh.edge(h);
	      edgeTessLevel[e] = i;
	    }

	    for(Surface_mesh::Halfedge h : mesh.halfedges(f1)){
	      Surface_mesh::Edge e = mesh.edge(h);
	      edgeTessLevel[e] = i;
	    }

	    for(Surface_mesh::Halfedge h : mesh.halfedges(f2)){
	      Surface_mesh::Edge e = mesh.edge(h);
	      edgeTessLevel[e] = i;
	    }

	    for(Surface_mesh::Halfedge h : mesh.halfedges(f3)){
	      Surface_mesh::Edge e = mesh.edge(h);
	      edgeTessLevel[e] = i;
	    }
	  }
	}
      }

      std::cout << "vertices : " << mesh.n_vertices()
		<< " edged : " << mesh.n_edges()
		<< " halfedges : " << mesh.n_halfedges()
		<< " faces : " << mesh.n_faces()
		<< std::endl;
    }

    std::cout << "creation loop over" << std::endl;

    // ##### fill the tess level vertexIDBuffer with vertices indices
    std::vector<unsigned int> vertexID[NB_TESS_LEVELS];


    for(Surface_mesh::Face f : mesh.faces()){
      unsigned int level = faceTessLevel[f];
      for(Surface_mesh::Vertex v : mesh.vertices(f)){
	vertexID[level].push_back(v.idx());
      }
    }

    for(int i=0; i<NB_TESS_LEVELS; ++i){
      _nbElements[i] = vertexID[i].size();

      _vertexIDBuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
      _vertexIDBuffer[i]->create();
      _vertexIDBuffer[i]->bind();
      _vertexIDBuffer[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
      _vertexIDBuffer[i]->allocate(vertexID[i].data(), sizeof(unsigned int)*vertexID[i].size());
      _vertexIDBuffer[i]->release();
    }

    // ##### fill the vertexPosition and vertexParents vectors
    for(Surface_mesh::Vertex v : mesh.vertices()){
      Vector3f p = point[v];
      _vertexPosition.push_back(p.x());
      _vertexPosition.push_back(p.y());
      _vertexPosition.push_back(p.z());
      //TODO 4th value is border value
      _vertexPosition.push_back(0);

      //for parents  0 = no parent (idx = -1)   all idx are shifted by one to fit in an unsigned int
      /* _vertexParents.push_back(0); */
      /* _vertexParents.push_back(0); */
      _vertexParents.push_back(vertex_parent1[v].idx()+1);
      _vertexParents.push_back(vertex_parent2[v].idx()+1);
    }

    std::cout << "vertices : " << mesh.n_vertices()
	      << " edged : " << mesh.n_edges()
	      << " halfedges : " << mesh.n_halfedges()
	      << " faces : " << mesh.n_faces()
	      << std::endl;

    std::cout << "vertexPosition size : " << _vertexPosition.size() << std::endl;


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

    if(_culling)
      _f->glEnable(GL_CULL_FACE);
    else
      _f->glDisable(GL_CULL_FACE);
    if(_fill)
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
      _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //std::cout << "tess levels : " << _patchTessLevel[0] << " " <<  _patchTessLevel[1] << std::endl;

    //we fill the patchID buffers
    std::vector<unsigned int> patchID[NB_TESS_LEVELS];
    float levels[7] = {1, 2, 4, 8, 16, 32, 64};
    for(unsigned int i=0; i<_nbPatchs; ++i){
      float level = _patchTessLevel[i];
      for(unsigned int j=0; j<7; ++j){
	if(level <= levels[j]){
	  patchID[j].push_back(i);
	  break;
	}
      }
    }

    //we copy patch tessellation levels to the ssbo
    _f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _patchTessLevelsSSBO);
    _f->glBufferData(GL_SHADER_STORAGE_BUFFER, _patchTessLevel.size()*sizeof(float), _patchTessLevel.data(), GL_DYNAMIC_COPY);
    _f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _patchTessLevelsSSBO);

    _shader->bind();

    _heightMap->bind(0);
    _shader->setUniformValue(_shader->uniformLocation("heightmap"), 0);

    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    for(int patchIt = 0; patchIt < NB_TESS_LEVELS; ++patchIt){

      _vertexArray.bind();
      _vertexIDBuffer[patchIt]->bind();

      int vertexID_loc = _shader->attributeLocation("vtx_ID");
      if(vertexID_loc>=0) {
	//_shader->setAttributeBuffer(vertexID_loc, GL_UNSIGNED_INT, 0, 1, sizeof(unsigned int));
	_f->glVertexAttribIPointer(vertexID_loc, 1, GL_UNSIGNED_INT, 0, 0);
	_shader->enableAttributeArray(vertexID_loc);
      }

      _patchIDBuffer[patchIt]->bind();

      _patchIDBuffer[patchIt]->write(0, patchID[patchIt].data(), sizeof(unsigned int) * patchID[patchIt].size());

      int patchID_loc = _shader->attributeLocation("patch_ID");
      if(patchID_loc>=0) {
	//_shader->setAttributeBuffer(vertexID_loc, GL_UNSIGNED_INT, 0, 1, sizeof(unsigned int));
	_f->glVertexAttribIPointer(patchID_loc, 1, GL_UNSIGNED_INT, 0, 0);
	_shader->enableAttributeArray(patchID_loc);
      }

      _f->glVertexAttribDivisor(vertexID_loc, 0);
      _f->glVertexAttribDivisor(patchID_loc, 1);


      _f->glUniformMatrix4fv(_shader->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
      _f->glUniformMatrix4fv(_shader->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());

      _f->glUniform1ui(_shader->uniformLocation("patchLevel"), patchIt);


      _f->glDrawArraysInstanced(GL_TRIANGLES, 0, _nbElements[patchIt], patchID[patchIt].size());

      if(vertexID_loc)
	_shader->disableAttributeArray(vertexID_loc);
      _vertexIDBuffer[patchIt]->release();
      _vertexArray.release();
    }
    _heightMap->release();
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

    QPushButton * toggleWireframeButton = new QPushButton;
    toggleWireframeButton->setText("Toggle wireframe / fill");
    VLayout->addWidget(toggleWireframeButton);

    QObject::connect(toggleWireframeButton, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this](){
		       _fill = !_fill;
		     });

    QPushButton * toggleCullingButton = new QPushButton;
    toggleCullingButton->setText("Toggle culling");
    VLayout->addWidget(toggleCullingButton);

    QObject::connect(toggleCullingButton, static_cast<void (QPushButton::*)()>(&QPushButton::pressed),
		     [this](){
		       _culling = !_culling;
		     });

    VariableOption* firstPatchLevel = new VariableOption("First patch level :", 1, 1, 64, 0.1);
    QObject::connect(firstPatchLevel, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _patchTessLevel[0] = val;

		     });
    VLayout->addWidget(firstPatchLevel);

    VariableOption* secondPatchLevel = new VariableOption("Second patch level :", 1, 1, 64, 0.1);
    QObject::connect(secondPatchLevel, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _patchTessLevel[1] = val;

		     });
    VLayout->addWidget(secondPatchLevel);


    VLayout->setAlignment(Qt::AlignTop);
    frame->setLayout(VLayout);
    scrollArea->setWidget(frame);
    dock->setWidget(scrollArea);
    return dock;
  }

  virtual void connectToMainWindow(const MainWindow& mw){

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
    if (e->button() == Qt::LeftButton)
      _camera->processMousePress(e->x(), e->y());
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

  bool _fill = true;
  bool _culling = true;

  unsigned int _displayedPatch;
  unsigned int _nbElements[NB_TESS_LEVELS];
  unsigned int _nbPatchs;

  std::vector<float> _patchTessLevel;


  std::vector<unsigned int> test;

  std::vector<float> _vertexPosition;
  std::vector<unsigned int> _vertexParents;

  std::vector<float> _patchTransform;

  GLuint _vertexPositionSSBO;
  GLuint _vertexParentsSSBO;
  GLuint _patchTransformSSBO;

  GLuint _patchTessLevelsSSBO;

  QOpenGLShaderProgram * _shader = nullptr;

  QOpenGLVertexArrayObject _vertexArray;
  QOpenGLBuffer* _vertexIDBuffer[NB_TESS_LEVELS];

  QOpenGLBuffer* _patchIDBuffer[NB_TESS_LEVELS];

  QOpenGLTexture *_heightMap = nullptr;

  std::shared_ptr<FreeFlyCamera> _camera;

  bool _needShaderReloading;
};

#endif //TERRAINTINTIN_PATCHTESSTESTSCENE_H
