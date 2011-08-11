#include <iostream>
#include <stdexcept>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <XnCppWrapper.h>

// �ݒ�t�@�C���̃p�X(���ɍ��킹�ĕύX���Ă�������)
const char* CONFIG_XML_PATH = "SamplesConfig.xml";

int main (int argc, char * argv[])
{
    IplImage* camera = 0;

    try {
        // �R���e�L�X�g�̏����� ... (1)
        xn::Context context;
        XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // �C���[�W�W�F�l���[�^�̍쐬 ... (2)
        xn::ImageGenerator image;
        rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // �J�����T�C�Y�̃C���[�W���쐬(8bit��RGB) ... (3)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode(outputMode);
        camera = ::cvCreateImage(cvSize(outputMode.nXRes, outputMode.nYRes),
            IPL_DEPTH_8U, 3);
        if (!camera) {
            throw std::runtime_error("error : cvCreateImage");
        }

        // ���C�����[�v
        while (1) {
            // �J�����C���[�W�̍X�V��҂��A�摜�f�[�^���擾���� ... (4)
            context.WaitOneUpdateAll(image);
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // �J�����摜�̕\�� ... (5)
            //  Kinect����̓��͂�RGB�ł��邽�߁ABGR�ɕϊ����ĕ\������
            memcpy(camera->imageData, imageMD.RGB24Data(), camera->imageSize);
            ::cvCvtColor(camera, camera, CV_RGB2BGR);
            ::cvShowImage("openni_opencv21", camera);

            // �L�[�̎擾
            char key = cvWaitKey(10);
            // �I������
            if (key == 'q') {
                break;
            }
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    ::cvReleaseImage(&camera);

    return 0;
}
