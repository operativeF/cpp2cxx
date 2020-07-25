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


#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <map>
#include <string>

template<typename T>
std::ostream& operator<<(std::ostream& os,std::set<T> const& l)
{
  std::for_each(l.begin(),l.end(),
      [&os](T t){
        os<<"  - "<<t<<"\n";
      });
return os;
}

// this files only requires list of files from grep and aftre that a little preprocessing
// grep '#include' * > files_list
// it outputs in following format
// file_name: #include "header.h" or <header.h>
// and this program takes format
// file_name header.h
// so all it requires is:
//  a. removal of #include,
//  b. removal of quotes/ angle brackets
//  c. removal of the colon(:) sign
int main()
{
  //the files is represented as follows
  //file1 file2
  //that means file1 includes file2 as header
  const char* file_name = "files_list";
  std::ifstream fp(file_name);
  if(!fp.is_open())
    return -1;

  std::pair<std::string, std::string> file_include_pair;
  std::map<std::string, std::set<std::string>> dep_list;

  while(fp.good()){
    fp >> file_include_pair.first;
    fp >> file_include_pair.second;
    //what we want is an index on header file
    //so that we know how many files include this particular header-file
    dep_list[file_include_pair.second].insert(file_include_pair.first);
  }

  //print the list
  //make sure the dm_dir is already there
  std::ofstream ofp("dm_dir/include_dependency.yaml");

  if(!ofp.is_open())
    return -1;
  std::map<std::string, std::set<std::string> >::iterator dep_list_iter;
  for_each(dep_list.begin(), dep_list.end(),
      [&ofp](std::pair<std::string, std::set<std::string> > p){
        std::cout<<"\nPrinting "<<p.first<<" and "<<p.second;
        ofp<<p.first<<":\n"<<p.second<<"\n";
      });
return 0;
}
