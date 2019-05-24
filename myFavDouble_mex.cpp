#include "mex.h"
#include "class_handle.hpp"
#include "myClass.hpp"

/*
  This files receives instructions from matlab about which operation should be
  performed with the c++ library, and it calls the class to execute these
  instructions.

  Note that classes object going to matlab need to be allocated dynamically
  (otherwise they won't survive between two calls to this interface). Some
  special care is thus taken when dealing with these objects (such as calling
  functions ending with "_new" that allocate memory for such objects). Doing so
  allows one to rely on different parts of the code for memory management. Apart
  from the object creation, the only parts of the code which require manipulating
  pointers are in the class_handle.hpp file. We thus choose to deal here with
  references to these objects, rather than pointers. This allows for a cleaner
  code and more reliable memory management.
*/


// The following function is supposed to deal with all the memory allocation
// by itself.
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    /* Get the command string */
    char cmd[64];
    if (nrhs < 1 || mxGetString(prhs[0], cmd, sizeof(cmd)))
        mexErrMsgTxt("First input should be a command string less than 64 characters long.");

    /* New */
    if (!strcmp("new", cmd)) {
        // Check parameters
        if (nlhs != 1)
            mexErrMsgTxt("New: One output expected.");
        if (nrhs == 1)
        {
            // Called with no option, return the identifier to a new C++ instance:
            // for this, we send a reference to a new instance to the createMatlabIdFromObj
            // function.
            plhs[0] = createMatlabIdFromObj<myClass>(*(new myClass));
        }
        else if (nrhs == 2)
        {
            // Constructor called with one additional argument; we will call
            // the copy constructor: return a copy of the provided C++ instance

            // First, we restore the object to be copied from the matlab integer
            myClass& myClass_instance = recoverObjFromMatlabId<myClass>(prhs[1]);

            // Now we create a copy of this object
            myClass& copiedInstance(*(new myClass(myClass_instance)));

            //And return it (in the form of a matlab reference to it)
            plhs[0] = createMatlabIdFromObj<myClass>(copiedInstance);
        }
        else
            mexErrMsgTxt("New: Too many arguments.");
        return;
    }

    /* New from Matlab array */
    if (!strcmp("newFromMatlab", cmd)) {
        // Check parameters

        /* Check for proper number of input and output arguments */
        if (nrhs != 2) {
            mexErrMsgIdAndTxt( "myClass:invalidNumInputs",
                    "One input argument required.");
        }
        if(nlhs > 1){
            mexErrMsgIdAndTxt( "myClass:maxlhs",
                    "Too many output arguments.");
        }

        /* Check data type of input argument  */
        if (!(mxIsDouble(prhs[1]))){
            mexErrMsgIdAndTxt( "myClass:inputNotDouble",
                    "Input argument must be of type double.");
        }

        if (mxGetNumberOfDimensions(prhs[1]) != 2){
            mexErrMsgIdAndTxt( "myClass:inputNot2D",
                    "Input argument must be two dimensional\n");
        }

        // We now forward the matlab pointer to the data to the constructor
        // of myClass to let it copy the data into a new object. We then
        // send back the identifier of the newly created object to matlab.
        plhs[0] = createMatlabIdFromObj<myClass>(*(new myClass(prhs[1])));
        return;
    }

    /* Delete */
    if (!strcmp("delete", cmd)) {
        // Check parameters
        if (nrhs != 2)
            mexErrMsgTxt("Delete: Wrong number of arguments.");

        // Destroy the C++ object
        destroyObject<myClass>(prhs[1]);

        return;
    }

    // If we reached here, then there must be a second input parameter
    if (nrhs < 2)
	   mexErrMsgTxt("Second input not found.");

    // We reactivate the class instance referenced by the second input
    // All procedures below don't need it, but it costs virtually nothing.
    myClass& myClass_instance = recoverObjFromMatlabId<myClass>(prhs[1]);


    /* isValid */
    if (!strcmp("isValid", cmd)) {
        // Check parameters
        if (nlhs < 1 || nrhs > 2)
            mexErrMsgTxt("isValid: Unexpected arguments.");

        // We allocate space for the result
        plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);

        // We check where the output data should be places
        int* outputMatrix = (int*)mxGetData(plhs[0]);

        // Call the method
        bool validity(checkValidity<myClass>(prhs[1]));

        // And stock it at the right place
        if (validity)
            outputMatrix[0] = 1;
        else
            outputMatrix[0] = 0;
        return;
    }


    /* display */
    if (!strcmp("display", cmd)) {
        myClass_instance.display();
        return;
    }


    /* Extract a matlab table */
    if (!strcmp("double", cmd)) {
        // We check that an outcome is expected...
        if (nlhs != 1)
            mexErrMsgTxt("double: Unexpected arguments.");

        plhs[0] = myClass_instance.toDouble();
        return;
    }


    // Now come the functions involving a second instance as well.
    // We reactivate this object from identifier present in the third input.
    myClass& myClass_instance2 = recoverObjFromMatlabId<myClass>(prhs[2]);


    /* Call the class method "plus" */
    if (!strcmp("plus", cmd)) {
        // Check parameters
        if (nlhs < 1 || nrhs < 2)
            mexErrMsgTxt("plus: Unexpected arguments.");


        /* The object to be returned to matlab needs to be created explicitely
        by hand (otherwise its memory would be dis-allocated upon exiting of
        this function). Therefore, we need to do things in a slightly unusual
        way here. Below are three examples of how to do that. */
        if (true) {
            /* Here we use the the special addition function defined in our
            class, that also creates a new object. We keep track of the result
            produced with the help of a reference object.*/
            myClass& result(myClass_instance.plus_new(myClass_instance2));

            // Now we can return the reference to this object to matlab
            plhs[0] = createMatlabIdFromObj<myClass>(result);

        } else if (false) {
            /* Alternatively, we could create a new object first as a copy of the
            first one, and apply operations on it. If the first object is much
            larger than the result, this could lead to a less efficient procedure,
            because the entire first object will be copied here. */

            // The object is created here by reference.
            myClass& result(*(new myClass(myClass_instance)));

            // Now we add the other object
            result += myClass_instance2;

            // We return the reference to this object to matlab
            plhs[0] = createMatlabIdFromObj<myClass>(result);

        } else if (false) {
            /* Finally, here is another way to achieve the same result with pointers. */

            // We define a pointer
            myClass* result;

            // We assign it the result of the addition, taking care again to call a
            // special addition procedure that will allocate memory to create a new
            // object with the result of the operation.
            result = myClass_instance.plus_new_ptr(myClass_instance2);

            // Now we can return the reference to this object to matlab
            plhs[0] = createMatlabIdFromPtr<myClass>(result);
        }

        return;
    }

    // Got here, so command not recognized
    mexErrMsgTxt("Command not recognized.");
}
