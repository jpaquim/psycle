// Copyright David Abrahams 2002. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#ifndef AS_TO_PYTHON_FUNCTION_DWA2002121_HPP
# define AS_TO_PYTHON_FUNCTION_DWA2002121_HPP
# include <boost/python/converter/to_python_function_type.hpp>

namespace boost { namespace python { namespace converter { 

// Given a typesafe to_python conversion function, produces a
// to_python_function_t which can be registered in the usual way.
template <class T, class ToPython>
struct as_to_python_function
{
    // Assertion functions used to prevent wrapping of converters
    // which take non-const reference parameters. The T* argument in
    // the first overload ensures it isn't used in case T is a
    // reference.
    template <class U>
    static int convert_function_must_take_value_or_const_reference(U(*)(T), int, T* = 0);
    template <class U>
    static int convert_function_must_take_value_or_const_reference(U(*)(T const&), long ...);
        
    static PyObject* convert(void const* x)
    {
        BOOST_STATIC_ASSERT(
            sizeof(
                convert_function_must_take_value_or_const_reference(&ToPython::convert, 1L))
            == sizeof(int));
        
        // Yes, the const_cast below opens a hole in const-correctness,
        // but it's needed to convert auto_ptr<U> to python.
        //
        // How big a hole is it?  It allows ToPython::convert() to be
        // a function which modifies its argument. The upshot is that
        // client converters applied to const objects may invoke
        // undefined behavior. The damage, however, is limited by the
        // use of the assertion function. Thus, the only way this can
        // modify its argument is if T is an auto_ptr-like type. There
        // is still a const-correctness hole w.r.t. auto_ptr<U> const,
        // but c'est la vie.
        return ToPython::convert(*const_cast<T*>(static_cast<T const*>(x)));
    }
};

}}} // namespace boost::python::converter

#endif // AS_TO_PYTHON_FUNCTION_DWA2002121_HPP
