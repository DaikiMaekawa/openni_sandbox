//#include <gtest\gtest.h>
//#include <gmock\gmock.h>
//
//#include "../App.h"
//
//const char* CONFIG_XML_PATH = "SamplesConfig.xml";
//const char* RECORDE_PATH = "record.oni";
//
//class MockApp : public App
//{
//public:
//
//    MOCK_METHOD1( EndOfFileReached, void( xn::ProductionNode& node ) );
//};
//
//TEST( PlayerTest, EofCallback )
//{
//    using ::testing::AtLeast; 
//    using ::testing::_;
//
//    MockApp app;
//
//    EXPECT_CALL( app, EndOfFileReached(_) )
//        .Times( 1 );        // �t�@�C���̏I�[���o��1��Ăяo�����
////        .Times( 2 );      // 2���NG
//
//    app.InitFromRecord( "EofCallback.oni" );
//    app.Run();
//}
