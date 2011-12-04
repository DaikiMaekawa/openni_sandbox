#include <iostream>
#include <stdexcept>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <XnCppWrapper.h>
#include <XnVSessionManager.h>
#include <XnVSelectableSlider1D.h>

// �ݒ�t�@�C���̃p�X(���ɍ��킹�ĕύX���Ă�������)
const char* CONFIG_XML_PATH = "SamplesConfig.xml";


// �Z�b�V�����J�n�̌��o��ʒm�����R�[���o�b�N
void XN_CALLBACK_TYPE SessionDetected(const XnChar* strFocus,
    const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
    std::cout << "SessionDetected:" << strFocus << "," << fProgress << std::endl;
}

// �Z�b�V�����̊J�n��ʒm�����R�[���o�b�N
void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& pFocus, void* UserCxt)
{
    std::cout << "SessionStart" << std::endl;
}

// �Z�b�V�����̏I����ʒm�����R�[���o�b�N
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt)
{
    std::cout << "SessionEnd" << std::endl;
}

// �X���C�h�Ώۂ��ς������Ă΂��
void XN_CALLBACK_TYPE MainSlider_OnHover(XnInt32 nItem, void* cxt)
{
    std::cout << __FUNCTION__ << "[" << nItem << "]" << std::endl;
}

// �I�����ꂽ���ɌĂ΂��
void XN_CALLBACK_TYPE MainSlider_OnSelect(XnInt32 nItem, XnVDirection dir, void* cxt)
{
    std::cout << __FUNCTION__ << "[" << XnVDirectionAsString(dir) << "]" << std::endl;
}

// �l���ς�������ɌĂ΂��
void XN_CALLBACK_TYPE MainSlider_OnValueChange(XnFloat fValue, void* cxt)
{
    std::cout << __FUNCTION__ << "[" << fValue << "]" << std::endl;
}

int main (int argc, char * const argv[]) {
    IplImage* camera = 0;

    try {
        // OpenNI�̃R���e�L�X�g������������
        xn::Context context;
        XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }        

        // �C���[�W�W�F�l���[�^�̍쐬
        xn::ImageGenerator image;
        rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // �J�����T�C�Y�̃C���[�W���쐬(8bit��RGB)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode(outputMode);
        camera = ::cvCreateImage(cvSize(outputMode.nXRes, outputMode.nYRes),
            IPL_DEPTH_8U, 3);
        if (!camera) {
            throw std::runtime_error("error : cvCreateImage");
        }


        // NITE�̃Z�b�V�����}�l�[�W���[������������
        XnVSessionManager sessionManager;
        rc = sessionManager.Initialize(&context, "Wave,Click", "RaiseHand");
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // �Z�b�V�����̊J�n�I����ʒm����R�[���o�b�N��o�^����
        XnVHandle sessionCallnack = sessionManager.RegisterSession( this, &SessionStart, &SessionEnd , &SessionDetected);

        XnVSelectableSlider1D slider( 3 );
	    slider.RegisterItemHover(NULL, &MainSlider_OnHover);
	    slider.RegisterItemSelect(NULL, &MainSlider_OnSelect);
	    slider.RegisterValueChange(NULL, &MainSlider_OnValueChange);

        // OffAxis
        //  ���͂���Fhttp://ejje.weblio.jp/content/off+axis
        // ���͂��ꂵ�Ă��A�l�̕ύX��ʒm���邩�ǂ���
	    //slider.SetValueChangeOnOffAxis(true);

        XnPoint3D center = { 0 };
	    slider.GetCenter( center );
        std::cout << "Center      : " << center.X << "," << center.Y << std::endl;
	    std::cout << "ItemCoun  t : " << slider.GetItemCount() << std::endl;
	    std::cout << "BorderWidth : " << slider.GetBorderWidth() << std::endl;
	    std::cout << "SliderSize  : " << slider.GetSliderSize() << std::endl;

        sessionManager.AddListener( &slider );

        // ���C�����[�v
        while (1) {
            // ���ꂼ��̃f�[�^���X�V����
            context.WaitAndUpdateAll();
            sessionManager.Update(&context);

            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // �J�����摜�̕\��
            memcpy(camera->imageData, imageMD.RGB24Data(), camera->imageSize);
            ::cvCvtColor(camera, camera, CV_BGR2RGB);
            ::cvShowImage("KinectImage", camera);

            // �L�[�̎擾
            char key = cvWaitKey(10);
            // �I������
            if (key == 'q') {
                break;
            }
        }

        // �Z�b�V�����̃R�[���o�b�N���폜����
        sessionManager.UnregisterSession(sessionCallnack);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    ::cvReleaseImage(&camera);

    return 0;
}
