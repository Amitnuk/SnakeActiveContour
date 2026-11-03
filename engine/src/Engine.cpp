#include "Engine.hpp"
#include <algorithm>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include <opencv2/imgproc.hpp>
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
    cv::resize(m_oImage, m_oImage, cv::Size(600,600));
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
  
 
  generateControlPoints(10);
  drawControlPoints(5, cv::Scalar(0,0,255)); 
  greedAlgorithm();
  drawControlPoints(4, cv::Scalar(0,255,255));  
  cv::waitKey(0);
}




void Engine::on_track(int , void* pDataFromUser)
{
  
  double dRadius = std::max(static_cast<double>(s_iRadiusSlider), static_cast<double>(s_iMinRadius));
  dRadius = std::min(dRadius, static_cast<double>(s_iMaxRadius));
  const Engine* const pEngine = static_cast<Engine*>(pDataFromUser);
  if(pEngine->m_bDrawControlPoints) 
  {
    return;
  }

  ImgDst = pEngine->m_oImage.clone();

  auto [iWidth, iHeight] = pEngine->m_oImage.size();
  
  cv::circle(ImgDst, 
             cv::Point(iWidth/2, iHeight/2), 
             static_cast<int>(dRadius), 
             cv::Scalar(255,0,0), 
             3);

  cv::imshow("Snake:Active Contours", ImgDst);
}


void Engine::generateControlPoints(int iNbCrontrolPoint/*=10*/)
{

  if(iNbCrontrolPoint <= 2) 
  {
    iNbCrontrolPoint = 3;
  }
  
  std::cout << "Generating: " << iNbCrontrolPoint << " control points\n";
  m_vControlPoints.clear();
  m_vControlPoints.reserve(iNbCrontrolPoint);

  float fDelta = 2*PI/iNbCrontrolPoint;
  float fAngle;
  auto [iWidth, iHeight] = m_oImage.size();

  int cx = iWidth/2.0f, cy = iHeight/2.0f;
  for(int i = 0; i < iNbCrontrolPoint; ++i) 
  {
    fAngle = i * fDelta;
    m_vControlPoints.push_back(cv::Point2f(cx + s_iRadiusSlider * std::cos(fAngle), cy + s_iRadiusSlider* std::sin(fAngle)));
  }
  
}


void Engine::drawControlPoints(int iThickness/*=3*/, cv::Scalar oColor/*=cv::Scalar(0,0,255)*/)
{

//  ImgDst = m_oImage.clone();
  for(const auto& vPoint : m_vControlPoints) 
  {
    cv::circle(ImgDst, vPoint, iThickness, oColor, -1);
  }
  cv::imshow("Snake:Active Contours", ImgDst);
  
}


void Engine::greedAlgorithm(Version eVersion /*=Version::V1*/)
{

  switch(eVersion) 
  {
    case Version::V1:
    {
      minimizeImageEnergy();
      break;
    }
    case Version::V2:
    {
      break;
    }
    case Version::V3:
    {
      break;
    }
    default:
    {
      throw std::invalid_argument("invalid version");
    }
  }

}




void Engine::minimizeImageEnergy()
{

  cv::Mat oGx, oGy, ImgDstGray;
  cv::cvtColor(m_oImage, ImgDstGray, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(ImgDstGray,  ImgDstGray, cv::Size(7,7), 5.0f);
  const int iKernelSize = 7;
  //cv::threshold(ImgDstGray, ImgDstGray, 0, 1, cv::THRESH_BINARY_INV);
  cv::Sobel(ImgDstGray, oGx, CV_32F, 1, 0, iKernelSize);
  cv::Sobel(ImgDstGray, oGy, CV_32F, 0, 1, iKernelSize);
  cv::Mat oEnergy = -(oGx.mul(oGx) + oGy.mul(oGy));

  int iRadius = 41;
  auto [iWidth,iHeight] = ImgDstGray.size();
  
  float fEpsilon = 10e-3;
  constexpr int iMaxIterations = 100;
  int iIteration = 0;
  
  while(iIteration++ < iMaxIterations)
  {
    float fDistance = 0.0f;
    bool bFoundBestPt = false;
    [[maybe_unused]]int index = 0;
    for(auto& vPoint : m_vControlPoints) 
    {

      cv::Point2i vPt = vPoint;
      auto [v,u] = vPt;
      
      float fMinValue = oEnergy.at<float>(u,v);
      cv::Point2f vBestPoint;

      for(int i = -iRadius; i <= iRadius; ++i)
      {
        for(int j = -iRadius; j <= iRadius; ++j)
        {
         
          int iRow = i + u;
          int iCol = j + v;
          if((iRow > 0 && iRow < iHeight) && (iCol > 0 && iCol < iWidth))
          {
            float fValue = oEnergy.at<float>(iRow,iCol);
            if( fValue < fMinValue )
            {
              fMinValue = fValue;
              vBestPoint = cv::Point2f(iCol, iRow);
              bFoundBestPt = true;
            }
          }
        }
      }

      if(bFoundBestPt) 
      {
        //std::cout <<"index :" << index <<std::endl;
        //std::cout <<"Old :"<<vPoint.x <<","<< vPoint.y << std::endl;
        fDistance += cv::norm(vBestPoint - vPoint);
        
        vPoint = vBestPoint;
        //std::cout <<"New :"<<vPoint.x <<","<< vPoint.y << std::endl;
        bFoundBestPt = false;
      }
      index++;
    }
    
    std::cout << "Iteration: " << iIteration
          << " | Total displacement: " << fDistance << '\n';

    if(fDistance <= fEpsilon)
    {
      std::cout << "Converged after " << iIteration
                << " iterations.\n";
      break;
    }

    if (iIteration == iMaxIterations)
    {
      std::cout << "Maximum iterations reached ("
                << iMaxIterations << ").\n";
    }
  }
   
   
  
  cv::imshow("Snake:Active Contours", ImgDst);
}


