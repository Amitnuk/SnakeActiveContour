#pragma once 

#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp> 
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <vector>

class Engine
{
  public:
    Engine() =delete;
    explicit Engine(const std::string& ImagePath);
    ~Engine() = default;
    void run();

  private :
    const std::string& m_sImagePath;
    cv::Mat m_oImage;
    bool m_bDrawControlPoints;
    static void on_track(int, void*);
    static int s_iRadiusSlider;
    static int s_iMinRadius;   
    static int s_iMaxRadius;   
    std::vector<cv::Point2f> m_vControlPoints;
    void generateControlPoints(int N = 10);
    void drawControlPoints();
};
