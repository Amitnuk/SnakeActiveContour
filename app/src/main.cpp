#include <iostream>
#include <string>
#include "Engine.hpp"
#include "config.h"



// TODO: Add parser
// For now the image will load directory

int main([[maybe_unused]]int arc, [[maybe_unused]]char* argv[])
{
  const std::string& sImagePath = GetResourcesDir() + "shapes.png";
   
  Engine oEngine(sImagePath);
  oEngine.run();

  std::cout << "Snake Active Contour\n";
}
