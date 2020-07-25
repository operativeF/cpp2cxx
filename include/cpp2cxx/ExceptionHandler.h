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

#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include "DemacBoostWaveIncludes.h"

#include <exception>
#include <sstream>
#include <string>

/**
 * @class ExceptionHandler
 * To handle exceptions
 * @TODO: Change the name to exception
 * Handle exception thrown as ExceptionHandler(const PPMacro* mac, std::string msg)
 */
class ExceptionHandler : public std::exception
{
public:
    ExceptionHandler() : message("Unknown Exception")
    {
    }

    ExceptionHandler(int i) : error_code(i)
    {
    }

    ExceptionHandler(std::string msg) : message(msg)
    {
    }

    ExceptionHandler(token_type const& tok, std::string msg)
    {
        std::stringstream err_msg;
        err_msg << "  - exception note: ";
        err_msg //<<tok.get_position().get_file()<<":"
                << tok.get_position().get_line() << ":" << tok.get_position().get_column() << ": ";
        err_msg << msg << "\n";
        message = err_msg.str();
    }

    /*
  ExceptionHandler(const PPMacro* mac, std::string msg)
  {
    std::stringstream err_msg;
    token_type tok = mac->get_identifier();
    err_msg <<tok.get_position().get_file()<<":"
            <<tok.get_position().get_line()<<":"
            <<tok.get_position().get_column()<<":";
    err_msg << msg;
    message = err_msg.str();
  }
*/
    inline const std::string& GetMessage() const
    {
        return message;
    }
    inline int GetErrorCode()
    {
        return error_code;
    }

private:
    std::string message;
    int error_code;
};

#endif /*EXCEPTIONHANDLER_H*/
