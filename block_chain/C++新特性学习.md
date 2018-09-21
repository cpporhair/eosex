# C++11,14,17新特性学习

* [模板元编程](##模板元编程)
    * [概念](###模板元编程概念)
* [折叠表达](##折叠表达式)

## 模板元编程



模板元编程概念：利用模板特化机制实现编译期条件选择结构，利用递归模板实现编译期循环结构，模板元编程则由编译器在编译器解释执行。
优势在于：1.以编译耗时为代价换来卓越的运行时性能，2.提供编译期类型计算。
劣势在于：1.代码可读性差，2.调试困难，3.编译时间长，4.可移植性差（对编译器来说）
模板元程序由元数据和元函数组成，元数据就是元编程可以操作的数据，即C++编译器在编译期可以操作的数据。元数据不是运行时变量，只能是编译期常量，不能修改，常见的元数据包括enum，静态常量，基本类型和自定义类型等。元函数是模板元编程中用于操作处理元数据的“构件”，可以在编译期被调用，因为它的功能和形式和运行时的函数类似，因而被成为元函数。元函数实际上表现为C++的一个类、模板类或模板函数。例如：

    ```cpp
    template<int N,int M>
    struct meta_func {
        enum {
            value = N+M
        };
    }
    ```

调用元函数获取value值：std::cout << meta_func<1,2>::value << std::endl;
meta_func的执行过程是在编译期完成的，实际执行时是没有计算动作，而是直接使用编译期的计算结果的。元函数只处理元数据，元数据是编译期常量和类型。
因此模板元编程不能使用运行时的关键字，常用的编译期关键字如下：
1.enum,static const,用来定义编译期的整数常量；
2.typedef/using,用于定义元数据；
3.T、Types...,声明元数据类型；
4.template,主要用于定义元函数；
5.“::”，域位运算，用于解析类型作用域获取计算结果（元数据）
模板元编程中的条件判断，是通过type_traits来实现的，它不仅仅可以在编译期做判断，还可以做计算、查询、转换和选择。
模板元中的for等逻辑可以通过递归、重载、和模板特化（偏特化）等方法实现。


类型萃取关键词： 
1.decltype 计算表达式的返回类型，并不执行表达式的计算。类似的还有std::result_of

2.编译期选择 std::conditional,它在编译期根据一个判断式选择连个类型中的一个，类似于运行时的三元表达式“？：”，用法如下：

    std::conditional<true,int,float>::type 此时type的类型为int
    std::conditional<false,int,float>::type 此时type为float

3.std::decay(退化)，它对于普通类型来说是移除引用和cv符（const,volatile）。除了普通类型外，它还可以用于数组和函数，具体转化规则如下：
    1.若T为“U的数组”或“U的数组的引用”，则成员 typedef type U*
    2.若T为函数类型F或函数的引用，则成员typedef type 为 函数指针
    3.否则，成员typedef type 定义为 std::remove_cv<std::remove_reference<T>>::type，即普通类型对应的转化规则移除引用和cv符。
4.std::enable_if来实现编译期的if-else逻辑,它利用SFINAE(substitude failure is not an error)特性，根据条   件选择重载函数的元函数std::enable_if，它的原型是：
  template<bool B, class T = void> struct enable_if;
  根据enable_if的字面意思就可以知道，它使得函数在判断条件B仅仅为true时才有效，它的基本用法：
  
    template <class T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type foo(T t)
    {
        return t;
    }
    auto r = foo(1); //返回整数1
    auto r1 = foo(1.2); //返回浮点数1.2
    auto r2 = foo(“test”); //compile error
   可以通过enable_if来实现编译期的if-else逻辑，比如下面的例子通过enable_if和条件判断式来将入参分为两大类，从而满足所有的入参类型：

   template <class T>
    typename std::enable_if<std::is_arithmetic<T>::value, int>::type foo1(T t)  
    {  
        cout << t << endl;  
        return 0;  
    }

    template <class T>
    typename std::enable_if<!std::is_arithmetic<T>::value, int>::type foo1(T &t)
    {
        cout << typeid(T).name() << endl;
        return 1;
    }
    对于arithmetic类型的入参则返回0，对于非arithmetic的类型则返回1，通过arithmetic将所有的入参类型分成了两大类进行处理。从上面的例子还可以看到，std::enable_if可以实现强大的重载机制，因为通常必须是参数不同才能重载，如果只有返回值不同是不能重载的，而在上面的例子中，返回类型相同的函数都可以重载。

可变模板参数的应用
    template<typename... T>void f(T... args);
上面的定义中，省略号的作用：
1.声明一个参数包T... args,这个参数包中可以包含0到任意个模板参数
2...args参数包可以展开为一个个独立的参数
展开可变模板参数函数的方法有两种：一种是通过递归函数来展开，另外一种是通过逗号表达式来展开参数包，如下：
1.递归式：
    需要一个递归终止函数，如：
    void print()
    template<typename T,class ...Types>
    void print(T first,Types... args) {
        std::cout << first << std::endl;
        print(args...);
    }
    print(1,2,3,4);
    递归调用的过程为：
    print(1,2,3,4);
    print(2,3,4);
    print(3,4);
    print(4);
    //print();
    递归终止函数还可以写成这样：
    template<typename T>
    void print(T v) {
        std::cout << v << std::endl;
    }
     void print(T first,Types... args) {
        print(first);
        print(args...);
    }
    递归调用的过程为：
    print(1,2,3,4);
    print(2,3,4);
    print(3,4);
    print(4);
2.逗号表达式展开：
    template <class T>
    void printarg(T t)
    {
        cout << t << endl;
    }

    template <class ...Args>
    void expand(Args... args)
    {
        int arr[] = {(printarg(args), 0)...};
    }

    expand(1,2,3,4);
    同时还利用了初始化列表的技术。还可以利用lambda表达式：
    template<class F, class... Args>void expand(const F& f, Args&&...args) 
    {
        //这里用到了完美转发。
        initializer_list<int>{(f(std::forward< Args>(args)),0)...};
    }
    expand([](int i){cout<<i<<endl;}, 1,2,3);
3.折叠表达式：
    就1中的例子来说，print函数可以改成这样：
    template<typename... Types>
    void print(Types const&... args) {
        (std::cout << ... << args) << '\n';
    }

可变参数模板类：
可变参数模板类是一个带可变模板参数的模板类，如标准库中的std::tuple。可以能的定义如下：
template<typename... Types>
class tuple;
这个可变参数模板类可以携带任意类型任意个数的模板参数:
    std::tuple<int> tp1 = std::make_tuple(1);
    std::tuple<int, double> tp2 = std::make_tuple(1, 2.5);
    std::tuple<int, double, string> tp3 = std::make_tuple(1, 2.5, “”);
可变参数模板的模板参数个数也可以为0，所以下面的定义也是合法的：
    std::tuple<> tp;
可变参数模板类的参数包展开的方式和可变参数模板函数的展开方式不同，可变参数模板类的参数包展开需要通过模板特化和继承方式去展开，展开方式比可变参数模板函数要复杂。如下：
1.模板篇特化和递归方式展开参数包
可变参数模板类的展开一般需要定义两到三个类，包括类声明和偏特化的模板类：
    //前置声明，表明这是一个可变参数模板类
    template<typename... Args>
    struct Sum;

    //基本定义
    template<typename First, typename... Rest>
    struct Sum<First, Rest...>
    {
        enum { value = Sum<First>::value + Sum<Rest...>::value };
    };

    //递归终止特化类，通过这个特化类来终止递归
    template<typename Last>
    struct Sum<Last>
    {
        enum { value = sizeof (Last) };
    };
    上面的前置声明可以包含任意个数的参数，下面的声明就要求模板参数至少要有一个：
    template<typename First, typename... Args>struct sum;他的展开方式为：

    template<typename First, typename... Args>struct sum；//前置声明，可以省略
    template<typename First, typename... Rest>//定义
    struct Sum
    {
        enum { value = Sum<First>::value + Sum<Rest...>::value };
    };

    template<typename Last>//递归终止定义
    struct Sum<Last>
    {
        enum{ value = sizeof(Last) };
    };
递归终止模板类写法有多个：
    template<typename... Args> struct sum;
    template<typename First, typenameLast>
    struct sum<First, Last>
    { 
        enum{ value = sizeof(First) +sizeof(Last) };
    };
    在展开到两个参数时终止。还可以展开到0个参数时终止：
    template<>struct sum<> { enum{ value = 0 }; };
还可以使用std::integral_constant来消除枚举定义value。利用std::integral_constant可以获得编译期常量的特性，可以将前面的sum例子改为这样：
    //前置声明
    template<typename First, typename... Args>
    struct Sum;

    //基本定义
    template<typename First, typename... Rest>
    struct Sum<First, Rest...> : std::integral_constant<int, Sum<First>::value + Sum<Rest...>::value>
    {
    };

    //递归终止
    template<typename Last>
    struct Sum<Last> : std::integral_constant<int, sizeof(Last)>
    {
    };
    sum<int,double,short>::value;//值为14

    template<>
    struct Sum<> {
        return 0;
    }

2.继承方式展开参数包：
    //整型序列的定义
    template<int...>
    struct IndexSeq{};

    //继承方式，开始展开参数包
    template<int N, int... Indexes>
    struct MakeIndexes : MakeIndexes<N - 1, N - 1, Indexes...> {};

    // 模板特化，终止展开参数包的条件
    template<int... Indexes>
    struct MakeIndexes<0, Indexes...>
    {
        typedefIndexSeq<Indexes...> type;
    };
    

折叠表达式：
1.left fold expression:
    (...op pack) =====> (((pack1 op pack2) op pack3) ...op packN)
2.right fold expression:
    (pack op...) =====> (pack op (...(packN-1 op packN-2)))
3.left init fold expression:
    (init op ...op pack) =====> ((( init op pack1 ) op pack2 ) ... op packN )
4.right init fold expression
    (pack op ...op init) =====> ( pack1 op ( ... ( packN op init )))

右值引用和移到语义：
    右值引用相关概念：右值，纯右值，将亡值，universal references,引用折叠，移动语义和完美转发。
    int i = getVar();
    以上面代码为例，从函数getVar()获取一个整形值，然而这会产生几种类型的值呢？答案是会产生两种类型的值，一种是左值，一种是函数返回的临时的值，这个临时的值在表达式结束后就销毁了，而左值在表达式结束后依然存在（直到其作用域结束），这个临时的值就是右值，具体来说是一个纯右值，右值是不具名的。区分左值和右值的一个简单办法是：看能不能对表达式取地址，如果能，则为左值，否则为右值。
    所有具名变量或对象都是左值，而匿名变量则是右值。如 int i = 0;这条语句，i是左值，0是字面量为右值。具体来说0是纯右值（prvalue）,在C++11以上中所有的值必属于左值，将亡值，纯右值三者之一。比如，非引用返回的临时变量、运算表达式产生的临时变量、原始字面量和lambda表达式等都是纯右值。而将亡值是C++11新增的、与右值引用相关的表达式，比如，将要被移动的对象、T&&函数返回值、std::move返回值和转换为T&&的类型的转换函数的返回值等。关于将亡值我们会在后面介绍，先看下面的代码：
        int j = 5;
        auto f = []{return 5;};
    上面的代码中5是一个原始字面量， []{return 5;}是一个lambda表达式，都是属于纯右值，他们的特点是在表达式结束之后就销毁了。

    在看下面这行代码：
        T&& k = getVar();和 int i = getVar();很像，只是比后者多了“&&”，这个声明就是右值引用。我们知道左值引用是对左值的引用，对应的，右值引用就是对右值的引用，而且右值是匿名变量，我们也只能通过应用的方式来获得右值。
        这里，getVar()产生的临时值不会像后者那样，在表达是结束后就销毁了，而是被续命，他的生命周期将会通过右值引用得以延续，和变量k的声明周期一样长。
        右值引用的一个特点：通过右值引用的声明，右值又重获新生，其生命周期与右值引用类型变量的生命周期一样的长。
        右值引用的第二个特点：右值引用独立于左值和右值。即右值引用类型的变量可能是左值也可能是右值，比如：int&& var1 = 1;
        var1类型为右值引用，但var1本身是左值，因为具名变量都是左值。

        关于右值引用一个有意思的问题是：T&&是什么，一定是右值吗？如例：
        template<typename T>
        void f(T&& t){}
        f(10); //t是右值
        int x = 10;
        f(x); //t是左值
        从上面的代码中可以看到，T&&表示的值类型不确定，可能是左值又可能是右值，这一点看起来有点奇怪，这就是右值引用的一个特点。
        右值引用的第三个特点：T&& t在发生自动类型推断的时候，它是未定的引用类型（universal references），如果被一个左值初始化，它就是一个左值；如果它被一个右值初始化，它就是一个右值，它是左值还是右值取决于它的初始化。
        我们再回过头看上面的代码，对于函数template<typename T>void f(T&& t)，当参数为右值10的时候，根据universal references的特点，t被一个右值初始化，那么t就是右值；当参数为左值x时，t被一个左值引用初始化，那么t就是一个左值。需要注意的是，仅仅是当发生自动类型推导（如函数模板的类型自动推导，或auto关键字）的时候，T&&才是universal references。再看看下面的例子：
        template<typename T>
        void f(T&& param); 

        template<typename T>
        class Test {
            Test(Test&& rhs); 
        };

        上面的例子中，param是universal reference，rhs是Test&&右值引用，因为模版函数f发生了类型推断，而Test&&并没有发生类型推导，因为Test&&是确定的类型了。
        　　正是因为右值引用可能是左值也可能是右值，依赖于初始化，并不是一下子就确定的特点，我们可以利用这一点做很多文章，比如后面要介绍的移动语义和完美转发。
        　　这里再提一下引用折叠，正是因为引入了右值引用，所以可能存在左值引用与右值引用和右值引用与右值引用的折叠，C++11确定了引用折叠的规则，规则是这样的：
        所有的右值引用叠加到右值引用上仍然还是一个右值引用；
        所有的其他引用类型之间的叠加都将变成左值引用。
        考虑下面的代码：
        T(T&& a) : m_val(val){ a.m_val=nullptr; }
        这行代码实际上来自于一个类的构造函数，构造函数的一个参数是一个右值引用，为什么将右值引用作为构造函数的参数呢？在解答这个问题之前我们先看一个例子。如下：
        class A
        {
        public:
            A():m_ptr(new int(0)){cout << "construct" << endl;}
            A(const A& a):m_ptr(new int(*a.m_ptr)) //深拷贝的拷贝构造函数
            {
                cout << "copy construct" << endl;
            }
            ~A(){ delete m_ptr;}
        private:
            int* m_ptr;
        };
        A getA() {
            return A;
        }
        int main() {
            A a = GetA();
            return 0;
        }
            输出：
        construct
        copy construct
        copy construct
        这个例子很简单，一个带有堆内存的类，必须提供一个深拷贝拷贝构造函数，因为默认的拷贝构造函数是浅拷贝，会发生“指针悬挂”的问题。如果不提供深拷贝的拷贝构造函数，上面的测试代码将会发生错误（编译选项-fno-elide-constructors），内部的m_ptr将会被删除两次，一次是临时右值析构的时候删除一次，第二次外面构造的a对象释放时删除一次，而这两个对象的m_ptr是同一个指针，这就是所谓的指针悬挂问题。提供深拷贝的拷贝构造函数虽然可以保证正确，但是在有些时候会造成额外的性能损耗，因为有时候这种深拷贝是不必要的。这个时候就需要一个移动构造函数：如下：
        lass A
        {
        public:
            A() :m_ptr(new int(0)){}
            A(const A& a):m_ptr(new int(*a.m_ptr)) //深拷贝的拷贝构造函数
            {
                cout << "copy construct" << endl;
            }
            A(A&& a) :m_ptr(a.m_ptr)
            {
                m_ptr = a.m_ptr;
                a.m_ptr = nullptr;
                cout << "move construct" << endl;
            }
            ~A(){ delete m_ptr;}
        private:
            int* m_ptr;
        };
        int main(){
            A a = Get(false); 
        } 
        输出：
        construct
        move construct
        move construct
    std::move的理解，move实际上并不移动任何东西，他唯一的功能就是将一个左值强制转换为一个右值引用，如果是一些基本类型比如int和char[10]定长数组等类型，使用move的话仍然会发生拷贝（因为没有对应的移动构造函数）。所以，move对于含资源（堆内存或句柄）的对象来说更有意义。
    完美转发：在函数模板中，完全依照模板参数的类型（即保持参数的左值，右值特征），将参数传递给模板函数中调用的另外一个函数
    std::forward正是做这个事情的，他会按照参数的实际类型进行转发，例如：
    void processValue(int& a){ cout << "lvalue" << endl; }
    void processValue(int&& a){ cout << "rvalue" << endl; }
    template <typename T>
    void forwardValue(T&& val)
    {
        processValue(std::forward<T>(val)); //照参数本来的类型进行转发。
    }
    void Testdelcl()
    {
        int i = 0;
        forwardValue(i); //传入左值 
        forwardValue(0);//传入右值 
    }
    输出：
    lvaue 
    rvalue
    右值引用T&&是一个universal references，可以接受左值或者右值，正是这个特性让他适合作为一个参	数的路由，然后再通过std::forward按照参数的实际类型去匹配对应的重载函数，最终实现完美转发。