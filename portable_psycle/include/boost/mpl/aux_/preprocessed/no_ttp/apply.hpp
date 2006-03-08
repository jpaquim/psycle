// preprocessed version of 'boost/mpl/apply.hpp' header
// see the original for copyright information

namespace boost {
namespace mpl {

template<
      typename F, typename T1 = void_, typename T2 = void_
    , typename T3 = void_, typename T4 = void_, typename T5 = void_
    >
struct apply;

template< typename F >
struct apply0 : F
{
    static int const arity = 1; typedef F arg1;
 friend class apply0_rebind;
 typedef apply0_rebind rebind;
 };
 class apply0_rebind { public: template< typename U1 > struct apply : apply0<U1> { };
 
};

template<
      typename F
    >
struct apply< F,void_,void_,void_,void_,void_ >
    : apply0<F>
{
};

template<
      typename F, typename T1
    >
struct apply1
    : F::template apply<
          T1
        >
{
    static int const arity = 2; typedef F arg1;
 typedef T1 arg2;
 friend class apply1_rebind;
 typedef apply1_rebind rebind;
 };
 class apply1_rebind { public: template< typename U1, typename U2 > struct apply : apply1< U1,U2 > { };
 
};

template<
      typename F, typename T1
    >
struct apply< F,T1,void_,void_,void_,void_ >
    : apply1< F,T1 >
{
};

template<
      typename F, typename T1, typename T2
    >
struct apply2
    : F::template apply<
          T1, T2
        >
{
    static int const arity = 3; typedef F arg1;
 typedef T1 arg2;
 typedef T2 arg3;
 friend class apply2_rebind;
 typedef apply2_rebind rebind;
 };
 class apply2_rebind { public: template< typename U1, typename U2, typename U3 > struct apply : apply2< U1,U2,U3 > { };
 
};

template<
      typename F, typename T1, typename T2
    >
struct apply< F,T1,T2,void_,void_,void_ >
    : apply2< F,T1,T2 >
{
};

template<
      typename F, typename T1, typename T2, typename T3
    >
struct apply3
    : F::template apply<
          T1, T2, T3
        >
{
    static int const arity = 4; typedef F arg1;
 typedef T1 arg2;
 typedef T2 arg3;
 typedef T3 arg4;
 friend class apply3_rebind;
 typedef apply3_rebind rebind;
 };
 class apply3_rebind { public: template< typename U1, typename U2, typename U3, typename U4 > struct apply : apply3< U1,U2,U3,U4 > { };
 
};

template<
      typename F, typename T1, typename T2, typename T3
    >
struct apply< F,T1,T2,T3,void_,void_ >
    : apply3< F,T1,T2,T3 >
{
};

template<
      typename F, typename T1, typename T2, typename T3, typename T4
    >
struct apply4
    : F::template apply<
          T1, T2, T3, T4
        >
{
    static int const arity = 5; typedef F arg1;
 typedef T1 arg2;
 typedef T2 arg3;
 typedef T3 arg4;
 typedef T4 arg5;
 friend class apply4_rebind;
 typedef apply4_rebind rebind;
 };
 class apply4_rebind { public: template< typename U1, typename U2, typename U3, typename U4, typename U5 > struct apply : apply4< U1,U2,U3,U4,U5 > { };
 
};

template<
      typename F, typename T1, typename T2, typename T3, typename T4
    >
struct apply< F,T1,T2,T3,T4,void_ >
    : apply4< F,T1,T2,T3,T4 >
{
};

template<
      typename F, typename T1, typename T2, typename T3, typename T4
    , typename T5
    >
struct apply5
    : F::template apply<
          T1, T2, T3, T4, T5
        >
{
    static int const arity = 6; typedef F arg1;
 typedef T1 arg2;
 typedef T2 arg3;
 typedef T3 arg4;
 typedef T4 arg5;
 typedef T5 arg6;
 friend class apply5_rebind;
 typedef apply5_rebind rebind;
 };
 class apply5_rebind { public: template< typename U1, typename U2, typename U3, typename U4, typename U5, typename U6 > struct apply : apply5< U1,U2,U3,U4,U5,U6 > { };
 
};

// primary template (not a specialization!)
template<
      typename F, typename T1, typename T2, typename T3, typename T4
    , typename T5
    >
struct apply
    : apply5< F,T1,T2,T3,T4,T5 >
{
};

} // namespace mpl
} // namespace boost

