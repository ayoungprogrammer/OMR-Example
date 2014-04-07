#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>
#include <iostream>

using namespace cv;
using namespace std;
using namespace zbar;

//g++ main.cpp /usr/local/include/ /usr/local/lib/ -lopencv_highgui.2.4.8 -lopencv_core.2.4.8

void drawQRCodes(Mat img,Image& image){
  // extract results  
    for(Image::SymbolIterator symbol=image.symbol_begin();  symbol != image.symbol_end();++symbol) {  
      vector<Point> vp; 

      //draw QR Codes
      int n = symbol->get_location_size();  
        for(int i=0;i<n;i++){  
          vp.push_back(Point(symbol->get_location_x(i),symbol->get_location_y(i))); 

        }  
        RotatedRect r = minAreaRect(vp);  
        Point2f pts[4];  
        r.points(pts);  
        //Display QR code
        for(int i=0;i<4;i++){  
          line(img,pts[i],pts[(i+1)%4],Scalar(255,0,0),3);  
        } 
    } 
}

Rect makeRect(float x,float y,float x2,float y2){
  return Rect(Point2f(x,y),Point2f(x2,y2));
}

Point2f rotPoint(Point2f p,Point2f o,double rad){
  Point2f p1 = Point2f(p.x-o.x,p.y-o.y);

  return Point2f(p1.x * cos(rad)-p1.y*sin(rad)+o.x,p1.x*sin(rad)+p1.y*cos(rad)+o.y);
}

void drawRects(Mat& img,Point2f rtr,Point2f rbl){
    vector<Rect> rects;

    Point2f tr(1084,76);
    Point2f bl(77,1436);

    

    rects.push_back(makeRect(223,105,603,152));
    rects.push_back(makeRect(223,152,603,198));
    rects.push_back(makeRect(223,198,603,244));
    rects.push_back(makeRect(223,244,603,290));
    rects.push_back(makeRect(223,291,603,336));

    rects.push_back(makeRect(129,491,765,806));


    //Fix rotation angle
    double angle = atan2(tr.y-bl.y,tr.x-bl.x);
    double realAngle = atan2(rtr.y-rbl.y,rtr.x-rbl.x);

    double angleShift = -(angle-realAngle);

    //Rotate image
    Point2f rc((rtr.x+rbl.x)/2,(rbl.y+rtr.y)/2);
    Mat rotMat = getRotationMatrix2D(rc,angleShift/3.14159265359*180.0,1.0);
    warpAffine(img,img,rotMat,Size(img.cols,img.rows),INTER_CUBIC,BORDER_TRANSPARENT);

    rtr = rotPoint(rtr,rc,-angleShift);
    rbl = rotPoint(rbl,rc,-angleShift);

    //Calculate ratio between template and real image
    double realWidth = rtr.x-rbl.x;
    double realHeight = rbl.y-rtr.y;

    double width = tr.x-bl.x;
    double height = bl.y - tr.y;

    double wr = realWidth/width;
    double hr = realHeight/height;

    circle(img,rbl,3,Scalar(0,255,0),2);
    circle(img,rtr,3,Scalar(0,255,0),2);

    for(int i=0;i<rects.size();i++){
      Rect r = rects[i];
      double x1 = (r.x-tr.x)*wr+rtr.x;
      double y1 = (r.y-tr.y)*hr+rtr.y;
      double x2 = (r.x+r.width-tr.x)*wr +rtr.x;
      double y2 = (r.y+r.height-tr.y)*hr + rtr.y;
      rectangle(img,Point2f(x1,y1),Point2f(x2,y2),Scalar(0,0,255),3);
      //circle(img,Point2f(x1,y1),3,Scalar(0,0,255));
    }

    
}

int main(int argc, char* argv[])
{
    

    Mat img = imread(argv[1]);

    ImageScanner scanner;  
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);  

    namedWindow("OMR",CV_WINDOW_AUTOSIZE); //create a window

    Mat grey;
    cvtColor(img,grey,CV_BGR2GRAY);

    int width = img.cols;  
    int height = img.rows;  
    uchar *raw = (uchar *)grey.data;  
    // wrap image data  
    Image image(width, height, "Y800", raw, width * height);  
    // scan the image for barcodes  
    scanner.scan(image);  

    //Top right point
    Point2f tr(0,0);
    Point2f bl(0,0);

    // extract results  
    for(Image::SymbolIterator symbol = image.symbol_begin();  symbol != image.symbol_end();++symbol) {  
      vector<Point> vp; 

      //Find TR point
      if(tr.y==0||tr.y>symbol->get_location_y(3)){
        tr = Point(symbol->get_location_x(3),symbol->get_location_y(3));
      }

      //Find BL point
      if(bl.y==0||bl.y<symbol->get_location_y(1)){
        bl = Point(symbol->get_location_x(1),symbol->get_location_y(1));
      }
    } 

    drawQRCodes(img,image);
    drawRects(img,tr,bl);
    imwrite("omr.jpg", img); 

   

    //show the frame in "MyVideo" window
    //waitKey(0);

    return 0;

}