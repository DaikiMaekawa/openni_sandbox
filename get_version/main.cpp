#include <iostream>
#include <XnCppWrapper.h>

void main()
{
    XnVersion version;
    ::xnGetVersion( &version );
    std::cout << (int)version.nMajor << "." << (int)version.nMinor << "." <<
        version.nMaintenance << "." << version.nBuild << std::endl;

    xn::Version base( 1, 4, 0, 2 );
    if ( base > xn::Version( version ) ) {
        std::cout << "���̃o�[�W�����ł́A�|�[�Y�Ȃ��L�����u���[�V�����͂ł��܂���" << std::endl;
    }

    if ( base == xn::Version( version ) ) {
        std::cout << "���̃o�[�W��������A�|�[�Y�Ȃ��L�����u���[�V�������ł��܂�" << std::endl;
    }

    if ( base < xn::Version( version ) ) {
        std::cout << "���̃o�[�W�����́A�|�[�Y�Ȃ��L�����u���[�V�������ł��܂�" << std::endl;
    }
}
