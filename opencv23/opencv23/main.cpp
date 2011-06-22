#include <iostream>

#include <opencv2/opencv.hpp>

void main()
{
  try {
    char* name = "openni";
    cv::namedWindow( name );

    cv::VideoCapture     capture( CV_CAP_OPENNI );
    while ( 1 ) { 
      // �f�[�^�̍X�V��҂�
      capture.grab(); 

      // RGB���擾���ĕ\��
      cv::Mat  rgbImage;
      capture.retrieve( rgbImage, CV_CAP_OPENNI_BGR_IMAGE ); 
      cv::imshow( name, rgbImage );

      if ( cv::waitKey( 10 ) >= 0 ) {
        break; 
      }
    }

    cv::destroyAllWindows();
  }
  catch ( ... ) {
    std::cout << "exception!!" << std::endl;
  }
}
