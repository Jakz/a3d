#pragma once

#include "Common.h"

#include "SDL.h"
#include "SDL_image.h"

#include <cstdint>
#include <cstdio>
#include <cassert>

#if !_WIN32
constexpr int32_t WIDTH = 320;
constexpr int32_t HEIGHT = 240;
#define MOUSE_ENABLED false
#else
constexpr int32_t WIDTH = 320;
constexpr int32_t HEIGHT = 240;
#define WINDOW_SCALE 2
#define MOUSE_ENABLED true
#endif

template<typename EventHandler, typename Renderer>
class SDL
{
protected:
  EventHandler& eventHandler;
  Renderer& loopRenderer;

  SDL_Window* _window;
  SDL_Renderer* _renderer;
  SDL_Texture* _canvas;

  bool willQuit;
  u32 ticks;
  float _lastFrameTicks;

  u32 frameRate;
  float ticksPerFrame;


public:
  SDL(EventHandler& eventHandler, Renderer& loopRenderer) : eventHandler(eventHandler), loopRenderer(loopRenderer),
    _window(nullptr), _renderer(nullptr), _canvas(nullptr), willQuit(false), ticks(0)
  {
    setFrameRate(60);
  }

  void setFrameRate(u32 frameRate)
  {
    this->frameRate = frameRate;
    this->ticksPerFrame = 1000 / (float)frameRate;
  }

  float lastFrameTicks() const { return _lastFrameTicks; }

  bool init();
  void deinit();
  void capFPS();

  void loop();
  void handleEvents();

  void exit() { willQuit = true; }

  void blit(SDL_Texture* texture, const SDL_Rect& src, int dx, int dy);
  void blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy);
  void blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh);
  void blit(SDL_Texture* texture, int dx, int dy);

  void drawRect(int x, int y, int w, int h, color_t color);
  void fillRect(int x, int y, int w, int h, color_t color);
  void line(int x1, int y1, int x2, int y2, color_t color);
  void point(int x, int y, color_t color);
  void clear(color_t color);

  void toggleMouseCursor(bool visible);

  //void slowTextBlit(TTF_Font* font, int dx, int dy, Align align, const std::string& string);

  SDL_Window* window() { return _window; }
  SDL_Renderer* renderer() { return _renderer; }
};

template<typename EventHandler, typename Renderer>
bool SDL<EventHandler, Renderer>::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    printf("Error on SDL_Init().\n");
    return false;
  }

  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    printf("Error on IMG_Init().\n");
    return false;
  }

  // SDL_WINDOW_FULLSCREEN
  _window = SDL_CreateWindow("3deng", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * WINDOW_SCALE, HEIGHT * WINDOW_SCALE, SDL_WINDOW_OPENGL);
  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  if (WINDOW_SCALE != 1)
    _canvas = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);

  //toggleMouseCursor(false);

  return true;
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::loop()
{
  while (!willQuit)
  {
#if defined(WINDOW_SCALE)
    SDL_SetRenderTarget(_renderer, _canvas);
#endif
    loopRenderer.render();

#if defined(WINDOW_SCALE)
    SDL_SetRenderTarget(_renderer, nullptr);
    SDL_RenderCopy(_renderer, _canvas, nullptr, nullptr);
#endif

    SDL_RenderPresent(_renderer);

    handleEvents();

    capFPS();
  }
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::capFPS()
{
  u32 ticks = SDL_GetTicks();
  u32 elapsed = ticks - SDL::ticks;

  _lastFrameTicks = elapsed;

  if (elapsed < ticksPerFrame)
  {
    SDL_Delay(ticksPerFrame - elapsed);
    _lastFrameTicks = ticksPerFrame;
  }

  SDL::ticks = SDL_GetTicks();
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::deinit()
{
  IMG_Quit();

  if (_canvas)
    SDL_DestroyTexture(_canvas);

  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

  SDL_Quit();
}

template<typename EventHandler, typename Renderer>
void SDL<EventHandler, Renderer>::handleEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_QUIT:
      willQuit = true;
      break;

    case SDL_KEYDOWN:
      eventHandler.handleKeyboardEvent(event, true);
      break;

    case SDL_KEYUP:
      eventHandler.handleKeyboardEvent(event, false);
      break;

#if MOUSE_ENABLED
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:
#if defined(WINDOW_SCALE)
      event.button.x /= WINDOW_SCALE;
      event.button.y /= WINDOW_SCALE;
#endif
      eventHandler.handleMouseEvent(event);
#endif
    }
  }
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  SDL_RenderCopy(_renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, const SDL_Rect& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  SDL_RenderCopy(_renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(texture, { sx, sy, w, h }, dx, dy);
}


template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::blit(SDL_Texture* texture, int dx, int dy)
{
  u32 dummy;
  int dummy2;

  SDL_Rect from = { 0, 0, 0, 0 };
  SDL_Rect to = { dx, dy, 0, 0 };

  SDL_QueryTexture(texture, &dummy, &dummy2, &from.w, &from.h);

  to.w = from.w;
  to.h = from.h;

  SDL_RenderCopy(_renderer, texture, &from, &to);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::drawRect(int x, int y, int w, int h, color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
  SDL_Rect rect = { x, y, w, h };
  SDL_RenderDrawRect(_renderer, &rect);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::fillRect(int x, int y, int w, int h, color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
  SDL_Rect rect = { x, y, w, h };
  SDL_RenderFillRect(_renderer, &rect);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::point(int x, int y, color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawPoint(_renderer, x, y);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::line(int x1, int y1, int x2, int y2, color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawLine(_renderer, x1, y1, x2, y2);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::clear(color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, 255);
  SDL_RenderClear(_renderer);
}

template<typename EventHandler, typename Renderer>
inline void SDL<EventHandler, Renderer>::toggleMouseCursor(bool visible)
{
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}
