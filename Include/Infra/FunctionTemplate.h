

#define FUNCTION_FUNCTION   UFACE_JOIN(TFunction,FUNCTION_NUMBER)

/**
 * @brief 函数指针类模版
 * 支持普通函数指针和成员函数指针的保存、比较、调用等操作;
 * 对于成员函数,要求类只能是普通类或者单继承类,不能是多继承或者虚继承的类;
 * FUNCTION_FUNCTION 是一个宏,根据参数个数会替换为TFunctionN,通过
 * TFunctionN<R,T1,T2,...,TN>的方式使用,其中 R 为返回值,TN为参数类型类型,
 * N 为参数个数，当前最大支持6个参数,使用示例如下:
 *
 * begin code
 * int g(int x) {
 * 	 return x * 2;
 * }
 * class G {
 * public:
 * 	int g(int x)
 * 	{
 * 		return x * 3;
 * 	}
 * }gg;
 * void test()
 * {
 * 	TFunction1<int, int> f1(g);
 * 	TFunction1<int, int> f2(&G::g, &gg);
 * 	assert(f1(1) = 2);
 * 	assert(f2(1) = 3);
 * 	f1 = f2;
 * 	assert(f1 == f2);
 * }
 * end code
 */



/** Comma if nonzero number of arguments*/
#if FUNCTION_NUMBER == 0
#define FUNCTION_COMMA
#else
#define FUNCTION_COMMA ,
#endif // FUNCTION_NUMBER > 0

#define UFACE_JOIN(X,Y)         UFACE_DO_JOIN(X,Y)
#define UFACE_DO_JOIN(X,Y)      UFACE_DO_JOIN2(X,Y)
#define UFACE_DO_JOIN2(X,Y)     X##Y

#define FUNCTION_INVOKER            UFACE_JOIN(function_invoker, FUNCTION_NUMBER)
#define FUNCTION_VOID_INVOKER       UFACE_JOIN(function_void_invoker, FUNCTION_NUMBER)
#define GET_FUNCTION_INVOKER        UFACE_JOIN(get_function_invoker, FUNCTION_NUMBER)
#define MEM_FUNCTION_INVOKER        UFACE_JOIN(mem_function_invoker, FUNCTION_NUMBER)
#define MEM_FUNCTION_VOID_INVOKER   UFACE_JOIN(mem_function_void_invoker, FUNCTION_NUMBER)
#define GET_MEM_FUNCTION_INVOKER    UFACE_JOIN(get_mem_function_invoker, FUNCTION_NUMBER)

//using namespace std;

template<FUNCTION_CLASS_TYPES>
struct FUNCTION_INVOKER {
    template<class F>
    static typename Detail::FunctionReturnType<R>::type invoke(F f FUNCTION_COMMA FUNCTION_TYPE_ARGS) {
        return f(FUNCTION_ARGS);
    }
};

template<FUNCTION_CLASS_TYPES>
struct FUNCTION_VOID_INVOKER {
    template<class F>
    static typename Detail::FunctionReturnType<R>::type invoke(F f FUNCTION_COMMA FUNCTION_TYPE_ARGS) {
        UNI_UBI_FUNCTION_RETURN(f(FUNCTION_ARGS));
    }
};

template<class RT>
struct GET_FUNCTION_INVOKER {
    template<FUNCTION_CLASS_TYPES>
    struct Invoker {
        typedef FUNCTION_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
    };
};

template<>
struct GET_FUNCTION_INVOKER<void> {
    template<FUNCTION_CLASS_TYPES>
    struct Invoker {
        typedef FUNCTION_VOID_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
    };
};

////////////////////////////////////////////////////////////////////////////////

template<FUNCTION_CLASS_TYPES>
struct MEM_FUNCTION_INVOKER {
    template<class O, class F>
    static typename Detail::FunctionReturnType<R>::type invoke(O o, F f FUNCTION_COMMA FUNCTION_TYPE_ARGS) {
        return (o->*f)(FUNCTION_ARGS);
    }
};

template<FUNCTION_CLASS_TYPES>
struct MEM_FUNCTION_VOID_INVOKER {
    template<class O, class F>
    static typename Detail::FunctionReturnType<R>::type invoke(O o, F f FUNCTION_COMMA FUNCTION_TYPE_ARGS) {
        UNI_UBI_FUNCTION_RETURN((o->*f)(FUNCTION_ARGS));
    }
};

template<class RT>
struct GET_MEM_FUNCTION_INVOKER {
    template<FUNCTION_CLASS_TYPES>
    struct Invoker {
        typedef MEM_FUNCTION_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
    };
};

template<>
struct GET_MEM_FUNCTION_INVOKER<void> {
    template<FUNCTION_CLASS_TYPES>
    struct Invoker {
        typedef MEM_FUNCTION_VOID_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
    };
};

////////////////////////////////////////////////////////////////////////////////


template <FUNCTION_CLASS_TYPES>
class FUNCTION_FUNCTION {

    class X{};

    typedef R (X::*MEM_FUNCTION)(FUNCTION_TYPES);
    typedef R (*PTR_FUNCTION)(FUNCTION_TYPES);

public:
    enum FunctionType {
        typeEmpty,
        typeMember,
        typePointer
    };

    union {
        struct {
            MEM_FUNCTION proc;
            X* obj;
        } memFunction;

        PTR_FUNCTION ptrFunction;
    } m_function;

    FunctionType mType;
    const char* mObjectType;

public:
    /**
     * @brief 默认构造函数
     */
    FUNCTION_FUNCTION() : mType(typeEmpty), mObjectType(0){
    }

    /**
     * @brief   接收成员函数指针的构造函数；将类的成员和类对象的指针绑定保存
     * @param[in]   f 类的成员函数
     * @param[in]   o 类对象指针
     */
    template<typename O>
    FUNCTION_FUNCTION(R(O::*f)(FUNCTION_TYPES), const O * o) {
        mType = typeMember;
        m_function.memFunction.proc = Detail::horrible_cast<MEM_FUNCTION>(f);
        m_function.memFunction.obj = Detail::horrible_cast<X*>(o);
        mObjectType = typeid(o).name();
    }

    /**
     * @brief  接收普通函数指针的构造函数；保存普通函数指针
     * @param[in]   f 函数指针
     */
    FUNCTION_FUNCTION(PTR_FUNCTION f) {
        if(f) {
            mType = typePointer;
            m_function.ptrFunction = f;
            mObjectType = 0;
        } else {
            mType = typeEmpty;
            mObjectType = 0;
        }
    }

    /**
     * @brief  复制函数
     * @param[in]  f  源函数指针类
     */
    FUNCTION_FUNCTION& operator=(const FUNCTION_FUNCTION& f) {
        if (&f == this) {
            return *this;
        }

        m_function = f.m_function;
        mType = f.mType;
        mObjectType = f.mObjectType;
        return *this;
    }

    /**
     * @brief 判断函数指针是否为空
     * @return
     */
    bool empty() const {
        return (mType == typeEmpty);
    }

    /**
     * @brief 比较两个函数指针
     * @return
     */
    bool operator==(const FUNCTION_FUNCTION& f) const {

        if(mType != f.mType) {
            return false;
        }

        if(mType == typeMember) {
            return (m_function.memFunction.proc == f.m_function.memFunction.proc
                        && m_function.memFunction.obj == f.m_function.memFunction.obj);
        } else if(mType == typePointer) {
            return (m_function.ptrFunction == f.m_function.ptrFunction);
        } else {
            return true;
        }
    }

    /**
     * @brief 重载() 运算符,可以以函数调用方式调用函数指针
     * @return
     */
    inline typename Detail::FunctionReturnType<R>::type operator()(FUNCTION_TYPE_ARGS) {
        if(mType == typeMember) {
            typedef typename GET_MEM_FUNCTION_INVOKER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
            return Invoker::invoke(m_function.memFunction.obj, m_function.memFunction.proc FUNCTION_COMMA FUNCTION_ARGS);
        } else if(mType == typePointer) {
            typedef typename GET_FUNCTION_INVOKER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
            return Invoker::invoke(m_function.ptrFunction FUNCTION_COMMA FUNCTION_ARGS);
        } else {
            printf("FUNCTION_FUNCTION::operator() invalid function type : %u, typeid : %s\n", mType, mObjectType);
            typedef typename GET_MEM_FUNCTION_INVOKER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
            return Invoker::invoke(m_function.memFunction.obj, m_function.memFunction.proc FUNCTION_COMMA FUNCTION_ARGS);
        }
        
    }

    void * getObject() {
        return m_function.memFunction.obj;
    }

    const char* getObjectType() {
        return mObjectType;
    }
};

#undef FUNCTION_FUNCTION
#undef FUNCTION_COMMA
#undef FUNCTION_INVOKER
#undef FUNCTION_VOID_INVOKER
#undef GET_FUNCTION_INVOKER
#undef MEM_FUNCTION_INVOKER
#undef MEM_FUNCTION_VOID_INVOKER
#undef GET_MEM_FUNCTION_INVOKER