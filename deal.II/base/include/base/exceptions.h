//----------------------------  exceptions.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  exceptions.h  ---------------------------
#ifndef __deal2__exceptions_h
#define __deal2__exceptions_h


#include <base/config.h>

#include <exception>

// we only need output streams, but older compilers did not provide
// them in a separate include file
#ifdef HAVE_STD_OSTREAM_HEADER
#  include <ostream>
#else
#  include <iostream>
#endif


#ifndef __GNUC__
#  define __PRETTY_FUNCTION__ "(unknown)"
#endif


/**
 *  This class should be used as a base class for
 *  all exception classes. Do not use its methods
 *  and variables directly since the interface
 *  and mechanism may be subject to change. Rather
 *  create new exception classes using the
 *  @p{DeclException} macro family.
 *
 *
 *  @sect2{General overview of the exception handling mechanism in deal.II}
 *
 *  The error handling mechanism in @p{deal.II} is generally used in two ways.
 *  The first uses error checking in debug mode only and is useful for programs
 *  which are not fully tested. When the program shows no error anymore, one may
 *  switch off error handling and get better performance by this, since checks
 *  for errors are done quite frequently in the library (a typical speed up is
 *  a factor of four!). This mode of exception generation is most useful for
 *  internal consistency checks such as range checking or checking of the
 *  validity of function arguments. Errors of this kind usually are programming
 *  errors and the program should abort with as detailed a message as possible,
 *  including location and reason for the generation of the exception.
 *
 *  The second mode is for error checks which should always be on, such as for
 *  I/O errors, failing memory requests and the like. It does not make much
 *  sense to turn this mode off, since this kind of errors may happen in tested
 *  and untested programs likewise. Exceptions of this kind do not terminate the
 *  program, rather they throw exceptions in the @p{C++} manner, allowing the
 *  program to catch them and eventually do something about it. As it may be
 *  useful to have some information printed out if an exception could not be
 *  handled properly, additional information is passed along as for the first
 *  mode. The latter makes it necessary to provide a family of macros which
 *  enter this additional information into the exception class; this could
 *  in principle be done by the programmer himself each time by hand, but since
 *  the information can be obtained automatically, a macro is provided for
 *  this.
 *
 *  Both modes use exception classes, which need to have special features
 *  additionally to the @p{C++} standard's @p{exception} class. Such a class
 *  is declared by the following lines of code:
 *   @begin{verbatim}
 *     DeclException2 (ExcDomain, int, int,
 *                     << "Index= " << arg1 << "Upper Bound= " << arg2);
 *  @end{verbatim}
 *  This declares an exception class named @p{ExcDomain}, which
 *  has two variables as additional information (named
 *  @p{arg1} and @p{arg2} by default) and which outputs the
 *  given sequence (which is appended to an @p{std::ostream}
 *  variable's name, thus the weird syntax). There are
 *  other @p{DeclExceptionN} macros for exception classes
 *  with more or no parameters. It is proposed to let start
 *  the name of all exceptions by @p{Exc...} and to declare them locally to
 *  the class it is to be used in. Declaring exceptions globally is possible
 *  but pollutes the global namespace, is less readable and thus unnecessary.
 *
 *  Since exception classes are declared the same way for both modes of error
 *  checking, it is possible to use an exception declared through the
 *  @p{DeclExceptionN(...)} macro family in both modes; there is no need to
 *  declare different classes for each of these.
 *
 *
 *  @sect2{Use of the debug mode exceptions}
 *
 *  To use the exception mechanism for debug mode error checking, write lines
 *  like the following in your source code:
 *  @begin{verbatim}
 *    Assert (n<dim, ExcDomain(n,dim));
 *  @end{verbatim}
 *  which by macro expansion does the following:
 *  @begin{verbatim}
 *    #ifdef DEBUG
 *        if (!cond)
 *              issue error of class ExcDomain(n,dim)
 *    #else
 *        do nothing
 *    #endif
 *  @end{verbatim}
 *  i.e. it issues an error only if the preprocessor variable
 *  @p{DEBUG} is set and if the given condition (in this case
 *  @p{n<dim} is violated.
 *
 *  If the exception was declared using the @p{DeclException0 (...)}
 *  macro, i.e. without any additional parameters, its name has
 *  nonetheless to be given with parentheses:
 *  @p{Assert (i>m, ExcSomewhat());}
 *
 *  @sect3{How it works internally}
 *  If the @p{DEBUG} preprocessor directive is set, the call @p{Assert
 *  (cond, exc);} is basically converted by the preprocessor into the
 *  sequence (note that function names and exact calling sequences may
 *  change over time, but the general principle remains the same)
 *  @begin{verbatim}
 *    if (!(cond))
 *      __IssueError_Assert (__FILE__,
 *                           __LINE__,
 *                           __PRETTY_FUNCTION__,
 *                           #cond,
 *                           #exc,
 *                           &exc);
 *  @end{verbatim}
 *  i.e. if the given condition is violated, then the file and
 *  line in which the exception occured as well as
 *  the condition itself and the call sequence of the
 *  exception object is transferred. Additionally an object
 *  of the form given by @p{exc} is created (this is normally an
 *  unnamed object like in @p{ExcDomain (n, dim)} of class
 *  @p{ExcDomain}) and transferred to the @p{__IssueError_Assert}
 *  function.
 *
 *  @p{__PRETTY__FUNCTION__} is a macro defined only by the GNU CC
 *  compiler and gives the name of the function. If another compiler
 *  is used, we set @p{__PRETTY_FUNCTION__ = "(unknown)"}.
 *
 *  In @p{__IssueError} the given data
 *  is transferred into the @p{exc} object by calling the
 *  @p{SetFields} function; after that, the general error info
 *  is printed onto @p{std::cerr} using the @p{PrintError} function of
 *  @p{exc} and finally the exception specific data is printed
 *  using the user defined function @p{PrintError} (which is
 *  normally created using the @p{DeclException (...)} macro
 *  family.
 *
 *  After printing all this information, @p{abort()} is called (with
 *  one exception, see the end of this section). This terminates the
 *  program, which is the right thing to do for this kind of error
 *  checking since it is used to detect programming errors rather than
 *  run-time errors; a program can, by definition, not recover from
 *  programming errors.
 *
 *  If the preprocessor variable @p{DEBUG} is not set, then nothing
 *  happens, i.e. the @p{Assert} macro is expanded to @p{{}}.
 *
 *  Sometimes, there is no useful condition for an exception other
 *  than that the program flow should not have reached a certain point,
 *  e.g. a @p{default} section of a @p{switch} statement. In this case,
 *  raise the exception by the following construct:
 *  @begin{verbatim}
 *    Assert (false, ExcInternalError());
 *  @end{verbatim}
 *
 *  As mentioned above, the program is terminated once a call to
 *  @p{Assert} fails. However, there is one case where we do not want
 *  to do this, namely when a C++ exception is active. The usual case
 *  where this happens is that someone throws an exception through the
 *  @p{AssertThrow} mechanism (see below) which, while the stack is
 *  unwound, leads to the destruction of other objects in stack frames
 *  above. If other objects refer to the objects being thus destroyed,
 *  some destructors raise an exception through @p{Assert}. If we
 *  would abort the program then, we would only ever see the message
 *  that an object is being destroyed which is still referenced from
 *  somewhere, but we would never see the original exception that
 *  triggered this. (You can see it in the debugger by putting a break
 *  point on the function @p{__throw}, but you cannot see it from the
 *  program itself.) In that case, we use a C++ standard library
 *  function to detect the presence of another active exception and do
 *  not terminate the program to allow that the thrown exception
 *  propagates to some place where its message can be displayed.
 *
 *  Since it is common that one failed assertion leads to a whole
 *  chain of others, we only ever print the very first message. If the
 *  program is then aborted, that is no problem. If it is not (since a
 *  C++ exception is active), only the first is displayed and a
 *  message about suppressed follow-up messages is shown.
 *
 *
 *  @sect2{Use of run-time exceptions}
 *
 *  For this mode, the standard @p{C++} @p{throw} and @p{catch} concept exists. We
 *  want to keep to this, but want to extend it a bit. In general, the
 *  structure is the same, i.e. you normally raise and exception by
 *  @begin{verbatim}
 *    if (!cond)
 *      throw ExcSomething();
 *  @end{verbatim}
 *  and catch it using the statement
 *  @begin{verbatim}
 *    try {
 *      do_something ();
 *    }
 *    catch (exception &e) {
 *      std::cerr << "Exception occured:" << std::endl
 *           << e.what ()
 *           << std::endl;
 *      do_something_to_reciver ();
 *    };
 *  @end{verbatim}
 *  @p{exception} is a standard @p{C++} class providing basic functionality for
 *  exceptions, such as the virtual function @p{what()} which returns some
 *  information on the exception itself. This information is useful if an
 *  exception can't be handled properly, in which case as precise a description
 *  as possible should be printed.
 *
 *  The problem here is that to get significant and useful information out
 *  of @p{what()}, it is necessary to overload this function in out exception
 *  class and call the @p{throw} operator with additional arguments to the
 *  exception class. The first thing, overloading the @p{what} function is
 *  done using the @p{DeclExceptionN} macros, but putting the right information,
 *  which is the same as explained above for the @p{Assert} expansion, requires
 *  some work if one would want to write it down each time:
 *  @begin{verbatim}
 *    if (!cond)
 *      {
 *        ExcSomething e(additional information);
 *        e.SetFields (__FILE__, __LINE__, __PRETTY_FUNCTION__,
 *                     "condition as a string",
 *                     "name of condition as a string");
 *        throw e;
 *      };
 *  @end{verbatim}
 *
 *  For this purpose, the macro @p{AssertThrow} was invented. It does mainly
 *  the same job as does the @p{Assert} macro, but it does not kill the
 *  program, it rather throws an exception as shown above. The mode of usage
 *  is
 *  @begin{verbatim}
 *    AssertThrow (cond, ExcSomething(additional information));
 *  @end{verbatim}
 *  The condition to be checked is incorporated into the macro in order to
 *  allow passing the violated condition as a string. The expansion of the
 *  @p{AssertThrow} macro is not affected by the @p{DEBUG} preprocessor variable.
 *
 *
 *  @sect2{Description of the DeclExceptionN macro family}
 *
 *  Declare an exception class without any additional parameters.
 *  There is a whole family of @p{DeclException?} macros
 *  where @p{?} is to be replaced by the number of additional
 *  parameters (0 to 5 presently).
 *  
 *  The syntax is as follows:
 *  @begin{verbatim}
 *    DeclException2 (ExcDomain,
 *                    int,
 *                    int,
 *                    << " i=" << arg1 << ", m=" << arg2);
 *  @end{verbatim}
 *  The first is the name of the exception class to be created.
 *  The next arguments are the types of the parameters (in this
 *  case there are two type names needed) and finally the output
 *  sequence with which you can print additional information.
 *  
 *  The syntax of the output sequence is a bit weird but gets
 *  clear once you see how this macro is defined:
 *  @begin{verbatim}
 *  class name : public ExceptionBase {
 *    public:
 *      name (const type1 a1, const type2 a2) :
 *                     arg1 (a1), arg2(a2) {};
 *      virtual void PrintInfo (std::ostream &out) const {
 *        out outsequence << std::endl;
 *      };
 *    private:
 *      type1 arg1;
 *      type2 arg2;
 *  };
 *  @end{verbatim}
 *   
 *  If declared as specified, you can later use this exception class
 *  in the following manner:
 *  @begin{verbatim}
 *    int i=5;
 *    int m=3;
 *    Assert (i<m, MyExc2(i,m));
 *  @end{verbatim}
 *  and the output if the condition fails will be
 *  @begin{verbatim}
 *    --------------------------------------------------------
 *    An error occurred in line <301> of file <exc-test.cc>.
 *    The violated condition was: 
 *      i<m
 *    The name and call sequence of the exception was:
 *      MyExc2(i,m)
 *    Additional Information: 
 *      i=5, m=3
 *    --------------------------------------------------------
 *  @end{verbatim}
 *  
 *  Obviously for the @p{DeclException0(name)} macro, no types and
 *  also no output sequence is allowed.
 *
 *
 *  @author Wolfgang Bangerth, November 1997, extensions 1998
 */
class ExceptionBase : public std::exception
{
  public:
				     /**
				      * Default constructor.
				      */
    ExceptionBase ();
    
				     /**
				      *  The constructor takes the file in which the
				      *  error happened, the line and the violated
				      *  condition as well as the name of the
				      *  exception class as a @p{char*} as arguments.
				      */
    ExceptionBase (const char* f, const int l, const char *func,
		   const char* c, const char *e);

				     /**
				      * Destructor. Empty, but needed
				      * for the sake of exception
				      * specification, since the base
				      * class has this exception
				      * specification and the
				      * automatically generated
				      * destructor would have a
				      * different one due to member
				      * objects.
				      */
    virtual ~ExceptionBase () throw();
    
				     /**
				      *  Set the file name and line of where the
				      *  exception appeared as well as the violated
				      *  condition and the name of the exception as
				      *  a char pointer.
				      */
    void SetFields (const char *f,
		    const int   l,
		    const char *func,
		    const char *c,
		    const char *e);
    
				     /**
				      *  Print out the general part of the error
				      *  information.
				      */
    void PrintExcData (std::ostream &out) const;

				     /**
				      *  Print more specific information about the
				      *  exception which occured. Overload this
				      *  function in your own exception classes.
				      */
    virtual void PrintInfo (std::ostream &out) const;


				     /**
				      *  Function derived from the base class
				      *  which allows to pass information like
				      *  the line and name of the file where the
				      *  exception occured as well as user
				      *  information.
				      *
				      *  This function is mainly used when using
				      *  exceptions declared by the
				      *  @p{DeclException*} macros with the @p{throw}
				      *  mechanism or the @p{AssertThrow} macro.
				      */
    virtual const char * what () const throw ();

  protected:
				     /**
				      * Name of the file this exception happen in.
				      */
    const char  *file;

				     /**
				      * Line number in this file.
				      */
    unsigned int line;

				     /**
				      * Name of the function, pretty printed.
				      */
    const char  *function;

				     /**
				      * The violated condition, as a string.
				      */
    const char  *cond;

				     /**
				      * Name of the exception and call sequence.
				      */
    const char  *exc;
};




/**
 * In this namespace functions in connection with the Assert and
 * AssertThrow mechanism are declared.
 */
namespace deal_II_exceptions
{

				   /**
				    * Set a string that is printed
				    * upon output of the message
				    * indicating a triggered
				    * @p{Assert} statement. This
				    * string, which is printed in
				    * addition to the usual output may
				    * indicate information that is
				    * otherwise not readily available
				    * unless we are using a
				    * debugger. For example, with
				    * distributed programs on cluster
				    * computers, the output of all
				    * processes is redirected to the
				    * same console window. In this
				    * case, it is convenient to set as
				    * additional name the name of the
				    * host on which the program runs,
				    * so that one can see in which
				    * instance of the program the
				    * exception occured.
				    *
				    * The string pointed to by the
				    * argument is copied, so needs not
				    * be stored after the call to this
				    * function.
				    *
				    * Previously set additional output
				    * is replaced by the argument
				    * given to this function.
				    */
  void set_additional_assert_output (const char * const p);
  
  
/**
 * The functions in this namespace are in connection with the Assert
 * and AssertThrow mechanism but are solely for internal purposes and
 * are not for use outside the exception handling and throwing
 * mechanism.
 */
  namespace internals
  {

/**
 *  Relay exceptions from the @p{Assert} macro to the
 *  @p{__IssueError_Assert} function. Used to convert the last
 *  argument from arbitrary type to @ref{ExceptionBase} which is not
 *  possible inside the @p{Assert} macro due to syntactical
 *  difficulties in connection with the way we use the macro and the
 *  declaration of the exception classes.
 *
 *  @see ExceptionBase
 */
    template <class exc>
    inline
    void issue_error_assert_1 (const char *file,
			       int         line,
			       const char *function,
			       const char *cond,
			       const char *exc_name,
			       exc         e)
    {
      issue_error_assert (file,line,function,cond,exc_name,e);
    };
    


/**
 *  This routine does the main work for the
 *  exception generation mechanism used in the
 *  @p{Assert} macro.
 *
 *  @see ExceptionBase
 */
    void issue_error_assert (const char *file,
			     int         line,
			     const char *function,
			     const char *cond,
			     const char *exc_name,
			     ExceptionBase &         e);
  

/**
 *  This routine does the main work for the
 *  exception generation mechanism used in the
 *  @p{AssertThrow} macro.
 *
 *  @see ExceptionBase
 */
    template <class exc>
    void issue_error_throw (const char *file,
			    int         line,
			    const char *function,
			    const char *cond,
			    const char *exc_name,
			    exc         e)
    {
				       // Fill the fields of the
				       // exception object
      e.SetFields (file, line, function, cond, exc_name);
      throw e;
    };
    

/**
 * Abort the program. This function is used so that we need not
 * include <cstdlib> into this file since it is included into all
 * other files of the library and we would like to keep its include
 * list as short as possible.
 */
    void abort ();

  };
  
};



#ifdef DEBUG  ////////////////////////////////////////

/**
 * This is the main routine in the exception mechanism for debug mode
 * error checking. It asserts that a certain condition is fulfilled,
 * otherwise issues an error and aborts the program.
 *
 * See the @p{ExceptionBase} class for more information.
 *
 * @see ExceptionBase
 * @author Wolfgang Bangerth, November 1997, extensions 1998
 */
#define Assert(cond, exc)                                           \
  {                                                                 \
    if (!(cond))                                                    \
      deal_II_exceptions::internals::                               \
      issue_error_assert_1 (__FILE__,                               \
			     __LINE__,                              \
			     __PRETTY_FUNCTION__, #cond, #exc, exc);\
  }


#else        ////////////////////////////////////////

#define Assert(cond, exc)                     \
  { }
#endif      ////////////////////////////////////////



/**
 * This is the main routine in the exception mechanism for run-time
 * mode error checking. It assert that a certain condition is
 * fulfilled, otherwise issues an error and aborts the program.
 *
 * On some systems (we only know of DEC Alpha systems running under
 * OSF1 or Linux), the compiler fails to compile the @p{AssertThrow}
 * macro properly, yielding an internal compiler error. We detect this
 * at configure time. For these cases, the @p{AssertThrow} macro aborts
 * the program if the assertion is not satisfied. This, however,
 * happens in debug and optimized mode likewise.  Note that in these
 * cases, the meaning of a program changes. In particular, one cannot
 * catch exceptions thrown by @p{AssertThrow}, but we did not find
 * another way to work around this compiler bug.
 *
 * See the @p{ExceptionBase} class for more information.
 *
 * @see ExceptionBase
 * @author Wolfgang Bangerth, November 1997, extensions 1998
 */
#ifndef DISABLE_ASSERT_THROW
#define AssertThrow(cond, exc)                                   \
  {                                                              \
    if (!(cond))                                                 \
      deal_II_exceptions::internals::                            \
      issue_error_throw (__FILE__,                               \
			 __LINE__,                               \
			 __PRETTY_FUNCTION__, #cond, #exc, exc); \
  }
#else
#define AssertThrow(cond, exc)                                    \
  {                                                               \
    if (!(cond))                                                  \
      deal_II_exceptions::internals::abort ();                    \
  }
#endif




/**
 * See the @p{ExceptionBase} class for a detailed description.
 *
 * @see ExceptionBase
 * @author Wolfgang Bangerth, November 1997
 */
#define DeclException0(Exception0)  \
class Exception0 :  public ExceptionBase {}



/**
  *  Declare an exception class with
  *  one additional parameter.
  *
  *  @see ExceptionBase
  */
#define DeclException1(Exception1, type1, outsequence)                \
class Exception1 : public ExceptionBase {                             \
  public:                                                             \
      Exception1 (const type1 a1) : arg1 (a1) {};                     \
      virtual ~Exception1 () throw () {};                             \
      virtual void PrintInfo (std::ostream &out) const {              \
        out outsequence << std::endl;                                 \
      };                                                              \
  private:                                                            \
      const type1 arg1;                                               \
}



/**
 *  Declare an exception class with
 *  two additional parameters.
 *
 *  @see ExceptionBase
 */
#define DeclException2(Exception2, type1, type2, outsequence)         \
class Exception2 : public ExceptionBase {                             \
  public:                                                             \
      Exception2 (const type1 a1, const type2 a2) :                   \
	      arg1 (a1), arg2(a2) {};                                 \
      virtual ~Exception2 () throw () {};                             \
      virtual void PrintInfo (std::ostream &out) const {              \
        out outsequence << std::endl;                                 \
      };                                                              \
  private:                                                            \
      const type1 arg1;                                               \
      const type2 arg2;                                               \
}



/**
 *  Declare an exception class with
 *  three additional parameters.
 *
 *  @see ExceptionBase
 */
#define DeclException3(Exception3, type1, type2, type3, outsequence)  \
class Exception3 : public ExceptionBase {                             \
  public:                                                             \
      Exception3 (const type1 a1, const type2 a2, const type3 a3) :   \
	      arg1 (a1), arg2(a2), arg3(a3) {};                       \
      virtual ~Exception3 () throw () {};                             \
      virtual void PrintInfo (std::ostream &out) const {              \
        out outsequence << std::endl;                                 \
      };                                                              \
  private:                                                            \
      const type1 arg1;                                               \
      const type2 arg2;                                               \
      const type3 arg3;                                               \
}



/**
 *  Declare an exception class with
 *  four additional parameters.
 *
 *  @see ExceptionBase
 */
#define DeclException4(Exception4, type1, type2, type3, type4, outsequence) \
class Exception4 : public ExceptionBase {                             \
  public:                                                             \
      Exception4 (const type1 a1, const type2 a2,                     \
	    const type3 a3, const type4 a4) :                         \
	      arg1 (a1), arg2(a2), arg3(a3), arg4(a4) {};             \
      virtual ~Exception4 () throw () {};                             \
      virtual void PrintInfo (std::ostream &out) const {              \
        out outsequence << std::endl;                                 \
      };                                                              \
  private:                                                            \
      const type1 arg1;                                               \
      const type2 arg2;                                               \
      const type3 arg3;                                               \
      const type4 arg4;                                               \
}



/**
 *  Declare an exception class with
 *  five additional parameters.
 *
 *  @see ExceptionBase
 */
#define DeclException5(Exception5, type1, type2, type3, type4, type5, outsequence) \
class Exception5 : public ExceptionBase {                             \
  public:                                                             \
      Exception5 (const type1 a1, const type2 a2, const type3 a3,     \
	    const type4 a4, const type5 a5) :                         \
	      arg1 (a1), arg2(a2), arg3(a3), arg4(a4), arg5(a5) {};   \
      virtual ~Exception5 () throw () {};                             \
      virtual void PrintInfo (std::ostream &out) const {              \
        out outsequence << std::endl;                                 \
      };                                                              \
  private:                                                            \
      const type1 arg1;                                               \
      const type2 arg2;                                               \
      const type3 arg3;                                               \
      const type4 arg4;                                               \
      const type5 arg5;                                               \
}



/**
 * Declare some exceptions that occur over and over. This way, you can
 * simply use these exceptions, instead of having to declare them
 * locally in your class. The namespace in which these exceptions are
 * declared is later included into the global namespace by @p{using
 * namespace StandardExceptions;}.
 */
namespace StandardExceptions 
{
				   /**
				    * Exception denoting a division by
				    * zero.
				    */
  DeclException0 (ExcDivideByZero);

				   /**
				    * Trying to allocate a new
				    * object failed due to lack of
				    * free memory.
				    */
  DeclException0 (ExcOutOfMemory);

				   /**
				    * An error occured reading or
				    * writing a file.
				    */
  DeclException0 (ExcIO);

				   /**
				    * An error occured opening the named file.
				    */
  DeclException1 (ExcFileNotOpen,
		  char*,
		  << "Could not open file " << arg1);

				   /**
				    * Exception denoting a part of the
				    * library or application program
				    * that has not yet been
				    * implemented. In many cases, this
				    * only indicates that there wasn't
				    * much need for something yet, not
				    * that this is difficult to
				    * implement. It is therefore quite
				    * worth the effort to take a look
				    * at the corresponding place and
				    * see whether it can be
				    * implemented without too much
				    * effort.
				    */
  DeclException0 (ExcNotImplemented);

				   /**
				    * This exception usually indicates
				    * that some condition which the
				    * programmer thinks must be
				    * satisfied at a certain point in
				    * an algorithm, is not
				    * fulfilled. This might be due to
				    * some programming error above,
				    * due to changes to the algorithm
				    * that did not preserve this
				    * assertion, or due to assumptions
				    * the programmer made that are not
				    * valid at all (i.e. the exception
				    * is thrown although there is no
				    * error here). Within the library,
				    * this exception is most often
				    * used when we write some kind of
				    * complicated algorithm and are
				    * not yet sure whether we got it
				    * right; we then put in assertions
				    * after each part of the algorithm
				    * that check for some conditions
				    * that should hold there, and
				    * throw an exception if they do
				    * not.
				    *
				    * We usually leave in these
				    * assertions even after we are
				    * confident that the
				    * implementation is correct, since
				    * if someone later changes or
				    * extends the algorithm, these
				    * exceptions will indicate to him
				    * if he violates assumptions that
				    * are used later in the
				    * algorithm. Furthermore, it
				    * sometimes happens that an
				    * algorithm does not work in very
				    * rare corner cases. These cases
				    * will then be trapped sooner or
				    * later by the exception, so that
				    * the algorithm can then be fixed
				    * for these cases as well.
				    */
  DeclException0 (ExcInternalError);

				   /**
				    * This exception is used in
				    * functions that may not be called
				    * (i.e. in pure functions) but
				    * could not be declared pure since
				    * the class is intended to be used
				    * anyway, even though the
				    * respective function may only be
				    * called if a derived class is
				    * used.
				    */
  DeclException0 (ExcPureFunctionCalled);

				   /**
				    * This exception is used if some
				    * object is found uninitialized.
				    */
  DeclException0 (ExcNotInitialized);

				   /**
				    * This exception is raised if a
				    * functionality is not possible in
				    * the given dimension. Mostly used
				    * to throw function calls in 1d.
				    */
  DeclException1 (ExcImpossibleInDim,
		  int,
		  << "Impossible in " << arg1 << "d.");

				   /**
				    * This exception is raised
				    * whenever the sizes of two
				    * objects were assumed to be
				    * equal, but were not.
				    */
  DeclException2 (ExcDimensionMismatch,
		  int, int,
		  << "Dimension " << arg1 << " not equal to " << arg2);

				   /**
				    * This exception is one of the
				    * most often used ones, and
				    * indicates that an index is not
				    * within the expected range. For
				    * example, you might try to access
				    * an element of a vector which
				    * does not exist.
				    */
  DeclException3 (ExcIndexRange, int, int, int,
		  << "Index " << arg1 << " is not in ["
		  << arg2 << "," << arg3 << "[");

				   /**
				    * This exception works around a
				    * design flaw in the
				    * @p{DeclException0} macro: that
				    * does not allow one to specify a
				    * message that is displayed when
				    * the exception is raised, as
				    * opposed to the other exceptions
				    * which allow to show a text along
				    * with the given parameters.
				    *
				    * When throwing this exception,
				    * you can give a message as a
				    * @p{char*} as argument to the
				    * exception that is then
				    * displayed.
				    */
  DeclException1 (ExcMessage, char*,
		  << arg1);

				   /**
				    * Exception used when running into
				    * functions that are only supported
				    * in a backward compatibility mode.
				    */
  DeclException1 (ExcCompatibility,
		  char*,
		  << "You are using a backward compatibility feature\n"
		  << "that you have disabled during configuration of\n"
		  << "the library by the --disable-compat="
		  << arg1 << " switch. You should either use an\n"
		  << "alternative function, or configure again without\n"
		  << "this switch and recompile the library.");
};


using namespace StandardExceptions;


#endif
