#ifndef Thread_H_INCLUDE
#define Thread_H_INCLUDE

#include <XnOS.h>
#include <map>
#include <memory>
#include <boost/utility.hpp>

namespace std {
	using namespace std::tr1;
}

// xn extend
namespace xne {
	class Thread : public boost::noncopyable
	{
	public:

		// �R���X�g���N�^
		Thread();

		/**
		* �R���X�g���N�^
		* @param pr �R�[���o�b�N���������I�u�W�F�N�g(�֐��|�C���^�A�֐��I�u�W�F�N�g��)
		* @note
		*  pr�� int pr( Thread* thread ) �̌`�Ő錾���邱�Ɗ֐����͉��ł��悢
		*/
		template< typename Pred >
		Thread(Pred pr)
			: pr_( new ThreadProc< Pred >( pr ) )
			, thread_()
		{
			Create( lpThreadAttributes ,dwStackSize );
		}

		/**
		* �R���X�g���N�^
		* @param fn �R�[���o�b�N�������������o�֐��|�C���^
		* @param p �R�[���o�b�N�������������o�֐��|�C���^�����I�u�W�F�N�g�ւ̃|�C���^
		* @note
		*  fn�� int T::fn( Thread* thread ) �̌`�Ő錾���邱�ƃ����o�֐����͉��ł��悢
		*/
		template< class T >
		Thread( int (T::*fn)(xne::Thread* thread ), T* p)
			: pr_( CreateThread( std::bind1st( std::mem_fun1( fn ), p ) ) )
			, thread_()
		{
			Create();
		}

		// �f�X�g���N�^
		~Thread();

		/**
		* �X���b�h�̍쐬
		*
		* @param pr �R�[���o�b�N���������I�u�W�F�N�g(�֐��|�C���^�A�֐��I�u�W�F�N�g��)
		* @param lpThreadAttributes �Z�L�����e�B�L�q�q
		* @param dwStackSize �X���b�h�̃X�^�b�N�T�C�Y
		*/
		template<typename Pred>
		void Create(Pred pr)
		{
			pr_ = ThreadLocalEntry(CreateThread(pr));

			Create();
		}

		/**
		* �X���b�h�̍쐬
		*
		* @param fn �R�[���o�b�N�������������o�֐��|�C���^
		* @param p �R�[���o�b�N�������������o�֐��|�C���^�����I�u�W�F�N�g�ւ̃|�C���^
		* @param lpThreadAttributes �Z�L�����e�B�L�q�q
		* @param dwStackSize �X���b�h�̃X�^�b�N�T�C�Y
		*/
		template< class T >
		void Create( int (T::*fn)( xne::Thread* thread ), T* p, LPSECURITY_ATTRIBUTES lpThreadAttributes = 0, DWORD dwStackSize = 0 )
		{
			pr_ = ThreadLocalEntry(CreateThread(std::bind1st(std::mem_fun1(fn), p)));

			Create();
		}

		// �I����҂�
		void Wait(XnUInt32 nMilliseconds = XN_WAIT_INFINITE);

		// �X���b�hID���擾����
		static XN_THREAD_ID GetCurrentID();

	private:

		// �X���b�h�̍쐬
		void Create();

		// ���ʂ̃X���b�h�̃G���g���|�C���g
		static XN_THREAD_PROC ThreadEntry(XN_THREAD_PARAM);

		// �ŗL�̃X���b�h�̃G���g���|�C���g
		int Run();

	private:

		/**
		* @brief �X���b�h�֐��C���^�[�t�F�C�X
		* @note
		*  �ʂ̃X���b�h�G���g���|�C���g�̃x�[�X�C���^�[�t�F�C�X
		*  ����邱�ƂŁAtemplate< typename Pred > class ThreadProc
		*  �ɂǂ�ȃe���v���[�g�������^�����Ă�ThreadProcBase�ŏ���
		*  �ł���悤�ɂȂ�B
		*/
		class ThreadProcBase
		{
		public:

			/// �f�X�g���N�^
			virtual ~ThreadProcBase(){};

			/// �G���g���|�C���g
			virtual int Run(Thread* thread) = 0;
		};

		/// �X���b�h�֐�
		template< typename Pred >
		class ThreadProc : public ThreadProcBase
		{
		public:

			/**
			* �R���X�g���N�^
			* @param pr �R�[���o�b�N���������I�u�W�F�N�g(�֐��|�C���^�A�֐��I�u�W�F�N�g��)
			*/
			ThreadProc(Pred pr) : pr_(pr){}

			/**
			* �f�X�g���N�^
			*/
			~ThreadProc(){};

			/**
			* �X���b�h�̃G���g���|�C���g
			* @param thread �R�[���o�b�N�����X���b�h�I�u�W�F�N�g�ւ̃|�C���^
			* @return �X���b�h�̖߂�l
			*/
			int Run(Thread* thread){ return pr_(thread); }

		private:

			Pred     pr_;
		};

		/**
		* �X���b�h�֐��I�u�W�F�N�g�����֐�
		* @param pr �R�[���o�b�N���������I�u�W�F�N�g(�֐��|�C���^�A�֐��I�u�W�F�N�g��)
		* @return �X���b�h�֐��I�u�W�F�N�g
		*/
		template<typename Pred>
		static ThreadProc<Pred>* CreateThread(Pred pr)
		{
			return new ThreadProc<Pred>(pr);
		}

	private:

		typedef std::shared_ptr<ThreadProcBase>   ThreadLocalEntry;
		typedef std::map<XN_THREAD_ID, Thread*>   ThreadList;

		XN_THREAD_HANDLE   thread_;    ///< �X���b�h�n���h��
		ThreadLocalEntry   pr_;        ///< �G���g���|�C���g
		static ThreadList  threads_;   ///< �X���b�h�ꗗ
	};
}

#endif // #ifndef Thread_H_INCLUDE
