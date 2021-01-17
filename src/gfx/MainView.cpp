#pragma once

#include "MainView.h"
#include "ViewManager.h"

#include <vector>


#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"


using namespace ui;

namespace math
{
  using real_t = float;
  
  class vecz
  {
  public:
    real_t x, y, z;

    vecz() : vecz(0, 0, 0) { }
    vecz(real_t x, real_t y, real_t z) : x(x), y(y), z(z) { }

  public:
    vecz operator+(const vecz& o) const { return vecz(x + o.x, y + o.y, z + o.z); }

    inline real_t length() const { return std::sqrt(squaredLength()); }
    inline real_t squaredLength() const { return x * x + y * y + z * z; }
  };

  class mat4
  {
  public:
    float v[16];

  public:

  };
}

namespace a3d
{
  using vec2 = glm::vec2;
  using vec3 = glm::vec3;
  using vec4 = glm::vec4;
  using mat4 = glm::mat4;

  class Camera
  {
  private:
    mutable mat4 _transform;

    vec2 _angle;
    vec3 _position;
    vec3 _target;

  public:

    void setPosition(const vec3& position) { _position = position; }
    void setTarget(const vec3& target) { _target = target; }
    
    const vec3& position() const { return _position; }
    const vec3& target() const { return _target; }

    const vec2 angle() const { return _angle; }
    void rotate(const vec2& angle) { _angle = angle; }

    glm::vec3 directionUp() const { return glm::vec3(glm::inverse(_transform)*glm::vec4(0, 1, 0, 0)); }
    glm::vec3 directionRight() const { return glm::vec3(glm::inverse(_transform)*glm::vec4(1, 0, 0, 0)); }
    glm::vec3 directionForward() const { return glm::vec3(glm::inverse(_transform)*glm::vec4(0, 0, -1, 0)); }

    const mat4& transform() const
    {
      glm::mat4 _transform = glm::mat4(1.0f);
      _transform = glm::rotate(_transform, angle().x, glm::vec3(0, 1, 0));
      _transform = glm::rotate(_transform, angle().y, glm::vec3(1, 0, 0));
      _transform = glm::translate(_transform, -position());
      return _transform;
    }
  };

  class Mesh
  {
    std::vector<vec3> _vertices;
    vec3 _position;
    vec3 _rotation;
    vec3 _scale;

  public:
    void add(vec3&& v) { _vertices.push_back(v); }

    decltype(_vertices)::const_iterator begin() const { return _vertices.begin(); }
    decltype(_vertices)::const_iterator end() const { return _vertices.end(); }

    const vec3& position() const { return _position; }

    const vec3& rotation() const { return _rotation; }
    void setRotation(const vec3& rot) { _rotation = rot; }

    const vec3& scale() const { return _scale; }
    void setScale(const vec3 scale) { _scale = scale; }

  };
}

using namespace a3d;

Mesh cube;
Camera camera;

MainView::MainView(ViewManager* gvm) : gvm(gvm)
{
  mouse = { -1, -1 };

  cube.add(vec3(-1,  1,  1));
  cube.add(vec3( 1,  1,  1));
  cube.add(vec3( 1, -1,  1));
  cube.add(vec3(-1, -1,  1));
  cube.add(vec3(-1, -1, -1));
  cube.add(vec3(-1,  1, -1));
  cube.add(vec3( 1,  1, -1));
  cube.add(vec3( 1, -1, -1));

  cube.setScale(vec3(1.0f));

  camera.setPosition(vec3(0, 0, -5.0f));
  camera.setTarget(vec3());

  std::fill(keymap, keymap + 256, false);
}

void MainView::render()
{
  auto r = gvm->renderer();

  gvm->clear({ 0, 0, 0 });

  glm::mat4 viewMatrix = camera.transform();

  glm::mat4 projectionMatrix = glm::perspective(glm::radians(80.0f), float(WIDTH) / float(HEIGHT), 0.01f, 1.0f);

  glm::mat4 modelMatrix = mat4(1.0f);
  modelMatrix = glm::scale(modelMatrix, cube.scale());
  modelMatrix = glm::rotate(modelMatrix, cube.rotation().x, glm::vec3(1.0f, 0.0f, 0.0f));
  modelMatrix = glm::rotate(modelMatrix, cube.rotation().y, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrix = glm::translate(modelMatrix, cube.position());

  glm::mat4 transformMatrix = projectionMatrix * viewMatrix * modelMatrix;

  for (const auto& vertex : cube)
  {
    vec4 point = transformMatrix * vec4(vertex, 1.0f);
    point /= point.w;
    
    if (&vertex == &(*cube.begin()))
    {
      printf("%2.2f x %2.2f y %2.2f z %2.2f\n", camera.position().z, point.x, point.y, point.z);
    }


    vec2 rasterPoint = vec2(point.x * WIDTH/2.0f + WIDTH / 2.0f, - point.y * HEIGHT/2.0f + HEIGHT / 2.0f);

    float z = 1.0f;// (point.z + 1.0f) / 2.0f;

    gvm->fillRect(rasterPoint.x, rasterPoint.y, 2, 2, { 255, 255, 255 });
  }


  if (keymap[SDL_SCANCODE_DOWN])
    camera.setPosition(camera.position() + vec3(0, 0, 1)*0.05f);
  else if (keymap[SDL_SCANCODE_UP])
    camera.setPosition(camera.position() + vec3(0, 0, 1)*-0.05f);


  cube.setRotation(cube.rotation() + vec3(0.01f, 0.01f, 0.0f));
}

void MainView::handleKeyboardEvent(const SDL_Event& event)
{
  keymap[event.key.keysym.scancode] = event.type == SDL_KEYDOWN;

  
  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE: gvm->exit(); break;
    }
  }
}

void MainView::handleMouseEvent(const SDL_Event& event)
{
  if (event.type == SDL_MOUSEMOTION)
  {
    mouse.x = event.motion.x;
    mouse.y = event.motion.y;
  }
}