/**
cpp2cxx is an open source software distributed under terms of the
Apache2.0 licence.

Copyrights remain with the original copyright holders.
Use of this material is by permission and/or license.

Copyright [2012] Aditya Kumar, Andrew Sutton, Bjarne Stroustrup
          [2020] Thomas Figueroa
          
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef VALIDATORMAP_H
#define VALIDATORMAP_H

#include "ExceptionHandler.h"
#include "general_utilities/set_utils.hpp"

#include <fstream>
#include <set>
#include <string>


using ValidMacros_t = std::set<std::string>;

class ValidatorMap
{
public:
    // @TODO: Change this to a constructor for initialization.
    void InitValidatorMap(std::string const& v_file)
    {
        validator_file = v_file;
        std::ifstream p_file(validator_file);
        if(!p_file.is_open())
            throw ExceptionHandler("could not open the validator file");
        std::string dummy;
        // @TODO: Initialize this
        int val;
        while(p_file.good())
        {
            std::string macro_id;
            //#define macro_switch 1
            p_file >> dummy;    //#define
            p_file >> macro_id; //macro_id
            p_file >> val;      //replacement text
            validated_macros.insert(macro_id);
        }
    }

    ValidMacros_t const& GetValidMacros() const
    {
        return validated_macros;
    }

private:
    ValidMacros_t validated_macros;
    std::string validator_file;
};

#endif // VALIDATORMAP_H
