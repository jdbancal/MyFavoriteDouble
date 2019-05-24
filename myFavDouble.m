% myFavDouble is a MATLAB class wrapper to the myClass C++ class
%
% It can be used as follows:
%   a = myFavDouble(3.5)
%   b = myFavDouble(2.5+1i)
%   c = a+b
%
% Note that the class here is a handle class. Therefore, calling 'd=c' will
% make a soft copy of c into d (both copies point to the same data in
% memory). Therefore, any procedure that modifies the internal data must
% first perform a copy of it.
classdef myFavDouble < handle
    properties (SetAccess = private, Hidden = true)
        objectIdentifier; % The identifier of the underlying C++ class instance (i.e. the integer version of the pointer to the handle_class instance containing a pointer to the object of interest)
    end
    methods

        %% Constructor
        % This function can be called to
        %  - Create a new C++ class instance
        %  - Create a new copy of a C++ instance (i.e. a new C++ object
        %    with same data)
        %  - Create a new C++ class instance from a matlab object
        %  - Create a myFavDouble object to encapsulate a reference to an
        %    already existing C++ class instance. For this the constructor
        %    should be called with two arguments : 'encapsulate', followed
        %    by the integer reference to the existing object to be
        %    encapsulated.
        function this = myFavDouble(varargin)
            if nargin == 0
                % Without further argument we construct a new empty instance
                this.objectIdentifier = myFavDouble_mex('new');
            elseif nargin == 1
                % Then we check that the argument is of the same class
                % before copying it
                if isequal(class(varargin{1}), 'myFavDouble')
                    % If it is an object of type 'myFavDouble', we interpret
                    % this call to a constructor as a call for copying the object
                    % into a new one
                    this.objectIdentifier = myFavDouble_mex('new', varargin{1}.objectIdentifier);
                else
                    % Then we interpret this call as a call for the library to
                    % create an instance of this class from some numerical
                    % matlab data (e.g. a numerical number). We thus transfer
                    % this data to the C++ library, and let it decide how to
                    % create a class instance from it. Upon completion, we
                    % return the newly created object containing this data.
                    this.objectIdentifier = myFavDouble_mex('newFromMatlab', varargin{1});
                end
            elseif nargin == 2
                if isequal(upper(varargin{1}),'ENCAPSULATE') && isequal(class(varargin{2}), 'uint64')
                    % If the second argument is of type 'uint64', then we interpret
                    % it as pointing to an existing instance of a C++ class, so we
                    % encapsulate it into the current instance of myFavDouble.
                    
                    % But since this should be a private constructor, we
                    % first we check that the caller is the current file
                    % (i.e. myFavDouble.m)
                    [ST I] = dbstack;
                    if ~isequal(ST(2).file, [mfilename, '.m']) && ~isequal(ST(2).file, [mfilename('fullpath'), '.m'])
                        error('Only myFavDouble.m is allowed to encapsulate an integer into a new myFavDouble object.');
                    end
                    
                    % This creates a matlab object which points to the C++
                    % object referred to by this number.
                    this.objectIdentifier = varargin{2};

                    % We check that the created object is valid (that it really points
                    % to a proper c++ object). Otherwise we produce an error.
                    if ~(this.checkIdentifierValidity)
                        this.objectIdentifier = 0; % We reset the handle...
                        error('Invalid reference given upon construction of a new myFavDouble object.');
                    end
                else
                    error('Wrong instruction upon creation of a new myFavDouble object');
                end
            else
                error('Too many parameters in the creation of a new myFavDouble object.');
            end
        end


        %% Destructor - Destroy the C++ class instance
        function delete(this)
            myFavDouble_mex('delete', this.objectIdentifier);
        end


        %% checkIdentifierValidity - checks whether the c++ pointer class to the
        % underlying c++ object that this class instance refers to exists in memory
        function result = checkIdentifierValidity(this, varargin)
            result = myFavDouble_mex('isValid', this.objectIdentifier, varargin{:});
        end


        %% Convert the data to a table of matlab double
        function result = double(this)
            result = myFavDouble_mex('double', this.objectIdentifier);
        end

        %% display function
        function display(this)
            disp(' ');
            disp([inputname(1), ' = ']);
            disp(' ');
            myFavDouble_mex('display', this.objectIdentifier);
            disp(' ');
        end

        %% Here are the save and load functions for this object
        function structure = saveobj(this)
            % This function returns a matlab structure that fully 
            % characterizes the current object
            structure.dataVersion = 1;
            structure.data = this.double;
        end
        
        % Since the load function does not depend on a class instance (but 
        % creates one), it needs to be a static method.    
    end
    methods (Static)
        function result = loadobj(structure)
            % The result should be an instance of myFavDouble with the data contained in the provided structure
            if structure.dataVersion > 1
                error('The object was saved with a newer version of the library. Please upgrade the library to load it again.');
            else
                % Then we create a new matlab object with the 
                result = myFavDouble(structure.data);
            end
        end
    end
    methods

        %% Now we continue with the other methods for this class

        %% plus - an example class method call
        function varargout = plus(this, varargin)
            % This is a function which involves a second instance of a similar object,
            % so we check if this second instance was also provided
            if length(varargin) ~= 1
                error('Wrong number of arguments in myFavDouble::plus');
            end

            % Now we call the addition procedure. Since the function creates a
            % new object with the result, we keep the corresponding handle...
            newObjectIdentifier = myFavDouble_mex('plus', this.objectIdentifier, varargin{1}.objectIdentifier);

            % ...  and create a new matlab object to keep this handle
            varargout{1} = myFavDouble('encapsulate', newObjectIdentifier);
        end

    end
end
