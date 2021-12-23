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

#include "Teapot.h"

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

  struct vec3 : public glm::vec3
  {
  public:
    using glm::vec3::vec3;

    vec2 xy() const { return vec2(x, y); }
  };

  struct vec4 : public glm::vec4
  {
  public:
    using glm::vec4::vec4;

    vec2 xy() const { return vec2(x, y); }
  };
  
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
    Mesh() : _position(0, 0, 0), _rotation(0, 0, 0), _scale(1, 0, 0) { }

    void add(const vec3& v) { _vertices.push_back(v); }

    const vec3& operator[](size_t index) const { return _vertices[index]; }

    decltype(_vertices)::const_iterator begin() const { return _vertices.begin(); }
    decltype(_vertices)::const_iterator end() const { return _vertices.end(); }

    const vec3& position() const { return _position; }

    const vec3& rotation() const { return _rotation; }
    void setRotation(const vec3& rot) { _rotation = rot; }

    const vec3& scale() const { return _scale; }
    void setScale(const vec3 scale) { _scale = scale; }
  };

  class Quad : public Mesh
  {
  public:
    Quad() = default;
    Quad(vec3 v, float w, float h)
    {
      add(v);
      add(vec3(v.x + w, v.y,     v.z));
      add(vec3(v.x + w, v.y + h, v.z));
      add(vec3(v.x,     v.y + h, v.z));

    }
  };
}

namespace math
{
  class intersections
  {
  public:
    static bool is2dPointInsideTriangle(a3d::vec2 p, a3d::vec2 p0, a3d::vec2 p1, a3d::vec2 p2)
    {
      float s = (p0.x - p2.x) * (p.y - p2.y) - (p0.y - p2.y) * (p.x - p2.x);
      float t = (p1.x - p0.x) * (p.y - p0.y) - (p1.y - p0.y) * (p.x - p0.x);

      if ((s < 0) != (t < 0) && s != 0 && t != 0)
        return false;

      float d = (p2.x - p1.x) * (p.y - p1.y) - (p2.y - p1.y) * (p.x - p1.x);
      return d == 0 || (d < 0) == (s + t <= 0);
    }
  };

  struct barycentric_coords
  {
    std::array<float, 3> lambdas;
  };

  class triangles
  {
  public:
    static float edgeFunction(const a3d::vec2& a, const a3d::vec2& b, const a3d::vec2& c)
    {
      return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    }

    static barycentric_coords barycentricCoords(const a3d::vec2& p, const a3d::vec2& v0, const a3d::vec2& v1, const a3d::vec2& v2)
    {
      float area = edgeFunction(v0, v1, v2);
      float w0 = edgeFunction(v1, v2, p);
      float w1 = edgeFunction(v2, v0, p);
      float w2 = edgeFunction(v0, v1, p);

      if (w0 >= 0 && w1 >= 0 && w2 >= 0)
      {
        w0 /= area;
        w1 /= area;
        w2 /= area;
      }

      return { {{w0, w1, w2}} };
    }
  };
}


using namespace a3d;

Mesh cube;
Camera camera;

MainView::MainView(ViewManager* gvm) : gvm(gvm)
{
  mouse = { -1, -1 };

  cube = Quad(vec3(-2.0f, -2.0f, 2.0f), 4.0f, 4.0f);

  for (int i = 0; i < teapot_count; i += 3)
  {
 //   cube.add(vec3(teapot[i], teapot[i + 1], teapot[i + 2]));
  }


  /*cube.add(vec3(-1,  1,  1));
  cube.add(vec3( 1,  1,  1));
  cube.add(vec3( 1, -1,  1));
  cube.add(vec3(-1, -1,  1));
  cube.add(vec3(-1, -1, -1));
  cube.add(vec3(-1,  1, -1));
  cube.add(vec3( 1,  1, -1));
  cube.add(vec3( 1, -1, -1));*/

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

    vec2 rasterPoint = vec2(point.x * WIDTH/2.0f + WIDTH / 2.0f, - point.y * HEIGHT/2.0f + HEIGHT / 2.0f);

    float z = 1.0f;// (point.z + 1.0f) / 2.0f;

    //gvm->fillRect(rasterPoint.x, rasterPoint.y, 1, 1, { 255, 255, 255 });
  }

  {
    vec4 v1 = transformMatrix * vec4(cube[0], 1.0f);
    vec4 v2 = transformMatrix * vec4(cube[1], 1.0f);
    vec4 v3 = transformMatrix * vec4(cube[2], 1.0f);
    
    v1 /= v1.w;
    v2 /= v2.w;
    v3 /= v3.w;

    vec2 rp1 = vec2(v1.x * WIDTH / 2.0f + WIDTH / 2.0f, -v1.y * HEIGHT / 2.0f + HEIGHT / 2.0f);
    vec2 rp2 = vec2(v2.x * WIDTH / 2.0f + WIDTH / 2.0f, -v2.y * HEIGHT / 2.0f + HEIGHT / 2.0f);
    vec2 rp3 = vec2(v3.x * WIDTH / 2.0f + WIDTH / 2.0f, -v3.y * HEIGHT / 2.0f + HEIGHT / 2.0f);


    for (size_t x = 0; x < WIDTH; ++ x)
      for (size_t y = 0; y < HEIGHT; ++y)
      {
        if (math::intersections::is2dPointInsideTriangle(vec2(x, y), rp1, rp2, rp3))
        {
          auto bc = math::triangles::barycentricCoords(vec2(x, y), rp1, rp2, rp3);

          gvm->fillRect(x, y, 1, 1, { (u8)(bc.lambdas[0]*255), (u8)(bc.lambdas[1] * 255), (u8)(bc.lambdas[2] * 255) });

        }
      }
  }

  /*
  {
    vec4 v1 = transformMatrix * vec4(cube[0], 1.0f);
    vec4 v2 = transformMatrix * vec4(cube[2], 1.0f);
    vec4 v3 = transformMatrix * vec4(cube[3], 1.0f);

    v1 /= v1.w;
    v2 /= v2.w;
    v3 /= v3.w;

    vec2 rp1 = vec2(v1.x * WIDTH / 2.0f + WIDTH / 2.0f, -v1.y * HEIGHT / 2.0f + HEIGHT / 2.0f);
    vec2 rp2 = vec2(v2.x * WIDTH / 2.0f + WIDTH / 2.0f, -v2.y * HEIGHT / 2.0f + HEIGHT / 2.0f);
    vec2 rp3 = vec2(v3.x * WIDTH / 2.0f + WIDTH / 2.0f, -v3.y * HEIGHT / 2.0f + HEIGHT / 2.0f);


    for (size_t x = 0; x < WIDTH; ++x)
      for (size_t y = 0; y < HEIGHT; ++y)
      {
        if (math::intersections::is2dPointInsideTriangle(vec2(x, y), rp1, rp2, rp3))
          gvm->fillRect(x, y, 1, 1, { 255, 0, 0 });
      }
  }
  */


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