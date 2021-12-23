#include <cstdio>
#include <cstdlib>

#include "gfx/ViewManager.h"


#include <functional>


int main(int argc, char* argv[])
{
  ui::ViewManager ui;

  if (!ui.init())
    return -1;

  if (!ui.loadData())
  {
    printf("Error while loading and initializing data.\n");
    ui.deinit();
    return -1;
  }

  ui.loop();
  ui.deinit();

  //getchar();
  return 0;
}