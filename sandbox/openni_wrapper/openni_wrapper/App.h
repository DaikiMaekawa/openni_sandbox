#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv2/opencv.hpp>

#include <XnCppWrapper.h>
#include "openni/User.h"
#include "openni/Pose.h"
#include "openni/Skeleton.h"
#include "openni/Player.h"

#include "nite/SessionManager.h"
#include "nite/SwipeDetector.h"

#include "SkeltonDrawer.h"

// ���[�U�[�̐F�Â�
const XnFloat Colors[][3] =
{
    {1,1,1},    // ���[�U�[�Ȃ�
    {0,1,1},  {0,0,1},  {0,1,0},
    {1,1,0},  {1,0,0},  {1,.5,0},
    {.5,1,0}, {0,.5,1}, {.5,0,1},
    {1,1,.5},
};

class App : public openni::UserCallback,
            public openni::PoseCallback,
            public openni::SkeletonCallback,
            public nite::SessionManagerCallback,
            public nite::SwipeDetectorCallback, 
            public openni::PlayerCallback
{
public:

    App()
        : isShowImage( true )
        , isShowUser( true )
        , isShowSkeleton( true )
    {
    }

    App( const std::string& xmlFileName, const std::string& recordFileName )
        : isShowImage( true )
        , isShowUser( true )
        , isShowSkeleton( true )
    {
    }

    App( const std::string& recordFileName )
        : isShowImage( true )
        , isShowUser( true )
        , isShowSkeleton( true )
    {
    }

    void InitFromXml( const std::string& xmlFileName, const std::string& recordFileName )
    {
        // �R���e�L�X�g�̏�����
        XnStatus rc = context.InitFromXmlFile( xmlFileName.c_str() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // ���R�[�_�[�̍쐬
        rc = recorder.Create(context);
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // �L�^�ݒ�
        rc = recorder.SetDestination( XN_RECORD_MEDIUM_FILE, recordFileName.c_str() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        InitOpenNI();
        InitOpenCV();
    }

    void InitFromRecord( const std::string& recordFileName )
    {
        XnStatus rc = context.Init();
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // �L�^���ꂽ�t�@�C�����J��
        rc = context.OpenFileRecording( recordFileName.c_str() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // �v���[���[�̍쐬
        rc = context.FindExistingNode( XN_NODE_TYPE_PLAYER, player.GetPlayer() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // �R�[���o�b�N�̐ݒ�
        player.RegisterCallback( this );
        player.GetPlayer().SetRepeat( false );

        InitOpenNI();
        InitOpenCV();
    }

    void Run()
    {
        while ( !player.GetPlayer().IsEOF() ) {
            // ���ׂẴm�[�h�̍X�V��҂�
            context.WaitAndUpdateAll();
            sessionManager.Update();

            // �摜�f�[�^�̎擾
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // ���[�U�[�f�[�^�̎擾
            xn::SceneMetaData sceneMD;
            user.GetUserGenerator().GetUserPixels( 0, sceneMD );

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

            // �X�P���g���̕`��
            if ( isShowSkeleton ) {
                XnUserID users[15];
                XnUInt16 userCount = sizeof(users) / sizeof(users[0]);
                user.GetUserGenerator().GetUsers( users, userCount );
                for ( int i = 0; i < userCount; ++i ) {
                    if (skeleton.GetSkeletonCapability().IsTracking( users[i] ) ) {
                        SkeltonDrawer skeltonDrawer( camera, skeleton.GetSkeletonCapability(), depth, users[i] );
                        skeltonDrawer.draw();
                    }
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

protected:

    void InitOpenNI()
    {
        // �C���[�W�W�F�l���[�^�̍쐬
        XnStatus rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // �f�v�X�W�F�l���[�^�̍쐬
        rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // �f�v�X�̍��W���C���[�W�ɍ��킹��
        //  ���[�U�[���W�̃r���[�|�C���g���f�v�X�̍��W�ō��킹��
        depth.GetAlternativeViewPointCap().SetViewPoint( image );

        // ���[�U�[�̍쐬
        rc = context.FindExistingNode( XN_NODE_TYPE_USER, user.GetUserGenerator() );
        if ( rc != XN_STATUS_OK ) {
            rc = user.GetUserGenerator().Create( context );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

        // ���[�U�[���o�@�\���T�|�[�g���Ă��邩�m�F
        if ( !user.GetUserGenerator().IsCapabilitySupported( XN_CAPABILITY_SKELETON ) ) {
            throw std::runtime_error( "���[�U�[���o���T�|�[�g���Ă܂���" );
        }


        // �L�����u���[�V�����Ƀ|�[�Y���K�v
        skeleton.SetSkeletonCapability( user.GetUserGenerator().GetSkeletonCap() );
        if ( skeleton.GetSkeletonCapability().NeedPoseForCalibration() ) {
            // �|�[�Y���o�̃T�|�[�g�`�F�b�N
            if ( !user.GetUserGenerator().IsCapabilitySupported( XN_CAPABILITY_POSE_DETECTION ) ) {
                throw std::runtime_error( "�|�[�Y���o���T�|�[�g���Ă܂���" );
            }

            // �L�����u���[�V�����|�[�Y�̎擾
            XnChar p[20] = "";
            skeleton.GetSkeletonCapability().GetCalibrationPose( p );
            poseName = p;

            // �|�[�Y���o�̃R�[���o�b�N��o�^
            pose.SetPoseDetectionCapability( user.GetUserGenerator().GetPoseDetectionCap() );
            pose.RegisterCallback( this );
        }

        // ���[�U�[�F���̃R�[���o�b�N��o�^
        user.RegisterCallback( this );

        // �L�����u���[�V�����̃R�[���o�b�N��o�^
        skeleton.RegisterCallback( this );

        // ���[�U�[�g���b�L���O�ŁA���ׂĂ��g���b�L���O����
        skeleton.GetSkeletonCapability().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

        // HandsGenerator�̐���
        rc = context.FindExistingNode( XN_NODE_TYPE_HANDS, hands );
        if ( rc != XN_STATUS_OK ) {
            rc = hands.Create( context );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

        // �L�^����ꍇ�́A�W�F�l���[�^�̐ݒ�
        if ( recorder.IsValid() ) {
            // �C���[�W���L�^�Ώۂɒǉ�
            rc = recorder.AddNodeToRecording( image, XN_CODEC_JPEG );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        
            // �f�v�X���L�^�Ώۂɒǉ�
            rc = recorder.AddNodeToRecording( depth, XN_CODEC_UNCOMPRESSED );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        
            // �L�^�J�n(WaitOneUpdateAll�̃^�C�~���O�ŋL�^�����)
            rc = recorder.Record();
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

        sessionManager.Initialize( context, "Wave,Click,RaiseHand", "RaiseHand" );
        sessionManager.RegisterCallback( this );

        swipeDetector.RegisterCallback( this );
        sessionManager.GetSessionManager().AddListener( &swipeDetector.GetSwipeDetector() );

        // �W�F�X�`���[���o�̊J�n
        context.StartGeneratingAll();
    }

    void InitOpenCV()
    {
        // �J�����T�C�Y�̃C���[�W���쐬(8bit��RGB)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode( outputMode );
        camera = ::cvCreateImage( cvSize( outputMode.nXRes, outputMode.nYRes ), IPL_DEPTH_8U, 3 );
        if ( !camera ) {
            throw std::runtime_error( "error : cvCreateImage" );
        }
    }

protected:

    virtual void UserDetected( xn::UserGenerator& generator, XnUserID nId )
    {
        std::cout << "���[�U�[���o:" << nId << " " << generator.GetNumberOfUsers() << "�l��" << std::endl;
    
        if ( !poseName.empty() ) {
            generator.GetPoseDetectionCap().StartPoseDetection( poseName.c_str(), nId );
        }
        else {
            generator.GetSkeletonCap().RequestCalibration( nId, TRUE );
        }
    }

    virtual void UserLost( xn::UserGenerator& generator, XnUserID nId )
    {
        std::cout << "���[�U�[����:" << nId << std::endl;
    }

    // �|�[�Y���o
    void PoseDetected( xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId )
    {
      std::cout << "�|�[�Y���o:" << strPose << " ���[�U�[:" << nId << std::endl;

      user.GetUserGenerator().GetPoseDetectionCap().StopPoseDetection( nId );
      user.GetUserGenerator().GetSkeletonCap().RequestCalibration( nId, TRUE );
    }

    // �|�[�Y����
    void PoseLost( xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId)
    {
      std::cout << "�|�[�Y����:" << strPose << " ���[�U�[:" << nId << std::endl;
    }

    // �L�����u���[�V�����̊J�n
    void CalibrationStart( xn::SkeletonCapability& capability, XnUserID nId)
    {
      std::cout << "�L�����u���[�V�����J�n�B���[�U�[:" << nId << std::endl;
    }

    // �L�����u���[�V�����̏I��
    void CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess )
    {
      // �L�����u���[�V��������
      if (bSuccess) {
        std::cout << "�L�����u���[�V���������B���[�U�[:" << nId << std::endl;
        user.GetUserGenerator().GetSkeletonCap().StartTracking( nId );
      }
      // �L�����u���[�V�������s
      else {
        std::cout << "�L�����u���[�V�������s�B���[�U�[:" << nId << std::endl;
      }
    }

public:

    // �t�@�C���̏I�[
    virtual void EndOfFileReached( xn::ProductionNode& node )
    {
        std::cout << "�t�@�C���̏I�[" << std::endl;
    }


protected:

    virtual void SessionStart( const XnPoint3D& pFocus )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SessionEnd()
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeUp( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeDown( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeRight( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeLeft( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void Swipe( XnVDirection eDir, XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

private:
    
    // RGB�s�N�Z���̏�����
    inline XnRGB24Pixel xnRGB24Pixel( int r, int g, int b )
    {
        XnRGB24Pixel pixel = { r, g, b };
        return pixel;
    }

private:

    xn::Context context;
    xn::Recorder recorder;
    xn::ImageGenerator image;
    xn::DepthGenerator depth;
    xn::HandsGenerator hands;

    openni::User user;
    openni::Pose pose;
    openni::Skeleton skeleton;
    openni::Player player;

    nite::SessionManager sessionManager;
    nite::SwipeDetector swipeDetector;

    cv::Ptr< IplImage >camera;

    std::string poseName;

    // �\�����
    bool isShowImage;
    bool isShowUser;
    bool isShowSkeleton;
};

