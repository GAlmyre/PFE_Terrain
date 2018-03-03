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

    _displayedPatch = 0;
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

    // ##### fill the vertexPosition vector
    for(Surface_mesh::Vertex v : mesh.vertices()){
      Vector3f p = point[v];
      _vertexPosition.push_back(p.x());
      _vertexPosition.push_back(p.y());
      _vertexPosition.push_back(p.z());
    }
    
    // ##### fill the vertexParent1 vector
    for(Surface_mesh::Vertex v : mesh.vertices()){
      _vertexParent1.push_back(vertex_parent1[v].idx());
      std::cout << " parent : " << vertex_parent1[v].idx() << std::endl;
    }
    
    // ##### fill the vertexParent2 vector
    for(Surface_mesh::Vertex v : mesh.vertices()){
      _vertexParent2.push_back(vertex_parent2[v].idx());
    }
    
    _vertexArray.create();

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

    _shader->bind();

    
    
    _vertexArray.bind();
    _vertexIDBuffer[_displayedPatch]->bind();
	
    int vertexID_loc = _shader->attributeLocation("vtx_ID");
    if(vertexID_loc>=0) {
      //_shader->setAttributeBuffer(vertexID_loc, GL_UNSIGNED_INT, 0, 1, sizeof(unsigned int));
      _f->glVertexAttribIPointer(vertexID_loc, 1, GL_UNSIGNED_INT, 0, 0);
      _shader->enableAttributeArray(vertexID_loc);
    }

    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _f->glUniformMatrix4fv(_shader->uniformLocation("view"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_shader->uniformLocation("projection"), 1, GL_FALSE, _camera->projectionMatrix().data());

    _f->glUniform3fv(_shader->uniformLocation("vertex_position"), _vertexPosition.size(), _vertexPosition.data());

    //_f->glUniform1fv(_shader->uniformLocation("vertex_parent
    
    
    _f->glDrawArrays(GL_TRIANGLES, 0, _nbElements[_displayedPatch]);//_nbElements[_displayedPatch]);

    if(vertexID_loc)
      _shader->disableAttributeArray(vertexID_loc);
    _vertexIDBuffer[_displayedPatch]->release();
    _vertexArray.release();
    
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
    QObject::connect(displayedPatch, static_cast<void (VariableOption::*)(double)>(&VariableOption::valueChanged),
		     [this](double val){
		       _displayedPatch = (unsigned int)val - 1;

		       std::cout << "displayed patch : " << _displayedPatch << std::endl;
		     });
    VLayout->addWidget(displayedPatch);
    
    
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
  unsigned int _nbElements[NB_TESS_LEVELS];

  std::vector<unsigned int> test;
  
  std::vector<float> _vertexPosition;
  std::vector<unsigned int> _vertexParent1;
  std::vector<unsigned int> _vertexParent2;
  
  QOpenGLShaderProgram * _shader;

  QOpenGLVertexArrayObject _vertexArray;
  QOpenGLBuffer* _vertexIDBuffer[NB_TESS_LEVELS];
  
  bool _needShaderReloading;
};

#endif //TERRAINTINTIN_PATCHTESSTESTSCENE_H
