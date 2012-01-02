// Windows �̏ꍇ��Release�R���p�C���ɂ����
// �����I�ȑ��x�œ��삵�܂�
#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv2/opencv.hpp>

#include <XnCppWrapper.h>

// ���[�U�[���o
void XN_CALLBACK_TYPE UserDetected( xn::UserGenerator& generator, XnUserID nId, void* pCookie )
{
    std::cout << "���[�U�[���o:" << nId << " " << generator.GetNumberOfUsers() << "�l��" << std::endl;

    generator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// �L�����u���[�V�����̏I��
void XN_CALLBACK_TYPE CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
    // �L�����u���[�V��������
    if ( bSuccess ) {
        std::cout << "�L�����u���[�V���������B���[�U�[:" << nId << std::endl;
        capability.StartTracking(nId);
    }
    // �L�����u���[�V�������s
    else {
        std::cout << "�L�����u���[�V�������s�B���[�U�[:" << nId << std::endl;
    }
}

int main (int argc, char * argv[])
{
    try {
        cv::Ptr< IplImage > camera = 0;

        // �R���e�L�X�g�̏�����
        xn::Context context;
        XnStatus rc = context.InitFromXmlFile("SamplesConfig.xml");
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
        depth.GetAlternativeViewPointCap().SetViewPoint(image);

        // ���[�U�[�̍쐬
        xn::UserGenerator user;
        rc = context.FindExistingNode( XN_NODE_TYPE_USER, user );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // �X�P���g���E�g���b�L���O���T�|�[�g���Ă��邩�m�F
        if (!user.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
            throw std::runtime_error("���[�U�[���o���T�|�[�g���Ă܂���");
        }

        // �L�����u���[�V�����Ƀ|�[�Y���K�v
        xn::SkeletonCapability skeleton = user.GetSkeletonCap();
        if ( skeleton.NeedPoseForCalibration() ) {
            throw std::runtime_error("�ŐV��OpenNI���C���X�g�[�����Ă�������");
        }

        // ���[�U�[�F���̃R�[���o�b�N��o�^
        // �L�����u���[�V�����̃R�[���o�b�N��o�^
        XnCallbackHandle userCallbacks, calibrationCallbacks;
        user.RegisterUserCallbacks(&::UserDetected, 0, 0, userCallbacks);
        skeleton.RegisterCalibrationCallbacks( 0, &::CalibrationEnd, 0, calibrationCallbacks );

        // ���[�U�[�g���b�L���O�ŁA���ׂĂ��g���b�L���O����
        //XN_SKEL_PROFILE_ALL           ���ׂĂ��g���b�L���O����
        //XN_SKEL_PROFILE_UPPER         �㔼�g���g���b�L���O����
        //XN_SKEL_PROFILE_LOWER         �����g���g���b�L���O����
        //XN_SKEL_PROFILE_HEAD_HANDS    ���Ǝ���g���b�L���O����
        skeleton.SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

        // �J�����T�C�Y�̃C���[�W���쐬(8bit��RGB)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode(outputMode);
        camera = ::cvCreateImage(cvSize(outputMode.nXRes, outputMode.nYRes), IPL_DEPTH_8U, 3);
        if (!camera) {
            throw std::runtime_error("error : cvCreateImage");
        }

        // ���C�����[�v
        while (1) {
            // ���ׂẴm�[�h�̍X�V��҂�
            context.WaitAndUpdateAll();

            // �摜�f�[�^�̎擾
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // ���[�U�[�f�[�^�̎擾
            xn::SceneMetaData sceneMD;
            user.GetUserPixels(0, sceneMD);

            // �J�����摜�̕\��
            memcpy( camera->imageData, imageMD.Data(), camera->imageSize );

            // �X�P���g���̕`��
            XnUserID users[15];
            XnUInt16 userCount = 15;
            user.GetUsers(users, userCount);
            for (int i = 0; i < userCount; ++i) {
                if ( !skeleton.IsTracking( users[i] ) ) {
                    continue;
                }

                for ( int j = (int)XN_SKEL_HEAD; j <= (int)XN_SKEL_RIGHT_FOOT; ++j ) {
                    if ( !skeleton.IsJointAvailable( (XnSkeletonJoint)j ) ) {
                        continue;
                    }

                    // �e�ӏ��̍��W���擾����
                    XnSkeletonJointPosition joint;
                    skeleton.GetSkeletonJointPosition(users[i], (XnSkeletonJoint)j, joint);
                    if ( joint.fConfidence < 0.5 ) {
                        continue;
                    }

                    // ���W��ϊ�����
                    XnPoint3D pt = joint.position;
                    depth.ConvertRealWorldToProjective( 1, &pt, &pt );
                    cvCircle( camera, cvPoint(pt.X, pt.Y), 10, cvScalar( 255, 0, 0 ), -1 );
                }
            }

            ::cvCvtColor(camera, camera, CV_RGB2BGR);
            ::cvShowImage("KinectImage", camera);

            // �L�[�C�x���g
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

    return 0;
}
