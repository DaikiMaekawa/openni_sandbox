// Windows �̏ꍇ��Release�R���p�C���ɂ����
// �����I�ȑ��x�œ��삵�܂�
#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <XnCppWrapper.h>

const char* CONFIG_XML_PATH = "SamplesConfig.xml";

// ���[�U�[�̐F�Â�
const XnFloat Colors[][3] =
{
    {1,1,1},    // ���[�U�[�Ȃ�
    {0,1,1},  {0,0,1},  {0,1,0},
    {1,1,0},  {1,0,0},  {1,.5,0},
    {.5,1,0}, {0,.5,1}, {.5,0,1},
    {1,1,.5},
};

class UserCallback
{
    friend class User;

public:

    virtual ~UserCallback(){}

protected:

    static void XN_CALLBACK_TYPE Detected( xn::UserGenerator& generator, XnUserID nId, void* pCookie )
    {
        ((UserCallback*)pCookie)->Detected( generator, nId );
    }

    static void XN_CALLBACK_TYPE Lost( xn::UserGenerator& generator, XnUserID nId, void* pCookie )
    {
        ((UserCallback*)pCookie)->Lost( generator, nId );
    }

    virtual void Detected( xn::UserGenerator& generator, XnUserID nId )
    {
    }

    virtual void Lost( xn::UserGenerator& generator, XnUserID nId )
    {
    }
};

class User
{
public:

    User()
        : userCallbacks_( 0 )
    {
    }

    virtual ~User()
    {
        if ( user_.IsValid() && (userCallbacks_ != 0) ) {
            user_.UnregisterUserCallbacks(userCallbacks_);
        }
    }

    xn::UserGenerator& GetUserGenerator() { return user_; }
    void RegisterCallback( UserCallback* callback )
    {
        user_.RegisterUserCallbacks( &UserCallback::Detected, &UserCallback::Lost, callback, userCallbacks_);
    }

protected:

    xn::UserGenerator user_;
    XnCallbackHandle userCallbacks_;
};

class App : public UserCallback
{
protected:

    virtual void Detected( xn::UserGenerator& generator, XnUserID nId )
    {
        std::cout << "���[�U�[���o:" << nId << " " << generator.GetNumberOfUsers() << "�l��" << std::endl;
    }

    virtual void Lost( xn::UserGenerator& generator, XnUserID nId )
    {
        std::cout << "���[�U�[����:" << nId << std::endl;
    }

};

// RGB�s�N�Z���̏�����
inline XnRGB24Pixel xnRGB24Pixel( int r, int g, int b )
{
    XnRGB24Pixel pixel = { r, g, b };
    return pixel;
}

int main (int argc, char * argv[])
{
    IplImage* camera = 0;

    try {
        // �R���e�L�X�g�̏�����
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

        // �f�v�X�W�F�l���[�^�̍쐬
        xn::DepthGenerator depth;
        rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // �f�v�X�̍��W���C���[�W�ɍ��킹��
        //  ���[�U�[���W�̃r���[�|�C���g���f�v�X�̍��W�ō��킹��
        depth.GetAlternativeViewPointCap().SetViewPoint(image);

        // ���[�U�[�̍쐬
        User user;
        rc = context.FindExistingNode( XN_NODE_TYPE_USER, user.GetUserGenerator() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // ���[�U�[���o�@�\���T�|�[�g���Ă��邩�m�F
        if (!user.GetUserGenerator().IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
            throw std::runtime_error("���[�U�[���o���T�|�[�g���Ă܂���");
        }

        // ���[�U�[�F���̃R�[���o�b�N��o�^
        App app;
        user.RegisterCallback( &app );

        // �W�F�X�`���[���o�̊J�n
        context.StartGeneratingAll();

        // �J�����T�C�Y�̃C���[�W���쐬(8bit��RGB)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode(outputMode);
        camera = ::cvCreateImage(cvSize(outputMode.nXRes, outputMode.nYRes),
            IPL_DEPTH_8U, 3);
        if (!camera) {
            throw std::runtime_error("error : cvCreateImage");
        }

        // �\�����
        bool isShowImage = true;
        bool isShowUser = true;

        // ���C�����[�v
        while (1) {
            // ���ׂẴm�[�h�̍X�V��҂�
            context.WaitAndUpdateAll();

            // �摜�f�[�^�̎擾
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // ���[�U�[�f�[�^�̎擾
            xn::SceneMetaData sceneMD;
            user.GetUserGenerator().GetUserPixels(0, sceneMD);

            // �J�����摜�̕\��
            char* dest = camera->imageData;
            const xn::RGB24Map& rgb = imageMD.RGB24Map();
            for (int y = 0; y < imageMD.YRes(); ++y) {
                for (int x = 0; x < imageMD.XRes(); ++x) {
                    // ���[�U�[�\��
                    XnLabel label = sceneMD(x, y);
                    if (!isShowUser) {
                        label = 0;
                    }

                    // �J�����摜�̕\��
                    XnRGB24Pixel pixel = rgb(x, y);
                    if (!isShowImage) {
                        pixel = xnRGB24Pixel( 255, 255, 255 );
                    }

                    // �o�͐�ɕ`��
                    dest[0] = pixel.nRed   * Colors[label][0];
                    dest[1] = pixel.nGreen * Colors[label][1];
                    dest[2] = pixel.nBlue  * Colors[label][2];
                    dest += 3;
                }
            }

            ::cvCvtColor(camera, camera, CV_BGR2RGB);
            ::cvShowImage("KinectImage", camera);

            // �L�[�C�x���g
            char key = cvWaitKey(10);
            // �I������
            if (key == 'q') {
                break;
            }
            // �\������/���Ȃ��̐؂�ւ�
            else if (key == 'i') {
                isShowImage = !isShowImage;
            }
            else if (key == 'u') {
                isShowUser = !isShowUser;
            }
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    ::cvReleaseImage(&camera);

    return 0;
}
