
/** Signal.h会被包含多次,本地头文件不能使用宏保护*/

/**
 * @brief 信号类模版类
 * <p>
 * 一个信号被触发时，会调用所有连接到其上的TFunctionN对象,只支持返回void 的
 * TFunctionN. SIGNAL_SIGNAL是一个宏,根据参数个数会被替换为TSignalN,
 * 用户通过 TSignalN<T1,T2,...,TN>方式来使用,其中TN表示参数类型
 * @see FUNCTION_FUNCTION
 */

#define SIGNAL_SIGNAL       ARCFACE_JOIN(TSignal,SIGNAL_NUMBER)
#define FUNCTION_FUNCTION   ARCFACE_JOIN(TFunction, SIGNAL_NUMBER)

#if (SIGNAL_NUMBER != 0)
template <SIGNAL_CLASS_TYPES>
#endif
class SIGNAL_SIGNAL {
    /**
     * @brief 信号节点状态
     */
    enum SlotState {
        slotStateEmpty,		    /**节点为空*/
        slotStateNormal,	    /**节点已经链接*/
    };

public:
    /**
     * @brief 节点挂在未知
     */
    enum SlotPosition {
        any,				    /** 任意地方*/
        back,				    /** 挂载在尾部*/
        front				    /** 挂载在头部*/
    };

    /**
     * @brief 信号操作错误码
     */
    enum ErrorCode {
        errorNoFound = -1,		/** 没有找到指定对香*/
        errorExist = -2,		/** 对象已经存在*/
        errorFull = -3,			/** 已经达到可以挂载的上限*/
        errorEmptyProc = -4,	/** 对象包含的函数指针为空*/
        errorAllReuseProc = -5, /** 任意远程函数指针,没有意义*/
    };

    /**与信号模版参数类型匹配的函数指针对象类型*/
    typedef FUNCTION_FUNCTION<void SIGNAL_TYPES_COMMA > Proc;

private:
    /**信号节点结构*/
    struct SignalSlot {
        Proc proc;
        SlotState state;
        union {
            /**count == 0表示回调函数未被调用或者回调函数已经返回*/
            uint8_t count;
            uint32_t placeholder;
        };
        uint32_t cost;
    };

    int32_t             mNumber;
    int32_t	            mThreadId;
    int32_t             mNumberMax;
    SignalSlot*         mSlots;
    Infra::CMutex       mMutex;

public:
    /**
     * @brief 构造函数
     * @param[in]  maxSlots 能够连接的最大函数指针对象,默认为1
     */
    SIGNAL_SIGNAL(int32_t maxSlots = 1) :
            mNumberMax(maxSlots), mNumber(0), mThreadId(-1) {

        mSlots = new SignalSlot[maxSlots];
        for(int i = 0; i < mNumberMax; i++) {
            mSlots[i].state = slotStateEmpty;
            mSlots[i].placeholder = 0;
            mSlots[i].count = 0;
        }
    }

    /**
     * @brief 析构函数
     */
    ~SIGNAL_SIGNAL() {
        delete []mSlots;
    }

    /**
     * @brief  挂载函数指针对象
     * @param[in] proc      函数指针对象
     * @param[in] position  挂砸位置
     * @return
     */
    int32_t attach(const Proc &proc, SlotPosition position = any) {
        int32_t index = 0;
        if(proc.empty()) {
            return errorEmptyProc;
        }

        if(isAttached(proc)) {
            return errorExist;
        }

        CGuard guard(mMutex);

        switch(position) {
            case any:
                for(index = 0; index < mNumberMax; index++) {
                    if(mSlots[index].state == slotStateEmpty) {
                        mSlots[index].proc  = proc;mSlots[index].state = slotStateNormal;
                        return ++mNumber;
                    }
                }
                break;
            case back:
                for(index = mNumberMax - 1; index >= 0; index--) {
                    if(mSlots[index].state == slotStateEmpty) {
                        for(int j = index; j < mNumberMax - 1; j++) {
                            mSlots[j] = mSlots[j + 1];
                        }

                        mSlots[mNumberMax - 1].proc  = proc;mSlots[mNumberMax - 1].state = slotStateNormal;
                        return ++mNumber;
                    }
                }

                break;
            case front:
                for(index = 0; index < mNumberMax; index++) {
                    if(mSlots[index].state == slotStateEmpty) {
                        for(int j = index; j > 0; j--) {
                            mSlots[j] = mSlots[j - 1];
                        }

                        mSlots[0].proc  = proc;mSlots[0].state = slotStateNormal;
                        return ++mNumber;
                    }
                }
                break;
        }

        return errorFull;
    }

    /**
     * @brief 卸载函数指针对象,根据对象中保存的函数指针来匹配
     * @param[in] proc     函数指针对象
     * @param[in] wait     是否等待正在进行的回调结束，一般在使用者对象析构时需要
     *                     在函数回调里卸载不能等待,否则会死锁
     * @return
     */
    int32_t detach(const Proc &proc, bool wait = false) {
        if (proc.empty()) {
            return errorEmptyProc;
        }

        bool anyRemoteRemoved = false;
        CGuard guard(mMutex);

        for (int i = 0; i < mNumberMax; i++) {
            if(mSlots[i].proc == proc
               && mSlots[i].state == slotStateNormal) {

                /**回调线程和stop线程不是同一个线程,才需要等待,否则会引起自锁*/
                if(wait && mSlots[i].count && CThread::getCurrentThreadId() != mThreadId) {
                    while(mSlots[i].count) {
                        mMutex.leave();
                        CThread::sleep(10);
                        mMutex.enter();
                    }
                }

                mSlots[i].state = slotStateEmpty;
                /**移除所有复用*/
                return --mNumber;
            }
        };

        return errorNoFound;
    }

    /**
     * @brief  判断卸载函数是否挂载,根据函数指针对象匹配
     * @param[in] proc  函数指针对象
     * @return
     */
    bool isAttached(const Proc &proc) {

        CGuard guard(mMutex);
        if(proc.empty()) {
            return false;
        }

        for(int i = 0; i < mNumberMax; i++) {
            if(mSlots[i].proc == proc
               && mSlots[i].state == slotStateNormal) {
                return true;
            }
        }

        return false;
    }

    /**
     * @brief 重载(),可以以函数方式调用
     */
    inline void operator()(SIGNAL_TYPE_ARGS) {
        CGuard guard(mMutex);

        if (mNumber <= 0) {
            return;
        }

        uint64_t us1 = 0, us2 = 0;

        mThreadId = CThread::getCurrentThreadId();

        /**call back functions one by one*/
        for(int i = 0; i < mNumberMax; i++) {
            if(mSlots[i].state == slotStateNormal) {
                Proc temp = mSlots[i].proc;

                mSlots[i].count++;
                mMutex.leave();

                us1 = CTime::getCurrentMicroSecond();
                temp(SIGNAL_ARGS);
                us2 = CTime::getCurrentMicroSecond();
                mSlots[i].cost = (us1 <= us2) ? uint32_t(us2 - us1) : 1;

                mMutex.enter();
                mSlots[i].count--;
            }
        }
    }

    void stat() {

        int i;
        CGuard guard(mMutex);

        for(i = 0; i < mNumberMax; i++) {
            if(mSlots[i].state == slotStateNormal) {
                printf("\t%8d us, %p, %s\n",
                       mSlots[i].cost,
                       mSlots[i].proc.getObject(),
                       mSlots[i].proc.getObjectType());
            }
        }
    }
};

#undef SIGNAL_SIGNAL
#undef FUNCTION_FUNCTION
