#ifndef __myClass_HPP__
#define __myClass_HPP__
#include "mex.h"

/*
 This file contains the description of our c++ class, including all its
 required procedures. It also includes a function which allows for the creation
 of a class instance from numerical data received from matlab, as well as a few
 output functions tailored for matlab.
*/


/* The class that we are interfacing to */
class myClass
{
public:
    /*  The main constructor */
    myClass() : valueR(0.0), valueI(0.0) {};

    /* The copy constructor */
    myClass(const myClass& a) : valueR(a.valueR), valueI(a.valueI) {};

    /* Construction from a matlab table */
    myClass(const mxArray* prhs) {
        /* Get the size and pointers to input data */
        mwSize m(mxGetM(prhs));
        mwSize n(mxGetN(prhs));
        double* pr(mxGetPr(prhs));
        double* pi(mxGetPi(prhs));
        int complex((pi==NULL ? 0 : 1));

        // If the input is real, we set the imaginary data to nothing
        if (complex == 0)
            valueI = 0.0;

        // We verify that the size is [1 1]
        if ((m != 1) || (n != 1))
            mexErrMsgIdAndTxt( "myClass:invalidSize", "Size [1 1] expected.");

        // Now we copy the data over (actually, only one item should be copied
        // here, but we keep the loop for when we'll like to copy tables)
        for (mwIndex j = 0; j < n; ++j) {
            // We first iterate on the columns
            for (mwIndex i = 0; i < m; ++i) {
                valueR = pr[i];
                if (complex) {
                    valueI = pi[i];
                }
            }
            pr += m;
            pi += m;
        }
    };

    /* Destructor */
    virtual ~myClass() {};

    /* Printing of the object in matlab */
    void display()
    {
        if (valueI == 0)
            mexPrintf("%f\n", valueR);
        else
            mexPrintf("%f + %fi\n", valueR, valueI);
    }

    /* Extracting the raw data: the following function extracts the internal
       data from the class, and puts it in a basic c-type table to be accessed
       by matlab */
    mxArray* toDouble()
    {
        /* Get the size and pointers to input data */
        mwSize m(1);
        mwSize n(1);
        int complex((valueI==0.0 ? 0 : 1));
        mxArray* plhs;
        if (complex)
            plhs = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxCOMPLEX);
        else
            plhs = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);

        // Now we copy the data over (actually, only one item should be copied
        // here, but we keep the loop for when we'll like to copy tables)
        double* pointerR(mxGetPr(plhs));
        double* pointerI(mxGetPi(plhs));
        for (mwIndex j = 0; j < n; ++j) {
            // We first iterate on the columns
            for (mwIndex i = 0; i < m; ++i) {
                pointerR[i] = valueR;
                if (complex) {
                    pointerI[i] = valueI;
                }
            }
            pointerR += m;
            pointerI += m;
        }
        return plhs;
    }

    /* Some class operators */
    myClass& operator+=(const myClass& b)
    {
        valueR += b.valueR;
        valueI += b.valueI;
        return *this;
    }

    myClass operator+(const myClass& b)
    {
        myClass result;
        result.valueR = valueR + b.valueR;
        result.valueI = valueI + b.valueI;
        return result;
    }

    /* The following function is a special implementation of the + operator,
       which is tailored for the matlab interface: it computes the sum of two
       numbers, like operator+, but puts the result in a new instance of the
       class that is allocated dynamically with the 'new' function. This allows
       the result of this operation to remain in memory between two hands out of
       the interface to matlab.
       
       Here, we keep track of the created object through a reference for optimal
       clarity. A version with pointers is also given below.
       
       Note that any number produced with this function should be
       either manually managed, or encapsulated into a matlab object (in which
       case matlab will take care of clearing it when it needs it no more).
       Failure to do so will result in non-freed memory (just like alocation of
       memory with pointers needs to be carefully taken care of).*/
    myClass& plus_new(const myClass& b)
    {
        myClass& result(*(new myClass));
        result.valueR = valueR + b.valueR;
        result.valueI = valueI + b.valueI;
        return result;
    }

    /* Here is the same procedure as above, but which returns a pointer to the
       newly created object rather than a reference (less c++-style, more in the
       c-like)*/
    myClass* plus_new_ptr(const myClass& b)
    {
        return &(this->plus_new(b));
        /*myClass* result(new myClass);
        result->valueR = valueR + b.valueR;
        result->valueI = valueI + b.valueI;
        return result;*/
    }


private:
    double valueR, valueI;
};

#endif // __myClass_HPP__
