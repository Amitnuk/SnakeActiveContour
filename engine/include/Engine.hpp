#pragma once 
#include <opencv2/opencv.hpp> 
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>


class Engine
{
  public:
    Engine() =delete;
    explicit Engine(const std::string& ImagePath);
    ~Engine() = default;
    void run();

    
  const std::string& m_sImagePath;

};
