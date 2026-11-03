#pragma once 

#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp> 
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <vector>


enum class Version
{
  V1,
  V2,
  V3
};


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
    static int s_iRadiusSlider;
    static int s_iMinRadius;   
    static int s_iMaxRadius;   
    std::vector<cv::Point2f> m_vControlPoints;

    static void on_track(int, void*);
    void generateControlPoints(int iNbCrontrolPoint = 10);
    void drawControlPoints(int iThickness=3, cv::Scalar oColor=cv::Scalar(0,0,255));
    void greedAlgorithm(Version eVersion = Version::V1);

    void minimizeImageEnergy();
    void getMinimumNeighboorsValue(cv::Mat& oEnergy, int u, int v, int iHeight, int iWidth, float& fMinValue, float fValue );
};
