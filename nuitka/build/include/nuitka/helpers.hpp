//     Copyright 2013, Kay Hayen, mailto:kay.hayen@gmail.com
//
//     Part of "Nuitka", an optimizing Python compiler that is compatible and
//     integrates with CPython, but also works on its own.
//
//     Licensed under the Apache License, Version 2.0 (the "License");
//     you may not use this file except in compliance with the License.
//     You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//     Unless required by applicable law or agreed to in writing, software
//     distributed under the License is distributed on an "AS IS" BASIS,
//     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//     See the License for the specific language governing permissions and
//     limitations under the License.
//
#ifndef __NUITKA_HELPERS_H__
#define __NUITKA_HELPERS_H__

#define _DEBUG_UNFREEZER 0
#define _DEBUG_REFRAME 0

#include "nuitka/eval_order.hpp"

extern PyObject *_python_tuple_empty;
extern PyObject *_python_str_plain___dict__;
extern PyObject *_python_str_plain___class__;
extern PyObject *_python_str_plain___enter__;
extern PyObject *_python_str_plain___exit__;

// From CPython, to allow us quick access to the dictionary of an module, the structure is
// normally private, but we need it for quick access to the module dictionary.
typedef struct {
    PyObject_HEAD
    PyObject *md_dict;
} PyModuleObject;

extern void PRINT_ITEM_TO( PyObject *file, PyObject *object );
static PyObject *INCREASE_REFCOUNT( PyObject *object );
static PyObject *INCREASE_REFCOUNT_X( PyObject *object );

// Helper to check that an object is valid and has reference count better than 0.
static inline void assertObject( PyObject *value )
{
    assert( value != NULL );
    assert( Py_REFCNT( value ) > 0 );
}

static inline void assertObject( PyTracebackObject *value )
{
    assertObject( (PyObject *)value );
}

// Due to ABI issues, it seems that on Windows the symbols used by _PyObject_GC_TRACK are
// not exported and we need to use a function that does it instead.
#if defined( _WIN32 )
#define Nuitka_GC_Track PyObject_GC_Track
#define Nuitka_GC_UnTrack PyObject_GC_UnTrack
#else
#define Nuitka_GC_Track _PyObject_GC_TRACK
#define Nuitka_GC_UnTrack _PyObject_GC_UNTRACK
#endif

#include "nuitka/variables_temporary.hpp"
#include "nuitka/exceptions.hpp"

// For the EVAL_ORDER and MAKE_TUPLE macros.
#include "__helpers.hpp"


// Helper functions for reference count handling in the fly.
NUITKA_MAY_BE_UNUSED static PyObject *INCREASE_REFCOUNT( PyObject *object )
{
    assertObject( object );

    Py_INCREF( object );

    return object;
}

NUITKA_MAY_BE_UNUSED static PyObject *INCREASE_REFCOUNT_X( PyObject *object )
{
    Py_XINCREF( object );

    return object;
}

NUITKA_MAY_BE_UNUSED static PyObject *DECREASE_REFCOUNT( PyObject *object )
{
    assertObject( object );

    Py_DECREF( object );

    return object;
}

#include "nuitka/exceptions.hpp"

#include "printing.hpp"

#include "nuitka/helper/boolean.hpp"

#include "nuitka/helper/dictionaries.hpp"

#if PYTHON_VERSION >= 300
static char *_PyUnicode_AS_STRING( PyObject *unicode )
{
    PyObject *bytes = _PyUnicode_AsDefaultEncodedString( unicode, NULL );

    if (unlikely( bytes == NULL ))
    {
        throw _PythonException();
    }

    return PyBytes_AS_STRING( bytes );
}
#endif

typedef PyObject *(binary_api)( PyObject *, PyObject * );

NUITKA_MAY_BE_UNUSED static PyObject *BINARY_OPERATION( binary_api api, PyObject *operand1, PyObject *operand2 )
{
    assertObject( operand1 );
    assertObject( operand2 );

    PyObject *result = api( operand1, operand2 );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

#include "helper/operations.hpp"


typedef PyObject *(unary_api)( PyObject * );

NUITKA_MAY_BE_UNUSED static PyObject *UNARY_OPERATION( unary_api api, PyObject *operand )
{
    PyObject *result = api( operand );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *POWER_OPERATION( PyObject *operand1, PyObject *operand2 )
{
    PyObject *result = PyNumber_Power( operand1, operand2, Py_None );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *POWER_OPERATION_INPLACE( PyObject *operand1, PyObject *operand2 )
{
    PyObject *result = PyNumber_InPlacePower( operand1, operand2, Py_None );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

#include "nuitka/helper/richcomparisons.hpp"
#include "nuitka/helper/sequences.hpp"

static inline bool Nuitka_Function_Check( PyObject *object );
static inline PyObject *Nuitka_Function_GetName( PyObject *object );

static inline bool Nuitka_Generator_Check( PyObject *object );
static inline PyObject *Nuitka_Generator_GetName( PyObject *object );

static char const *GET_CALLABLE_NAME( PyObject *object )
{
    if ( Nuitka_Function_Check( object ) )
    {
        return Nuitka_String_AsString( Nuitka_Function_GetName( object ) );
    }
    else if ( Nuitka_Generator_Check( object ) )
    {
        return Nuitka_String_AsString( Nuitka_Generator_GetName( object ) );
    }
    else if ( PyMethod_Check( object ) )
    {
        return PyEval_GetFuncName( PyMethod_GET_FUNCTION( object ) );
    }
    else if ( PyFunction_Check( object ) )
    {
        return Nuitka_String_AsString( ((PyFunctionObject*)object)->func_name );
    }
#if PYTHON_VERSION < 300
    else if ( PyInstance_Check( object ) )
    {
        return Nuitka_String_AsString( ((PyInstanceObject*)object)->in_class->cl_name );
    }
    else if ( PyClass_Check( object ) )
    {
        return Nuitka_String_AsString(((PyClassObject*)object)->cl_name );
    }
#endif
    else if ( PyCFunction_Check( object ) )
    {
        return ((PyCFunctionObject*)object)->m_ml->ml_name;
    }
    else
    {
        return Py_TYPE( object )->tp_name;
    }
}

#include "nuitka/calling.hpp"

NUITKA_MAY_BE_UNUSED static long FROM_LONG( PyObject *value )
{
    long result = PyInt_AsLong( value );

    if (unlikely( result == -1 ))
    {
        THROW_IF_ERROR_OCCURED();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *TO_FLOAT( PyObject *value )
{
    PyObject *result;

#if PYTHON_VERSION < 300
    if ( PyString_CheckExact( value ) )
    {
        result = PyFloat_FromString( value, NULL );
    }
#else
    if ( PyUnicode_CheckExact( value ) )
    {
        result = PyFloat_FromString( value );
    }
#endif
    else
    {
        result = PyNumber_Float( value );
    }

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *TO_INT( PyObject *value )
{
    PyObject *result = PyNumber_Int( value );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

#define TO_INT2( value, base ) _TO_INT2( EVAL_ORDERED_2( value, base ) )
NUITKA_MAY_BE_UNUSED static PyObject *_TO_INT2( EVAL_ORDERED_2( PyObject *value, PyObject *base ) )
{
    int base_int = PyInt_AsLong( base );

    if (unlikely( base_int == -1 ))
    {
        THROW_IF_ERROR_OCCURED();
    }

    char *value_str = Nuitka_String_AsString( value );

    if (unlikely( value_str == NULL ))
    {
        throw _PythonException();
    }

    PyObject *result = PyInt_FromString( value_str, NULL, base_int );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *TO_LONG( PyObject *value )
{
    PyObject *result = PyNumber_Long( value );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

#define TO_LONG2( value, base ) _TO_LONG2( EVAL_ORDERED_2( value, base ) )
NUITKA_MAY_BE_UNUSED static PyObject *_TO_LONG2( EVAL_ORDERED_2( PyObject *value, PyObject *base ) )
{
    int base_int = PyInt_AsLong( base );

    if (unlikely( base_int == -1 ))
    {
        THROW_IF_ERROR_OCCURED();
    }

    char *value_str = Nuitka_String_AsString( value );

    if (unlikely( value_str == NULL ))
    {
        throw _PythonException();
    }

    PyObject *result = PyLong_FromString( value_str, NULL, base_int );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *TO_BOOL( PyObject *value )
{
    return BOOL_FROM( CHECK_IF_TRUE( value ) );
}

NUITKA_MAY_BE_UNUSED static PyObject *TO_STR( PyObject *value )
{
    PyObject *result = PyObject_Str( value );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *TO_UNICODE( PyObject *value )
{
    PyObject *result = PyObject_Unicode( value );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

#define TO_UNICODE3( value, encoding, errors ) _TO_UNICODE3( EVAL_ORDERED_3( value, encoding, errors ) )
NUITKA_MAY_BE_UNUSED static PyObject *_TO_UNICODE3( EVAL_ORDERED_3( PyObject *value, PyObject *encoding, PyObject *errors ) )
{
    char *encoding_str;

    PyObject *uarg2 = NULL;
    PyObject *uarg3 = NULL;

    if ( encoding == NULL )
    {
        encoding_str = NULL;
    }
    else if ( Nuitka_String_Check( encoding ) )
    {
        encoding_str = Nuitka_String_AsString_Unchecked( encoding );
    }
#if PYTHON_VERSION < 300
    else if ( PyUnicode_Check( encoding ) )
    {
        uarg2 = _PyUnicode_AsDefaultEncodedString( encoding, NULL );
        assertObject( uarg2 );

        encoding_str = Nuitka_String_AsString_Unchecked( uarg2 );
    }
#endif
    else
    {
        PyErr_Format( PyExc_TypeError, "unicode() argument 2 must be string, not %s", Py_TYPE( encoding )->tp_name );
        throw _PythonException();
    }

    char *errors_str;

    if ( errors == NULL )
    {
        errors_str = NULL;
    }
    else if ( Nuitka_String_Check( errors ) )
    {
        errors_str = Nuitka_String_AsString_Unchecked( errors );
    }
#if PYTHON_VERSION < 300
    else if ( PyUnicode_Check( errors ) )
    {
        uarg3 = _PyUnicode_AsDefaultEncodedString( errors, NULL );
        assertObject( uarg3 );

        errors_str = Nuitka_String_AsString_Unchecked( uarg3 );
    }
#endif
    else
    {
        Py_XDECREF( uarg2 );

        PyErr_Format( PyExc_TypeError, "unicode() argument 3 must be string, not %s", Py_TYPE( errors )->tp_name );
        throw _PythonException();
    }

    PyObject *result = PyUnicode_FromEncodedObject( value, encoding_str, errors_str );

    Py_XDECREF( uarg2 );
    Py_XDECREF( uarg3 );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    assert( PyUnicode_Check( result ) );

    return result;
}


NUITKA_MAY_BE_UNUSED static PyObject *MAKE_SET()
{
    PyObject *result = PySet_New( NULL );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *MAKE_SET( PyObject *tuple )
{
    assertObject( tuple );
    assert( PyTuple_Check( tuple ) );

    PyObject *result = PySet_New( tuple );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *MAKE_STATIC_METHOD( PyObject *method )
{
    assertObject( method );

    PyObject *attempt = PyStaticMethod_New( method );

    if ( attempt )
    {
        return attempt;
    }
    else
    {
        PyErr_Clear();

        return method;
    }
}

// Stolen from CPython implementation, so we can access it.
typedef struct {
    PyObject_HEAD
    long      it_index;
    PyObject *it_seq;
} seqiterobject;

NUITKA_MAY_BE_UNUSED static PyObject *MAKE_ITERATOR( PyObject *iterated )
{
    getiterfunc tp_iter = NULL;

#if PYTHON_VERSION < 300
    if ( PyType_HasFeature( Py_TYPE( iterated ), Py_TPFLAGS_HAVE_ITER ))
    {
#endif
        tp_iter = Py_TYPE( iterated )->tp_iter;
#if PYTHON_VERSION < 300
    }
#endif

    if ( tp_iter )
    {
        PyObject *result = (*Py_TYPE( iterated )->tp_iter)( iterated );

        if (likely( result != NULL ))
        {
            if (unlikely( !PyIter_Check( result )) )
            {
                PyErr_Format( PyExc_TypeError, "iter() returned non-iterator of type '%s'", Py_TYPE( result )->tp_name );

                Py_DECREF( result );
                throw _PythonException();
            }

            return result;
        }
        else
        {
            throw _PythonException();
        }
    }
    else if ( PySequence_Check( iterated ) )
    {
        seqiterobject *result = PyObject_GC_New( seqiterobject, &PySeqIter_Type );
        assert( result );

        result->it_index = 0;
        result->it_seq = INCREASE_REFCOUNT( iterated );

        Nuitka_GC_Track( result );

        return (PyObject *)result;
    }
    else
    {
        PyErr_Format( PyExc_TypeError, "'%s' object is not iterable", Py_TYPE( iterated )->tp_name );
        throw _PythonException();
    }
}

// Return the next item of an iterator. Avoiding any exception for end of iteration,
// callers must deal with NULL return as end of iteration, but will know it wasn't an
// Python exception, that will show as a thrown exception.
NUITKA_MAY_BE_UNUSED static PyObject *ITERATOR_NEXT( PyObject *iterator )
{
    assertObject( iterator );

    PyObject *result = (*Py_TYPE( iterator )->tp_iternext)( iterator );

    if (unlikely( result == NULL ))
    {
        THROW_IF_ERROR_OCCURED_NOT( PyExc_StopIteration );
    }
    else
    {
        assertObject( result );
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *BUILTIN_NEXT1( PyObject *iterator )
{
    assertObject( iterator );

    PyObject *result = (*Py_TYPE( iterator )->tp_iternext)( iterator );

    if (unlikely( result == NULL ))
    {
        // TODO: Throwing an error unless another exists, should be offered too.
        if ( !ERROR_OCCURED() )
        {
            PyErr_SetNone( PyExc_StopIteration );
        }

        throw _PythonException();
    }
    else
    {
        assertObject( result );
    }

    return result;
}


#define BUILTIN_NEXT2( iterator, default_value ) _BUILTIN_NEXT2( EVAL_ORDERED_2( iterator, default_value ) )
NUITKA_MAY_BE_UNUSED static PyObject *_BUILTIN_NEXT2( EVAL_ORDERED_2( PyObject *iterator, PyObject *default_value ) )
{
    assertObject( iterator );
    assertObject( default_value );

    PyObject *result = (*Py_TYPE( iterator )->tp_iternext)( iterator );

    if (unlikely( result == NULL ))
    {
        if ( ERROR_OCCURED() )
        {
            if ( PyErr_ExceptionMatches( PyExc_StopIteration ))
            {
                PyErr_Clear();

                return INCREASE_REFCOUNT( default_value );
            }
            else
            {
                throw _PythonException();
            }
        }
        else
        {
            return INCREASE_REFCOUNT( default_value );
        }
    }
    else
    {
        assertObject( result );
    }

    return result;
}


NUITKA_MAY_BE_UNUSED static inline PyObject *UNPACK_NEXT( PyObject *iterator, int seq_size_so_far )
{
    assertObject( iterator );
    assert( PyIter_Check( iterator ) );

    PyObject *result = (*Py_TYPE( iterator )->tp_iternext)( iterator );

    if (unlikely( result == NULL ))
    {
#if PYTHON_VERSION < 300
        if (unlikely( !ERROR_OCCURED() ))
#else
        if (unlikely( !ERROR_OCCURED() || PyErr_ExceptionMatches( PyExc_StopIteration ) ))
#endif
        {
            if ( seq_size_so_far == 1 )
            {
                PyErr_Format( PyExc_ValueError, "need more than 1 value to unpack" );
            }
            else
            {
                PyErr_Format( PyExc_ValueError, "need more than %d values to unpack", seq_size_so_far );
            }
        }

        throw _PythonException();
    }

    assertObject( result );

    return result;
}

NUITKA_MAY_BE_UNUSED static inline PyObject *UNPACK_PARAMETER_NEXT( PyObject *iterator, int seq_size_so_far )
{
    assertObject( iterator );
    assert( PyIter_Check( iterator ) );

    PyObject *result = (*Py_TYPE( iterator )->tp_iternext)( iterator );

    if (unlikely( result == NULL ))
    {
#if PYTHON_VERSION < 300
        if (unlikely( !ERROR_OCCURED() ))
#else
        if (unlikely( !ERROR_OCCURED() || PyErr_ExceptionMatches( PyExc_StopIteration ) ))
#endif
        {
            if ( seq_size_so_far == 1 )
            {
                PyErr_Format( PyExc_ValueError, "need more than 1 value to unpack" );
            }
            else
            {
                PyErr_Format( PyExc_ValueError, "need more than %d values to unpack", seq_size_so_far );
            }
        }

        return NULL;
    }

    assertObject( result );

    return result;
}

#if PYTHON_VERSION < 300
#define UNPACK_ITERATOR_CHECK( iterator, count ) _UNPACK_ITERATOR_CHECK( iterator )
NUITKA_MAY_BE_UNUSED static inline void _UNPACK_ITERATOR_CHECK( PyObject *iterator )
#else
NUITKA_MAY_BE_UNUSED static inline void UNPACK_ITERATOR_CHECK( PyObject *iterator, int count )
#endif
{
    assertObject( iterator );
    assert( PyIter_Check( iterator ) );

    PyObject *attempt = (*Py_TYPE( iterator )->tp_iternext)( iterator );

    if (likely( attempt == NULL ))
    {
        THROW_IF_ERROR_OCCURED_NOT( PyExc_StopIteration );
    }
    else
    {
        Py_DECREF( attempt );
#if PYTHON_VERSION < 300
        PyErr_Format( PyExc_ValueError, "too many values to unpack" );
#else
        PyErr_Format( PyExc_ValueError, "too many values to unpack (expected %d)", count );
#endif
        throw _PythonException();
    }
}


NUITKA_MAY_BE_UNUSED static inline bool UNPACK_PARAMETER_ITERATOR_CHECK( PyObject *iterator )
{
    assertObject( iterator );
    assert( PyIter_Check( iterator ) );

    PyObject *attempt = (*Py_TYPE( iterator )->tp_iternext)( iterator );

    if (likely( attempt == NULL ))
    {
        if ( ERROR_OCCURED() )
        {
            if (likely( PyErr_ExceptionMatches( PyExc_StopIteration ) ))
            {
                PyErr_Clear();
            }
            else
            {
                return false;
            }
        }

        return true;
    }
    else
    {
        Py_DECREF( attempt );

        PyErr_Format( PyExc_ValueError, "too many values to unpack" );
        return false;
    }
}

NUITKA_MAY_BE_UNUSED static bool HAS_KEY( PyObject *source, PyObject *key )
{
    assertObject( source );
    assertObject( key );

    return PyMapping_HasKey( source, key ) != 0;
}

NUITKA_MAY_BE_UNUSED static PyObject *LOOKUP_VARS( PyObject *source )
{
    assertObject( source );

    PyObject *result = PyObject_GetAttr( source, _python_str_plain___dict__ );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *IMPORT_NAME( PyObject *module, PyObject *import_name )
{
    assertObject( module );
    assertObject( import_name );

    PyObject *result = PyObject_GetAttr( module, import_name );

    if (unlikely( result == NULL ))
    {
        if ( PyErr_ExceptionMatches( PyExc_AttributeError ) )
        {
            PyErr_Format( PyExc_ImportError, "cannot import name %s", Nuitka_String_AsString( import_name ));
        }

        throw _PythonException();
    }

    return result;
}


#include "nuitka/helper/indexes.hpp"
#include "nuitka/helper/subscripts.hpp"
#include "nuitka/helper/slices.hpp"

#if PYTHON_VERSION < 300
NUITKA_MAY_BE_UNUSED static PyObject *FIND_ATTRIBUTE_IN_CLASS( PyClassObject *klass, PyObject *attr_name )
{
    PyObject *result = GET_PYDICT_ENTRY( (PyDictObject *)klass->cl_dict, (PyStringObject *)attr_name )->me_value;

    if ( result == NULL )
    {
        Py_ssize_t base_count = PyTuple_Size( klass->cl_bases );

        for ( Py_ssize_t i = 0; i < base_count; i++ )
        {
            result = FIND_ATTRIBUTE_IN_CLASS( (PyClassObject *)PyTuple_GetItem( klass->cl_bases, i ), attr_name );

            if ( result )
            {
                break;
            }
        }
    }

    return result;
}
#endif

#if PYTHON_VERSION < 300
static PyObject *LOOKUP_INSTANCE( PyObject *source, PyObject *attr_name )
{
    assertObject( source );
    assertObject( attr_name );

    assert( PyInstance_Check( source ) );
    assert( PyString_Check( attr_name ) );

    PyInstanceObject *source_instance = (PyInstanceObject *)source;

    // TODO: The special cases should get their own SET_ATTRIBUTE variant on the code
    // generation level as SET_ATTRIBUTE is called with constants only.
    if (unlikely( attr_name == _python_str_plain___dict__ ))
    {
        return INCREASE_REFCOUNT( source_instance->in_dict );
    }
    else if (unlikely( attr_name == _python_str_plain___class__ ))
    {
        return INCREASE_REFCOUNT( (PyObject *)source_instance->in_class );
    }
    else
    {
        // Try the instance dict first.
        PyObject *result = GET_PYDICT_ENTRY( (PyDictObject *)source_instance->in_dict, (PyStringObject *)attr_name )->me_value;

        if ( result )
        {
            return INCREASE_REFCOUNT( result );
        }

        // Next see if a class has it
        result = FIND_ATTRIBUTE_IN_CLASS( source_instance->in_class, attr_name );

        if ( result )
        {
            descrgetfunc func = Py_TYPE( result )->tp_descr_get;

            if ( func )
            {
                result = func( result, source, (PyObject *)source_instance->in_class );

                if (unlikely( result == NULL ))
                {
                    throw _PythonException();
                }

                assertObject( result );

                return result;
            }
            else
            {
                return INCREASE_REFCOUNT( result );
            }
        }

        THROW_IF_ERROR_OCCURED_NOT( PyExc_AttributeError );

        // Finally allow a __getattr__ to handle it or else it's an error.
        if ( source_instance->in_class->cl_getattr == NULL )
        {
            PyErr_Format( PyExc_AttributeError, "%s instance has no attribute '%s'", PyString_AS_STRING( source_instance->in_class->cl_name ), PyString_AS_STRING( attr_name ) );

            throw _PythonException();
        }
        else
        {
            PyObject *result = CALL_FUNCTION(
                source_instance->in_class->cl_getattr,
                PyObjectTemporary( MAKE_TUPLE2( source, attr_name ) ).asObject(),
                NULL
            );

            assertObject( result );

            return result;
        }
    }
}
#endif

NUITKA_MAY_BE_UNUSED static PyObject *LOOKUP_ATTRIBUTE( PyObject *source, PyObject *attr_name )
{
    assertObject( source );
    assertObject( attr_name );

#if PYTHON_VERSION < 300
    if ( PyInstance_Check( source ) )
    {
        PyObject *result = LOOKUP_INSTANCE( source, attr_name );

        assertObject( result );

        return result;
    }
    else
#endif
    {
        PyTypeObject *type = Py_TYPE( source );

        if ( type->tp_getattro != NULL )
        {
            PyObject *result = (*type->tp_getattro)( source, attr_name );

            if (unlikely( result == NULL ))
            {
                throw _PythonException();
            }

            assertObject( result );
            return result;
        }
        else if ( type->tp_getattr != NULL )
        {
            PyObject *result = (*type->tp_getattr)( source, Nuitka_String_AsString_Unchecked( attr_name ) );

            if (unlikely( result == NULL ))
            {
                throw _PythonException();
            }

            assertObject( result );
            return result;
        }
        else
        {
            PyErr_Format( PyExc_AttributeError, "'%s' object has no attribute '%s'", type->tp_name, Nuitka_String_AsString_Unchecked( attr_name ) );
            throw _PythonException();
        }
    }
}

NUITKA_MAY_BE_UNUSED static bool HAS_ATTRIBUTE( PyObject *source, PyObject *attr_name )
{
    assertObject( source );
    assertObject( attr_name );

    int res = PyObject_HasAttr( source, attr_name );

    if (unlikely( res == -1 ))
    {
        throw _PythonException();
    }

    return res == 1;
}

#if PYTHON_VERSION < 300
static void SET_INSTANCE( PyObject *target, PyObject *attr_name, PyObject *value )
{
    assertObject( target );
    assertObject( attr_name );
    assertObject( value );

    assert( PyInstance_Check( target ) );
    assert( PyString_Check( attr_name ) );


    PyInstanceObject *target_instance = (PyInstanceObject *)target;

    // TODO: The special cases should get their own SET_ATTRIBUTE variant on the code
    // generation level as SET_ATTRIBUTE is called with constants only.
    if (unlikely( attr_name == _python_str_plain___dict__ ))
    {
        if (unlikely( !PyDict_Check( value ) ))
        {
            PyErr_SetString( PyExc_TypeError, "__dict__ must be set to a dictionary" );
            throw _PythonException();
        }

        PyObjectTemporary old_dict( target_instance->in_dict );

        target_instance->in_dict = INCREASE_REFCOUNT( value );
    }
    else if (unlikely( attr_name == _python_str_plain___class__ ))
    {
        if (unlikely( !PyClass_Check( value ) ))
        {
            PyErr_SetString( PyExc_TypeError, "__class__ must be set to a class" );
            throw _PythonException();
        }

        PyObjectTemporary old_class( (PyObject *)target_instance->in_class );

        target_instance->in_class = (PyClassObject *)INCREASE_REFCOUNT( value );
    }
    else
    {
        if ( target_instance->in_class->cl_setattr != NULL )
        {
            PyObject *result = CALL_FUNCTION(
                target_instance->in_class->cl_setattr,
                PyObjectTemporary( MAKE_TUPLE3( target, attr_name, value ) ).asObject(),
                NULL
            );

            Py_DECREF( result );
        }
        else
        {
            int status = PyDict_SetItem( target_instance->in_dict, attr_name, value );

            if (unlikely( status == -1 ))
            {
                throw _PythonException();
            }
        }
    }
}
#endif

#define SET_ATTRIBUTE( value, target, attr_name ) _SET_ATTRIBUTE( EVAL_ORDERED_3( value, target, attr_name ) )
NUITKA_MAY_BE_UNUSED static void _SET_ATTRIBUTE( EVAL_ORDERED_3( PyObject *value, PyObject *target, PyObject *attr_name ) )
{
    assertObject( target );
    assertObject( attr_name );
    assertObject( value );

#if PYTHON_VERSION < 300
    if ( PyInstance_Check( target ) )
    {
        SET_INSTANCE( target, attr_name, value );
    }
    else
#endif
    {
        PyTypeObject *type = Py_TYPE( target );

        if ( type->tp_setattro != NULL )
        {
            int status = (*type->tp_setattro)( target, attr_name, value );

            if (unlikely( status == -1 ))
            {
                throw _PythonException();
            }
        }
        else if ( type->tp_setattr != NULL )
        {
            int status = (*type->tp_setattr)( target, Nuitka_String_AsString_Unchecked( attr_name ), value );

            if (unlikely( status == -1 ))
            {
                throw _PythonException();
            }
        }
        else if ( type->tp_getattr == NULL && type->tp_getattro == NULL )
        {
            PyErr_Format(
                PyExc_TypeError,
                "'%s' object has no attributes (assign to %s)",
                type->tp_name,
                Nuitka_String_AsString_Unchecked( attr_name )
            );

            throw _PythonException();
        }
        else
        {
            PyErr_Format(
                PyExc_TypeError,
                "'%s' object has only read-only attributes (assign to %s)",
                type->tp_name,
                Nuitka_String_AsString_Unchecked( attr_name )
            );

            throw _PythonException();
        }
    }
}

NUITKA_MAY_BE_UNUSED static void DEL_ATTRIBUTE( PyObject *target, PyObject *attr_name )
{
    assertObject( target );
    assertObject( attr_name );

    int status = PyObject_DelAttr( target, attr_name );

    if (unlikely( status == -1 ))
    {
        throw _PythonException();
    }
}

NUITKA_MAY_BE_UNUSED static PyObject *LOOKUP_SPECIAL( PyObject *source, PyObject *attr_name )
{
#if PYTHON_VERSION < 300
    if ( PyInstance_Check( source ) )
    {
        return LOOKUP_INSTANCE( source, attr_name );
    }
#endif

    // TODO: There is heavy optimization in CPython to avoid it. Potentially that's worth
    // it to imitate that.

    PyObject *result = _PyType_Lookup( Py_TYPE( source ), attr_name );

    if (likely( result ))
    {
        descrgetfunc func = Py_TYPE( result )->tp_descr_get;

        if ( func == NULL )
        {
            return INCREASE_REFCOUNT( result );
        }
        else
        {
            PyObject *func_result = func( result, source, (PyObject *)( Py_TYPE( source ) ) );

            if (unlikely( func_result == NULL ))
            {
                throw _PythonException();
            }

            return func_result;
        }
    }

    PyErr_SetObject( PyExc_AttributeError, attr_name );
    throw _PythonException();
}

// Necessary to abstract the with statement lookup difference between pre-Python2.7 and
// others. Since Python 2.7 the code does no full attribute lookup anymore, but instead
// treats enter and exit as specials.
NUITKA_MAY_BE_UNUSED static inline PyObject *LOOKUP_WITH_ENTER( PyObject *source )
{
#if PYTHON_VERSION < 270
    return LOOKUP_ATTRIBUTE( source, _python_str_plain___enter__ );
#else
    return LOOKUP_SPECIAL( source, _python_str_plain___enter__ );
#endif
}

NUITKA_MAY_BE_UNUSED static inline PyObject *LOOKUP_WITH_EXIT( PyObject *source )
{
#if PYTHON_VERSION < 270
    return LOOKUP_ATTRIBUTE( source, _python_str_plain___exit__ );
#else
    return LOOKUP_SPECIAL( source, _python_str_plain___exit__ );
#endif
}

NUITKA_MAY_BE_UNUSED static void APPEND_TO_LIST( PyObject *list, PyObject *item )
{
    assertObject( list );
    assertObject( item );

    int status = PyList_Append( list, item );

    if (unlikely( status == -1 ))
    {
        throw _PythonException();
    }
}

NUITKA_MAY_BE_UNUSED static void ADD_TO_SET( PyObject *set, PyObject *item )
{
    int status = PySet_Add( set, item );

    if (unlikely( status == -1 ))
    {
        throw _PythonException();
    }
}



NUITKA_MAY_BE_UNUSED static PyObject *SEQUENCE_CONCAT( PyObject *seq1, PyObject *seq2 )
{
    PyObject *result = PySequence_Concat( seq1, seq2 );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

#include "nuitka/builtins.hpp"

#include "nuitka/frame_guards.hpp"

#include "nuitka/variables_parameters.hpp"
#include "nuitka/variables_locals.hpp"
#include "nuitka/variables_shared.hpp"

extern PyModuleObject *module_builtin;

NUITKA_MAY_BE_UNUSED static PyObject *TUPLE_COPY( PyObject *tuple )
{
    assertObject( tuple );

    assert( PyTuple_CheckExact( tuple ) );

    Py_ssize_t size = PyTuple_GET_SIZE( tuple );

    PyObject *result = PyTuple_New( size );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    for ( Py_ssize_t i = 0; i < size; i++ )
    {
        PyTuple_SET_ITEM( result, i, INCREASE_REFCOUNT( PyTuple_GET_ITEM( tuple, i ) ) );
    }

    return result;
}

NUITKA_MAY_BE_UNUSED static PyObject *LIST_COPY( PyObject *list )
{
    assertObject( list );

    assert( PyList_CheckExact( list ) );

    Py_ssize_t size = PyList_GET_SIZE( list );

    PyObject *result = PyList_New( size );

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    for ( Py_ssize_t i = 0; i < size; i++ )
    {
        PyList_SET_ITEM( result, i, INCREASE_REFCOUNT( PyList_GET_ITEM( list, i ) ) );
    }

    return result;
}


// Compile source code given, pretending the file name was given.
extern PyObject *COMPILE_CODE( PyObject *source_code, PyObject *file_name, PyObject *mode, int flags );

// For quicker builtin open() functionality.
#define OPEN_FILE( file_name, mode, buffer ) _OPEN_FILE( EVAL_ORDERED_3( file_name, mode, buffer ) )
extern PyObject *_OPEN_FILE( EVAL_ORDERED_3( PyObject *file_name, PyObject *mode, PyObject *buffering ) );

// For quicker builtin chr() functionality.
extern PyObject *BUILTIN_CHR( PyObject *value );

// For quicker builtin ord() functionality.
extern PyObject *BUILTIN_ORD( PyObject *value );

// For quicker builtin bin() functionality.
extern PyObject *BUILTIN_BIN( PyObject *value );

// For quicker builtin oct() functionality.
extern PyObject *BUILTIN_OCT( PyObject *value );

// For quicker builtin hex() functionality.
extern PyObject *BUILTIN_HEX( PyObject *value );

// For quicker callable() functionality.
extern PyObject *BUILTIN_CALLABLE( PyObject *value );

// For quicker iter() functionality if 2 arguments arg given.
#define BUILTIN_ITER2( callable, sentinel ) _BUILTIN_ITER2( EVAL_ORDERED_2( callable, sentinel ) )
extern PyObject *_BUILTIN_ITER2( EVAL_ORDERED_2( PyObject *callable, PyObject *sentinel ) );

// For quicker type() functionality if 1 argument is given.
extern PyObject *BUILTIN_TYPE1( PyObject *arg );

// For quicker type() functionality if 3 arguments are given (to build a new type).
#define BUILTIN_TYPE3( module_name, name, bases, dict ) _BUILTIN_TYPE3( EVAL_ORDERED_4( module_name, name, bases, dict ) )
extern PyObject *_BUILTIN_TYPE3( EVAL_ORDERED_4( PyObject *module_name, PyObject *name, PyObject *bases, PyObject *dict ) );

// For quicker builtin range() functionality.
#define BUILTIN_RANGE3( low, high, step ) _BUILTIN_RANGE3( EVAL_ORDERED_3( low, high, step ) )
extern PyObject *_BUILTIN_RANGE3( EVAL_ORDERED_3( PyObject *low, PyObject *high, PyObject *step ) );
#define BUILTIN_RANGE2( low, high ) _BUILTIN_RANGE2( EVAL_ORDERED_2( low, high ) )
extern PyObject *_BUILTIN_RANGE2( EVAL_ORDERED_2( PyObject *low, PyObject *high ) );
extern PyObject *BUILTIN_RANGE( PyObject *boundary );

// For quicker builtin len() functionality.
extern PyObject *BUILTIN_LEN( PyObject *boundary );

// For quicker builtin dir(arg) functionality.
extern PyObject *BUILTIN_DIR1( PyObject *arg );

// For quicker builtin super() functionality.
#define BUILTIN_SUPER( type, object ) _BUILTIN_SUPER( EVAL_ORDERED_2( type, object ) )
extern PyObject *_BUILTIN_SUPER( EVAL_ORDERED_2( PyObject *type, PyObject *object ) );

// For quicker isinstance() functionality.
#define BUILTIN_ISINSTANCE( inst, cls ) _BUILTIN_ISINSTANCE( EVAL_ORDERED_2( inst, cls ) )
extern PyObject *_BUILTIN_ISINSTANCE( EVAL_ORDERED_2( PyObject *inst, PyObject *cls ) );

// For quicker getattr() functionality.
#define BUILTIN_GETATTR( object, attribute, default_value ) _BUILTIN_GETATTR( EVAL_ORDERED_3( object, attribute, default_value ) )
extern PyObject *_BUILTIN_GETATTR( EVAL_ORDERED_3( PyObject *object, PyObject *attribute, PyObject *default_value ) );

NUITKA_MAY_BE_UNUSED static PyObject *EVAL_CODE( PyObject *code, PyObject *globals, PyObject *locals )
{
    if ( PyDict_Check( globals ) == 0 )
    {
        PyErr_Format( PyExc_TypeError, "exec: arg 2 must be a dictionary or None" );
        throw _PythonException();
    }

    if ( locals == NULL || locals == Py_None )
    {
        locals = globals;
    }

    if ( PyMapping_Check( locals ) == 0 )
    {
        PyErr_Format( PyExc_TypeError, "exec: arg 3 must be a mapping or None" );
        throw _PythonException();
    }

    // Set the __builtins__ in globals, it is expected to be present.
    if ( PyDict_GetItemString( globals, (char *)"__builtins__" ) == NULL )
    {
        if ( PyDict_SetItemString( globals, (char *)"__builtins__", (PyObject *)module_builtin ) == -1 )
        {
            throw _PythonException();
        }
    }

#if PYTHON_VERSION < 300
    PyObject *result = PyEval_EvalCode( (PyCodeObject *)code, globals, locals );
#else
    PyObject *result = PyEval_EvalCode( code, globals, locals );
#endif

    if (unlikely( result == NULL ))
    {
        throw _PythonException();
    }

    return result;
}

// Create a code object for the given filename and function name
#if PYTHON_VERSION < 300
extern PyCodeObject *MAKE_CODEOBJ( PyObject *filename, PyObject *function_name, int line, PyObject *argnames, int arg_count, bool is_generator );
#else
extern PyCodeObject *MAKE_CODEOBJ( PyObject *filename, PyObject *function_name, int line, PyObject *argnames, int arg_count, int kw_only_count, bool is_generator );
#endif

#include "nuitka/importing.hpp"

// For the constant loading:
extern void UNSTREAM_INIT( void );
extern PyObject *UNSTREAM_CONSTANT( char const *buffer, Py_ssize_t size );
extern PyObject *UNSTREAM_STRING( char const *buffer, Py_ssize_t size, bool intern );

extern void enhancePythonTypes( void );

// Parse the command line parameters and provide it to sys module.
extern void setCommandLineParameters( int argc, char *argv[] );

// Replace inspect functions with ones that accept compiled types too.
extern void patchInspectModule( void );

// Replace builtin functions with ones that accept compiled types too.
extern void patchBuiltinModule( void );

#if PYTHON_VERSION >= 300
NUITKA_MAY_BE_UNUSED static PyObject *SELECT_METACLASS( PyObject *metaclass, PyObject *bases )
{
    assertObject( metaclass );
    assertObject( bases );

    if ( PyType_Check( metaclass ))
    {
        PyObject *winner = (PyObject *)_PyType_CalculateMetaclass( (PyTypeObject *)metaclass, bases );

        assertObject( winner );

        if ( winner == NULL )
        {
            throw _PythonException();
        }

        return INCREASE_REFCOUNT( winner );
    }
    else
    {
        return INCREASE_REFCOUNT( metaclass );
    }
}
#else

NUITKA_MAY_BE_UNUSED static PyObject *SELECT_METACLASS( PyObject *bases, PyObject *metaclass_global )
{
    assertObject( bases );

    PyObject *metaclass;

    assert( bases != Py_None );

    if ( PyTuple_GET_SIZE( bases ) > 0 )
    {
        PyObject *base = PyTuple_GET_ITEM( bases, 0 );

        metaclass = PyObject_GetAttr( base, _python_str_plain___class__ );

        if ( metaclass == NULL )
        {
            PyErr_Clear();

            metaclass = INCREASE_REFCOUNT( (PyObject *)Py_TYPE( base ) );
        }
    }
    else if ( metaclass_global != NULL )
    {
        metaclass = INCREASE_REFCOUNT( metaclass_global );
    }
    else
    {
        // Default to old style class.
        metaclass = INCREASE_REFCOUNT( (PyObject *)&PyClass_Type );
    }

    return metaclass;
}

#endif

#endif
