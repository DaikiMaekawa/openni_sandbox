#include <iostream>
#include <vector>

#include <XnCppWrapper.h>
#include <mmsystem.h>

#define NUMBER_OF_AUDIO_BUFFERS 100

const char* RECORDE_PATH = "record.oni";

class AudioOutput
{
    xn::Context context;
    xn::AudioGenerator audio;
    XnWaveOutputMode waveMode;

    xn::Recorder recorder;
    xn::Player player;

    HWAVEOUT hWaveOut;
    std::vector<WAVEHDR>  AudioBuffers;

public:

    // OpenNI�̏�����
    void initOpenNI()
    {
        // �R���e�L�X�g�̏�����
        XnStatus nRetVal = context.Init();
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �f�o�C�X���쐬����(XML����̐������ƁA�f�o�C�X���Ȃ��Ƃ�����)
        nRetVal = audio.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �擾����WAVE�f�[�^�̐ݒ�
        waveMode.nSampleRate = 44100;
        waveMode.nChannels = 2;
        waveMode.nBitsPerSample = 16;
        nRetVal = audio.SetWaveOutputMode(waveMode);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // ���R�[�_�[�̍쐬
        nRetVal = recorder.Create( context );
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �o�͐�̐ݒ�
        nRetVal = recorder.SetDestination( XN_RECORD_MEDIUM_FILE, RECORDE_PATH );
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �L�^����m�[�h�̒ǉ�
        nRetVal = recorder.AddNodeToRecording(audio, XN_CODEC_UNCOMPRESSED);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �L�^�̊J�n
        nRetVal = recorder.Record();
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �f�[�^�̎擾���J�n����
        context.StartGeneratingAll();
    }

    // OpenNI�̏�����
    void initOpenNI( const std::string& recordFileName )
    {
        // �R���e�L�X�g�̏�����
        XnStatus nRetVal = context.Init();
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �L�^�����t�@�C�����J��
        nRetVal = context.OpenFileRecording( recordFileName.c_str() );
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �v���[���[���쐬����
        nRetVal = context.FindExistingNode(XN_NODE_TYPE_PLAYER, player);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �f�o�C�X���쐬����
        nRetVal = audio.Create(context);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // WAVE�f�[�^���擾����
        nRetVal = audio.GetWaveOutputMode(waveMode);
        if (nRetVal != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(nRetVal));
        }

        // �f�[�^�̎擾���J�n����
        context.StartGeneratingAll();
    }

    // WAVE�̏�����
    void initWave()
    {
        // WAVE�f�[�^�̐ݒ�
        WAVEFORMATEX wf;
        wf.wFormatTag = 0x0001; // PCM
        wf.nChannels = waveMode.nChannels;
        wf.nSamplesPerSec = waveMode.nSampleRate;
        wf.wBitsPerSample = waveMode.nBitsPerSample;
        wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;
        wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;
        MMRESULT mmRes = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, NULL, NULL, CALLBACK_NULL);
        if (mmRes != MMSYSERR_NOERROR)
        {
            throw std::runtime_error( "Warning: Failed opening wave out device. Audio will not be played!\n" );
        }

        // �����f�[�^�p�̃o�b�t�@�̍쐬�Ə�����
        AudioBuffers.resize( NUMBER_OF_AUDIO_BUFFERS );
        xnOSMemSet(&AudioBuffers[0], 0, sizeof(WAVEHDR)*AudioBuffers.size());

        const XnUInt32 nMaxBufferSize = 2 * 1024 * 1024;
        for (int i = 0; i < NUMBER_OF_AUDIO_BUFFERS; ++i)
        {
            AudioBuffers[i].lpData = new XnChar[nMaxBufferSize];
            AudioBuffers[i].dwUser = i;
            AudioBuffers[i].dwFlags = WHDR_DONE; // mark this buffer as empty (already played)
        }
    }

    // ���C�����[�v
    void run()
    {
        int nAudioNextBuffer = 0;

        printf ("Press any key to exit...\n");

        // ���̃f�[�^���̂Ă�
        audio.WaitAndUpdateData();

        while (!xnOSWasKeyboardHit()) {
            // �f�[�^�̍X�V
            XnStatus nRetVal = context.WaitAndUpdateAll();
            if (nRetVal != XN_STATUS_OK) {
                throw std::runtime_error(xnGetStatusString(nRetVal));
            }

            // �o�b�t�@�̎擾
            WAVEHDR* pHeader = &AudioBuffers[nAudioNextBuffer];
            if ((pHeader->dwFlags & WHDR_DONE) == 0) {
                printf("No audio buffer is available!. Audio buffer will be lost!\n");
                continue;
            }

            // WAVE�w�b�_�̃N���[���A�b�v
            MMRESULT mmRes = waveOutUnprepareHeader(hWaveOut, pHeader, sizeof(WAVEHDR));
            if ( mmRes != MMSYSERR_NOERROR ) {
                OutputErrorText( mmRes );
            }

            // WAVE�f�[�^�̎擾
            pHeader->dwBufferLength = audio.GetDataSize();
            pHeader->dwFlags = 0;
            xnOSMemCopy(pHeader->lpData, audio.GetAudioBuffer(), pHeader->dwBufferLength);

            // WAVE�w�b�_�̏�����
            mmRes = waveOutPrepareHeader(hWaveOut, pHeader, sizeof(WAVEHDR));
            if ( mmRes != MMSYSERR_NOERROR ) {
                OutputErrorText( mmRes );
                continue;
            }

            // WAVE�f�[�^���o�̓L���[�ɓ����
            mmRes = waveOutWrite(hWaveOut, pHeader, sizeof(WAVEHDR));
            if ( mmRes != MMSYSERR_NOERROR ) {
                OutputErrorText( mmRes );
                continue;
            }

            // ���̃o�b�t�@�C���f�b�N�X
            nAudioNextBuffer = (nAudioNextBuffer + 1) % NUMBER_OF_AUDIO_BUFFERS;
        }
    }

private:

    // �G���[���b�Z�[�W�̏o��
    void OutputErrorText( MMRESULT mmRes )
    {
        CHAR msg[250];
        waveOutGetErrorText(mmRes, msg, 250);
        std::cout << msg << std::endl;
    }
};

void main( int argc, char* argv[] )
{
    try {
        AudioOutput audio;
        if ( argc == 1 ) {
            audio.initOpenNI();
        }
        else {
            audio.initOpenNI( argv[1] );
        }

        audio.initWave();
        audio.run();
    }
    catch ( std::exception& ex ) {
        std::cout << ex.what() << std::endl;
    }
}
