#include "Engine.hpp"
#include <algorithm>
#include <cstddef>
#include <opencv2/core.hpp>
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
constexpr float ALPHA = 1.5f;
constexpr float BETA  = 1.5f;
constexpr float GAMMA = 1.5f;
Engine::Engine( const std::string& sImagePath)
: m_sImagePath(sImagePath)
, m_oImage(cv::imread(m_sImagePath))
, m_bDrawControlPoints(false)
, m_bContourFinished(false)
, m_bDrawing(false)
{
    if(m_oImage.empty()) 
    {
      throw std::invalid_argument("Wrong image path: " + m_sImagePath);
    }
    cv::resize(m_oImage, m_oImage, cv::Size(700,700));
}

void Engine::run()
{

  char sTrackbarName[50];
  std::sprintf( sTrackbarName, "radius x %d", s_iMaxRadius );

  bool bHandDraw = true;
  if(bHandDraw) 
  {
    cv::namedWindow("Snake:Active Contours", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Snake:Active Contours", Engine::on_mouse, this);    
  }
  else 
  {
    cv::createTrackbar(sTrackbarName, "Snake:Active Contours", &s_iRadiusSlider, s_iMaxRadius, on_track, this);
    on_track(s_iRadiusSlider, this);
  }
  
  

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
  
  constexpr std::size_t iNBCtrlPts = 75;
  generateControlPoints(iNBCtrlPts, bHandDraw);
  drawControlPoints(5, cv::Scalar(0,0,255)); 
  greedAlgorithm(Version::V2);
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


void Engine::on_mouse(int event, int x, int y, [[maybe_unused]]int flags, void* userdata)
{
    Engine* pEngine = static_cast<Engine*>(userdata);

    if (pEngine->m_bContourFinished)
        return;
    

    switch (event)
    {
        case cv::EVENT_LBUTTONDOWN:
        {
            pEngine->m_bDrawing = true;
            pEngine->m_vDrawnPoints.clear();
            pEngine->m_vDrawnPoints.emplace_back(x, y);
            break;
        }

        case cv::EVENT_MOUSEMOVE:
        {
            if(pEngine->m_bDrawing)
            {
                pEngine->m_vDrawnPoints.emplace_back(x, y);
            }
            break;
        }

        case cv::EVENT_LBUTTONUP:
        {
            if (pEngine->m_bDrawing)
            {
                pEngine->m_bDrawing = false;

                if(pEngine->m_vDrawnPoints.size() >= 3)
                    pEngine->m_bContourFinished = true;
            }
            break;
        }

        default:
            return;
    }

    ImgDst = pEngine->m_oImage.clone();

    const auto& points = pEngine->m_vDrawnPoints;

    for (std::size_t i = 1; i < points.size(); ++i)
    {
        cv::line(ImgDst, points[i - 1], points[i],
                 cv::Scalar(0, 0, 255), 2);

    }

    if (pEngine->m_bContourFinished && points.size() >= 3)
    {
        cv::line(ImgDst, points.back(), points.front(),
                 cv::Scalar(0, 0, 255), 2);
    }

    cv::imshow("Snake:Active Contours", ImgDst);
}


void Engine::generateControlPoints(int iNbCrontrolPoint/*=10*/, bool bHandDrawn/* = false*/)
{

  if(iNbCrontrolPoint <= 2) 
  {
    iNbCrontrolPoint = 3;
  }
  
  std::cout << "Generating: " << iNbCrontrolPoint << " control points\n";
  m_vControlPoints.clear();
  m_vControlPoints.reserve(iNbCrontrolPoint);
  
  if(!bHandDrawn) 
  {
    
  
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
  else 
  {


    float fLength = 0.0f; 
    std::size_t M = m_vDrawnPoints.size();
    for(std::size_t i =0; i < M; ++i) 
    {
      fLength += cv::norm(m_vDrawnPoints[(i + 1)%M] - m_vDrawnPoints[i]);
    } 
    
    const float fSpacing = fLength/iNbCrontrolPoint;
    std::size_t iSegment = 0;
    float fAccumulated = 0.0f;

    
    for(int i = 0; i < iNbCrontrolPoint; ++i) 
    {
        const float fTarget = i * fSpacing;
        while(iSegment < M) 
        {
          const auto& p0 = m_vDrawnPoints[iSegment];
          const auto& p1 = m_vDrawnPoints[(iSegment + 1)%M];

          const float fSegmentLength = cv::norm(p1 - p0);

          if(fAccumulated + fSegmentLength >= fTarget) 
          {
            const float t = fSegmentLength > 0.0f ?(fTarget - fAccumulated)/fSegmentLength : 0.0f;
            m_vControlPoints.emplace_back(p0 + t*(p1-p0));
            break;
          }
          fAccumulated += fSegmentLength;
          ++iSegment;
        }
    }

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
      std::cout << "minimizing image energy\n";
      minimizeImageEnergy();
      break;
    }
    case Version::V2:
    {
      std::cout << "minimizing the  energy\n";
      minimizeTotalEnergy();
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
  cv::GaussianBlur(ImgDstGray,  ImgDstGray, cv::Size(3,3), 5.0f);
  const int iKernelSize = 7;
  //cv::threshold(ImgDstGray, ImgDstGray, 0, 1, cv::THRESH_BINARY_INV);
  cv::Sobel(ImgDstGray, oGx, CV_32F, 1, 0, iKernelSize);
  cv::Sobel(ImgDstGray, oGy, CV_32F, 0, 1, iKernelSize);
  cv::Mat oEnergyImage = -(oGx.mul(oGx) + oGy.mul(oGy));

  int iRadius = iKernelSize;
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
      
      float fMinValue = oEnergyImage.at<float>(u,v);
      cv::Point2f vBestPoint;

      for(int i = -iRadius; i <= iRadius; ++i)
      {
        for(int j = -iRadius; j <= iRadius; ++j)
        {
         
          int iRow = i + u;
          int iCol = j + v;
          if((iRow > 0 && iRow < iHeight) && (iCol > 0 && iCol < iWidth))
          {
            float fValue = oEnergyImage.at<float>(iRow,iCol);
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


void Engine::minimizeTotalEnergy()
{

  cv::Mat oGx, oGy, ImgDstGray;
  const int iKernelSize = 11;
  cv::cvtColor(m_oImage, ImgDstGray, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(ImgDstGray,  ImgDstGray, cv::Size(41,41), 0.0f);
  
  
  cv::Sobel(ImgDstGray, oGx, CV_32F, 1, 0, iKernelSize);
  cv::Sobel(ImgDstGray, oGy, CV_32F, 0, 1, iKernelSize);
  cv::Mat oEnergyImage = -(oGx.mul(oGx) + oGy.mul(oGy));

  int iRadius = iKernelSize;
  auto [iWidth,iHeight] = ImgDstGray.size();
  
  float fEpsilon = 10e-3;
  constexpr int iMaxIterations = 100;
  int iIteration = 0;
  
  while(iIteration++ < iMaxIterations)
  {
    float fDistance = 0.0f;
    bool bFoundBestPt = false;
    [[maybe_unused]]int index = 0;
    const std::size_t N = m_vControlPoints.size();
    for(std::size_t i = 0; i < N ; ++i) 
    {


      auto& vPrevPoint = m_vControlPoints[(i - 1 + N)%N];
      auto& vCurrPoint = m_vControlPoints[i];
      auto& vNextPoint = m_vControlPoints[(i + 1)%N];

      auto [v,u] = vCurrPoint;
      
      float fMinValue = oEnergyImage.at<float>(static_cast<int>(u),static_cast<int>(v)) + 
                        std::pow(cv::norm(vNextPoint - vCurrPoint),2) + 
                        std::pow(cv::norm(vNextPoint - 2*vCurrPoint + vPrevPoint),2);


      cv::Point2f vBestPoint;

      for(int j = -iRadius; j <= iRadius; ++j)
      {
        for(int k = -iRadius; k <= iRadius; ++k)
        {
         
          int iRow = j + u;
          int iCol = k + v;

          if((iRow > 0 && iRow < iHeight) && (iCol > 0 && iCol < iWidth))
          {
            cv::Point2f vCandidate(iCol, iRow);
            float fEnergyImage = oEnergyImage.at<float>(iRow,iCol);
            float fEnergyElastic = std::pow(cv::norm(vNextPoint - vCandidate),2);
            float fEnergySmooth = std::pow(cv::norm(vNextPoint - 2*vCandidate + vPrevPoint),2);
            
            float fTotalEnergy = GAMMA * fEnergyImage + ALPHA * fEnergySmooth + BETA * fEnergyElastic;
            if(  fTotalEnergy < fMinValue )
            {
              fMinValue =  fTotalEnergy;
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
        fDistance += cv::norm(vBestPoint - vCurrPoint);
        
        vCurrPoint = vBestPoint;
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
