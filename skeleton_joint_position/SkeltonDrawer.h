#ifndef SKELTONDRAWER_H_INCLUDE
#define SKELTONDRAWER_H_INCLUDE

#include <opencv/cv.h>

#include <XnCppWrapper.h>

class SkeltonDrawer
{
public:
    SkeltonDrawer( IplImage* camera, xn::SkeletonCapability& skelton,
        xn::DepthGenerator& depth, XnUserID player )
        :camera_(camera), skelton_(skelton), depth_(depth), player_(player)
    {
    }

    // �X�P���g����`�悷��
    void draw()
    {
        if (!skelton_.IsTracking(player_)) {
            throw std::runtime_error("�g���b�L���O����Ă��܂���");
        }

        for ( int i = XN_SKEL_HEAD; i <= XN_SKEL_RIGHT_FOOT; ++i ) {
            std::cout << "[" << i << "]:" << (skelton_.IsJointActive( (XnSkeletonJoint)i ) ? "TRUE" : "FALSE") << std::endl;
            if ( skelton_.IsJointActive( (XnSkeletonJoint)i ) ) {
                drawPoint( (XnSkeletonJoint)i );
            }
        }
    }

private:
    // �X�P���g���̐���`�悷��
    void drawLine(XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
    {
        // �e�ӏ��̍��W���擾����
        XnSkeletonJointPosition joint1, joint2;
        skelton_.GetSkeletonJointPosition(player_, eJoint1, joint1);
        skelton_.GetSkeletonJointPosition(player_, eJoint2, joint2);
        if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5) {
            return;
        }

        // ���W��ϊ�����
        XnPoint3D pt[2] = { joint1.position, joint2.position };
        depth_.ConvertRealWorldToProjective(2, pt, pt);
        cvLine(camera_,cvPoint(pt[0].X, pt[0].Y), cvPoint(pt[1].X, pt[1].Y),
            CV_RGB(0, 0, 0),2,CV_AA ,0);
    }

    // �X�P���g���̓_��`�悷��
    void drawPoint( XnSkeletonJoint eJoint )
    {
        // �e�ӏ��̍��W���擾����
        XnSkeletonJointPosition joint;
        skelton_.GetSkeletonJointPosition(player_, eJoint, joint);
        if ( joint.fConfidence < 0.5 ) {
            return;
        }

        // ���W��ϊ�����
        XnPoint3D pt = joint.position;
        depth_.ConvertRealWorldToProjective( 1, &pt, &pt );
        cvCircle( camera_, cvPoint(pt.X, pt.Y), 10, cvScalar( 0, 0, 255 ), -1 );
    }

private:

    IplImage* camera_;
    xn::SkeletonCapability& skelton_;
    xn::DepthGenerator& depth_;
    XnUserID player_;
};

#endif // #ifndef SKELTONDRAWER_H_INCLUDE
