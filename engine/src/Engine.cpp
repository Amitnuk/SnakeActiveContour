#include "Engine.hpp"
#include <algorithm>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include <stdexcept>





int Engine::s_iRadiusSlider=1;
int Engine::s_iMinRadius = 1;   
int Engine::s_iMaxRadius = 250;   
cv::Mat ImgDst;
constexpr float PI = 3.14159265358979323846f;

Engine::Engine( const std::string& sImagePath)
: m_sImagePath(sImagePath)
, m_oImage(cv::imread(m_sImagePath))
,  m_bDrawControlPoints(false)
{
    if(m_oImage.empty()) 
    {
      throw std::invalid_argument("Wrong image path: " + m_sImagePath);
    }
   
}



void Engine::run()
{

  char sTrackbarName[50];
  std::sprintf( sTrackbarName, "radius x %d", s_iMaxRadius );
  cv::namedWindow("Snake:Active Contours", cv::WINDOW_AUTOSIZE);
  
  cv::createTrackbar(sTrackbarName, "Snake:Active Contours", &s_iRadiusSlider, s_iMaxRadius, on_track, this);
  on_track(s_iRadiusSlider, this);
  

  while(!m_bDrawControlPoints)
  {
    int key = cv::waitKey(1);
    if(key != -1)
    {
      std::cout << "Pressed Key: " << key << std::endl;
      if(key == 'd')
      {
          m_bDrawControlPoints = true;
          break;
      }  
      else 
      {
        std::cout << "Press d to draw\n";
      }
    }
    
  }
  
 
  generateControlPoints(15);
  drawControlPoints();  
  cv::waitKey(0);
}


void Engine::generateControlPoints(int N/*=10*/)
{

  if(N <= 2) 
  {
    N = 3;
  }
  
  std::cout << "Generating: " << N << " control points\n";
  m_vControlPoints.clear();
  m_vControlPoints.reserve(N);

  float fDelta = 2*PI/N;
  float fAngle;
  auto [iWidth, iHeight] = m_oImage.size();

  int cx = iWidth/2.0f, cy = iHeight/2.0f;
  for(int i =0; i < N; ++i) 
  {
    fAngle = i * fDelta;
    m_vControlPoints.push_back(cv::Point2f(cx + s_iRadiusSlider * std::cos(fAngle), cy + s_iRadiusSlider* std::sin(fAngle)));
  }
  
}


void Engine::on_track(int , void* pDataFromUser)
{
  
  double dRadius = std::max(static_cast<double>(s_iRadiusSlider), static_cast<double>(s_iMinRadius));
  dRadius = std::min(dRadius, static_cast<double>(s_iMaxRadius));
  const Engine* const pEngine = static_cast<Engine*>(pDataFromUser);
  ImgDst = pEngine->m_oImage.clone();

  auto [iWidth, iHeight] = pEngine->m_oImage.size();
  
  cv::circle(ImgDst, 
             cv::Point(iWidth/2, iHeight/2), 
             static_cast<int>(dRadius), 
             cv::Scalar(255,0,0), 
             3);

  cv::imshow("Snake:Active Contours", ImgDst);
}


void Engine::drawControlPoints()
{

    

  for(const auto& vPoint : m_vControlPoints) 
  {
    cv::circle(ImgDst, vPoint, 7, cv::Scalar(0,0,255), -1);
  }
  cv::imshow("Snake:Active Contours", ImgDst);
  
}
