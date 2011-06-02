#include "Thread.h"

namespace xne {
	// �X���b�hID��this�|�C���^�̊֘A�t��
	Thread::ThreadList  Thread::threads_;

	// �R���X�g���N�^
	Thread::Thread()
		: pr_()
		, thread_()
	{
	}

	// �f�X�g���N�^
	Thread::~Thread()
	{
		Wait();
	}

	// �X���b�h�̍쐬
	void Thread::Create()
	{
		XnStatus rc = ::xnOSCreateThread(&Thread::ThreadEntry, 0, &thread_);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(::xnGetStatusString(rc));
		}

		threads_[GetCurrentID()] = this;
	}

	// �I����҂�
	void Thread::Wait(XnUInt32 nMilliseconds /*= XN_WAIT_INFINITE*/)
	{
		XnStatus rc = ::xnOSWaitForThreadExit(thread_, nMilliseconds);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(::xnGetStatusString(rc));
		}
	}

	//�X���b�hID���擾����
	/*static*/ XN_THREAD_ID Thread::GetCurrentID()
	{
		XN_THREAD_ID threadId = 0;
		XnStatus rc = ::xnOSGetCurrentThreadID(&threadId);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(::xnGetStatusString(rc));
		}

		return threadId;
	}

	// ���ʂ̃X���b�h�̃G���g���|�C���g
	/*static*/ XN_THREAD_PROC Thread::ThreadEntry(XN_THREAD_PARAM)
	{
		Thread* pThread = threads_[GetCurrentID()];
		XN_ASSERT( pThread != 0 );
		XN_THREAD_PROC_RETURN(pThread->Run());
	}

	// �X���b�h�̃G���g���|�C���g
	int Thread::Run()
	{
		return pr_->Run(this);
	}
}
// EOF
