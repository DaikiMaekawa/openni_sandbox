#include <gtest\gtest.h>
#include <gmock\gmock.h>

#include "openni\Player.h"

namespace {
    const std::string RECORD_FILE_NAME = "EndOfFileReached.oni";

    // �A�v���P�[�V�����N���X
    class App : public ::openni::PlayerCallback
    {
    public:

        App()
        {
            // �v���[���[�̍쐬�Ɛݒ�
            context.Init();
            context.OpenFileRecording( RECORD_FILE_NAME.c_str() );
            context.FindExistingNode( XN_NODE_TYPE_PLAYER, player.GetPlayer() );
            player.RegisterCallback( this );
            player.GetPlayer().SetRepeat( false );
        }

        void Run()
        {
            // EOF�܂ŉ�
            while ( !player.GetPlayer().IsEOF() ) {
                context.WaitAndUpdateAll();
            }
        }

        // �t�@�C���̏I�[
        virtual void EndOfFileReached( xn::ProductionNode& node )
        {
            std::cout << "�t�@�C���̏I�[" << std::endl;
        }

    private:

        xn::Context context;
        openni::Player player;
    };

    // �A�v���P�[�V�����N���X�̃��b�N
    class MockApp : public App
    {
    public:

        MOCK_METHOD1( EndOfFileReached, void( xn::ProductionNode& node ) );
    };
}

TEST( PlayerTest, EndOfFileReached )
{
    using ::testing::_;

    MockApp app;

    // ���b�N�̐ݒ�
    EXPECT_CALL( app, EndOfFileReached(_) )
        .Times( 1 );        // �t�@�C���̏I�[���o��1��Ăяo�����
//        .Times( 2 );      // 2���NG

    app.Run();
}
