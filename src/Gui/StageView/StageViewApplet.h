
#include "GlApplet.h"

#ifdef STACATO_USE_MAGNUM_ENGINE

#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/VertexColorGL.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Math/Matrix4.h>

class TestGlApplet : public GlApplet {
public:

	//triangle example
	Magnum::GL::Buffer mouseBuffer;
	Magnum::GL::Mesh mouseMesh;
	Magnum::Shaders::VertexColorGL2D shader;

	//primitives example
	Magnum::GL::Mesh cubeMesh;
	Magnum::Shaders::PhongGL phongShader;
	Magnum::Matrix4 _transformation, _projection;
	Magnum::Color3 _color;

	TestGlApplet(glm::vec2 s);
	virtual void updateEvent();
};

#endif