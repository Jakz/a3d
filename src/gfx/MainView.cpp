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
    vec3 xyz() const { return vec3(x, y, z); }
  };
  
  struct mat4 : public glm::mat4
  {
  public:
    using glm::mat4::mat4;

    mat4 inverse() { return glm::inverse(*this); }
  };

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

    vec3 directionUp() const { return vec3(_transform.inverse() * vec4(0, 1, 0, 0)); }
    vec3 directionRight() const { return vec3(_transform.inverse() * vec4(1, 0, 0, 0)); }
    vec3 directionForward() const { return vec3(_transform.inverse() * vec4(0, 0, -1, 0)); }

    mat4 transform() const
    {      
      //return glm::lookAt(_position, _target, vec3(0.0, -1.0, 0.0));
      
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
    Mesh() : _position(0, 0, 0), _rotation(0, 0, 0), _scale(1, 1, 1) { }

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

namespace a3d
{
  class Texture
  {
  private:
    size_t _width;
    size_t _height;
    std::vector<color_t> _data;
    
  public:
    Texture(size_t width, size_t height) : _width(width), _height(height), _data(width* height)
    {
      for (size_t y = 0; y < _height; ++y)
      {
        for (size_t x = 0; x < _width; ++x)
        {
          auto cy = y / 8, cx = x / 8;

          bool dark = (cx % 2 == 1 && cy % 2 == 0) || (cx % 2 == 0 && cy % 2 == 1);

          get(x, y) = dark ? color_t{ 120, 120, 120, 255 } : color_t{220, 220, 220, 255};
        }
      }
    }

    color_t& get(int32_t x, int32_t y)
    {
      return x >= 0 && x < _width && y >= 0 && y < _height ? _data[y * _width + x] : _data[0];
    }

    color_t& get(const vec2& coords)
    {
      int32_t x = coords.x * _width;
      int32_t y = coords.y * _height;
      return get(x, y);
    }

    size_t width() const { return _width; }
    size_t height() const { return _height; }
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

namespace a3d
{
  namespace rasterize
  {
    class Triangle
    {
    public:
      std::array<vec3, 3> vertices;

      const vec3& operator[](size_t i) const { return vertices[i]; }
    };

    class Rasterizer
    {
    public:


    private:
      mat4 _projectionMatrix;

    public:
      Rasterizer() : _projectionMatrix(1.0f) { }

      /* this assumes vertices have already been transformed into camera coordinates */
      Triangle projectRectangle(const std::array<vec3, 3>& vertices)
      {
        Triangle triangle;
        
        for (size_t i = 0; i < vertices.size(); ++i)
        {
          vec4 v = _projectionMatrix * vec4(vertices[i], 1.0f);
          v /= v.w;

          triangle.vertices[i] = vec3(v.x * WIDTH / 2.0f + WIDTH / 2.0f, v.y * HEIGHT / 2.0f + HEIGHT / 2.0f, v.z);
        }

        return triangle;
      }

      template<typename T>
      T computeVertexAttribute(const Triangle& triangle, const std::array<T, 3>& attribute, const vec2& fragment)
      {
        auto bc = math::triangles::barycentricCoords(fragment, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);
        return attribute[0] * bc.lambdas[0] + attribute[1] * bc.lambdas[1] + attribute[2] * bc.lambdas[2];
      }

      template<typename T>
      T computeCorrectedVertexAttribute(const Triangle& triangle, std::array<T, 3> attribute, const vec2& fragment)
      {
        auto bc = math::triangles::barycentricCoords(fragment, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);

        /* normalize attribute by z*/
        for (size_t i = 0; i < attribute.size(); ++i)
          attribute[i] /= triangle[i].z;

        float z = 1 / (bc.lambdas[0] * (1 / triangle[0].z) + bc.lambdas[1] * (1 / triangle[1].z) + bc.lambdas[2] * (1 / triangle[2].z));

        return z * attribute[0] * bc.lambdas[0] + z * attribute[1] * bc.lambdas[1] + z * attribute[2] * bc.lambdas[2];
      }

    };
  }
  
  

}

using namespace a3d;

Mesh cube;
Camera camera;

Texture texture = Texture(128, 128);

rasterize::Rasterizer rasterizer;

MainView::MainView(ViewManager* gvm) : gvm(gvm)
{
  mouse = { -1, -1 };

  cube = Quad(vec3(-1.0f, -1.0f, 0.0f), 2.0f, 2.0f);

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

  cube.setRotation(vec3(1.6f, 0.0f, 0.0f));

  camera.setPosition(vec3(0, 0, 5.0f));
  camera.setTarget(vec3(0, 0, 0.0f));

  std::fill(keymap, keymap + 256, false);
}

void MainView::render()
{
  auto r = gvm->renderer();

  gvm->clear({ 0, 0, 0 });

  glm::mat4 viewMatrix = camera.transform();

  glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f), float(WIDTH) / float(HEIGHT), 0.01f, 1.0f);

  {
    glm::mat4 modelMatrix = mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, cube.scale());
    modelMatrix = glm::translate(modelMatrix, /*cube.position()*/ vec3(-2.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, cube.rotation().x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, cube.rotation().y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, cube.rotation().z, glm::vec3(0.0f, 0.0f, 1.0f));


    glm::mat4 transformMatrix = projectionMatrix * viewMatrix * modelMatrix;
    
    std::array<vec3, 3> vertices = { cube[0], cube[1], cube[2] };
    
    std::for_each(vertices.begin(), vertices.end(), [&transformMatrix](vec3& v) {
      vec4 tv = transformMatrix * vec4(v, 1.0f);
      tv /= tv.w;
      v = tv.xyz();
    });
    
    std::array<vec3, 3> vertexColors = { vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) };
    std::array<vec2, 3> textureCoord = { vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f) };

    auto triangle = rasterizer.projectRectangle(vertices);
    
    for (size_t x = 0; x < WIDTH; ++ x)
      for (size_t y = 0; y < HEIGHT; ++y)
      {
        if (math::intersections::is2dPointInsideTriangle(vec2(x, y), triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]))
        {
          //vec3 color = rasterizer.computeCorrectedVertexAttribute(triangle, vertexColors, vec2(x, y));
          //gvm->fillRect(x, y, 1, 1, { (u8)(color.r * 255), (u8)(color.g * 255), (u8)(color.b * 255) });

          vec2 tx = rasterizer.computeCorrectedVertexAttribute(triangle, textureCoord, vec2(x, y));
          gvm->fillRect(x, y, 1, 1, texture.get(tx));
        }
      }
  }

  {
    glm::mat4 modelMatrix = mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, cube.scale());
    modelMatrix = glm::translate(modelMatrix, /*cube.position()*/ vec3(2.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, cube.rotation().x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, cube.rotation().y + 3.1415f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, cube.rotation().z, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 transformMatrix = projectionMatrix * viewMatrix * modelMatrix;

    std::array<vec3, 3> vertices = { cube[0], cube[1], cube[2] };

    std::for_each(vertices.begin(), vertices.end(), [&transformMatrix](vec3& v) {
      vec4 tv = transformMatrix * vec4(v, 1.0f);
      tv /= tv.w;
      v = tv.xyz();
      });

    std::array<vec3, 3> vertexColors = { vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) };
    std::array<vec2, 3> textureCoord = { vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f) };


    auto triangle = rasterizer.projectRectangle(vertices);

    for (size_t x = 0; x < WIDTH; ++x)
      for (size_t y = 0; y < HEIGHT; ++y)
      {
        if (math::intersections::is2dPointInsideTriangle(vec2(x, y), triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]))
        {
          //vec3 color = rasterizer.computeVertexAttribute(triangle, vertexColors, vec2(x, y));
          //gvm->fillRect(x, y, 1, 1, { (u8)(color.r * 255), (u8)(color.g * 255), (u8)(color.b * 255) });

          vec2 tx = rasterizer.computeVertexAttribute(triangle, textureCoord, vec2(x, y));
          gvm->fillRect(x, y, 1, 1, texture.get(tx));
        }
      }
  }



  /*for (const auto& vertex : cube)
  {
    vec4 point = transformMatrix * vec4(vertex, 1.0f);
    point /= point.w;

    vec2 rasterPoint = vec2(point.x * WIDTH / 2.0f + WIDTH / 2.0f, -point.y * HEIGHT / 2.0f + HEIGHT / 2.0f);

    float z = 1.0f;// (point.z + 1.0f) / 2.0f;

    //gvm->fillRect(rasterPoint.x, rasterPoint.y, 1, 1, { 255, 255, 255 });
  }*/

  if (keymap[SDL_SCANCODE_DOWN])
    camera.setPosition(camera.position() + vec3(0, 0, 1)*0.05f);
  else if (keymap[SDL_SCANCODE_UP])
    camera.setPosition(camera.position() + vec3(0, 0, 1)*-0.05f);
  else if (keymap[SDL_SCANCODE_A])
    camera.setPosition(camera.position() + vec3(1, 0, 0) * -0.05f);
  else if (keymap[SDL_SCANCODE_D])
    camera.setPosition(camera.position() + vec3(1, 0, 0) * +0.05f);

  cube.setRotation(cube.rotation() + vec3(0.01f, 0.0f, 0.0f));
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