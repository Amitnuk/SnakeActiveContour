#include <iostream>
#include <string>
#include "Engine.hpp"




// TODO: Add parser
// For now the image will load directory

int main([[maybe_unused]]int arc, [[maybe_unused]]char* argv[])
{
  const std::string& sImagePath = "/home/kuntima/Workspace/githubs/SnakeActiveContour/resources/Hammer.png";
   
  Engine oEngine(sImagePath);
  oEngine.run();

  std::cout << "Snake Active Contour\n";
}
