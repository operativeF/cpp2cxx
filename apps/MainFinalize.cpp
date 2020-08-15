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


/**
 *  @file Main.cpp
 *  @brief main file of the project
 *  @version 1.0
 *  @author Aditya Kumar
 *  @brief takes in the config file, generates configuration scheme for
 *  other classes and then calls the class Overseer.
 *  catches all possible unhandled exceptions and displayes them to user
 *  compiles with g++-4.5 or higher,
 *  for compiling pass -std=c++0x to the compiler
 */

#include "cpp2cxx/ConfigScheme.h"
#include "cpp2cxx/ExceptionHandler.h"
#include "cpp2cxx/Overseer.h"

//#include "PrintVector.h"

#include <boost/program_options.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace po = boost::program_options;

/**
  * @main

  typical command line argument:
  ./demacrofier configFile.cfg

  ./a.out -i ifile.cpp -o ofile.cpp --global-macros-formatted=gMacros.dat\
   --input-directory=. --output-directory=demac_dir --log-file=log.txt\
   -p __FILE__ -p __LINE__ --backup-directory=backup
  */

/// @todo add object like, fucntion like, open,closed, config etc..
/// to the macro_stat_file
int main(int argc, char* argv[])
{
    //configuration file --to be loaded instead of the command line parameters
    std::string config_file;
    if(argc < 2)
    {
        config_file = "ConfigFile.cfg";
        //std::cerr<<"usage: config-file\n";
        //return -1;
    }
    else
    {
        config_file = argv[1];
    }

    /**     ***********   BEGIN PROGRAM OPTIONS   ************/

    try
    {

        //the default file where the errors and warnings will be put to
        std::ostream* err_outstream = &std::cerr;
        std::ofstream plog_file;
        //the default file where the demacrofied macros will be listed
        std::ostream* stat_outstream = &std::cerr;
        std::ofstream pstat_file;
        std::ofstream mac_stat_file;
        std::vector<std::filesystem::path> input_files;
        std::vector<std::filesystem::path> output_files;
        std::vector<std::filesystem::path> search_paths;
        //the file where all the macros will be listed
        std::filesystem::path macro_list_file;
        std::filesystem::path log_file;
        std::filesystem::path stat_file;
        std::filesystem::path validator_file;
        std::filesystem::path input_directory;
        std::filesystem::path output_directory;
        std::filesystem::path cleanup_directory;
        std::filesystem::path backup_directory;
        bool enable_warning = false;
        bool no_translate = false;
        bool cleanup = false;
        bool multiple_definitions = false;
        std::string make_command;
        std::vector<std::string> macros_preventing_demacrofication;
        std::string demacrofication_granularity;
        std::string global_macros_raw;
        std::string global_macros_formatted;

        po::options_description config_file_options("Allowed options");
        config_file_options.add_options()("help,h", "produce help message")("verbose,v",
                po::value<int>()->implicit_value(1), //not used
                "enable verbosity (optionally specify level)")("enable-warning",
                po::value<bool>(&enable_warning),
                "display warning message")("no-translate", po::value<bool>(&no_translate),
                "only analyze the macros without translation")("cleanup", po::value<bool>(&cleanup),
                "perform cleanup based on already validated refactorings")("mul-def",
                po::value<bool>(&multiple_definitions),
                "allow demacrofication when multiple definitions of macros are allowed")(
                "input-file,i", po::value<std::vector<std::filesystem::path>>(&input_files), "input file")
                ///@todo if output file is null then the output will be printed on the screen
                ///check this
                ("output-file,o", po::value<std::vector<std::filesystem::path>>(&output_files),
                        "output file")("search-path,sp",
                        po::value<std::vector<std::filesystem::path>>(&search_paths),
                        "paths where the clang front end will search for header files")(
                        "macro-stat-file", po::value<std::filesystem::path>(&macro_list_file),
                        "file having all the macros listed for statistics")("log-file,l",
                        po::value<std::filesystem::path>(&log_file),
                        "file where the error and warnings will be logged")("stat-file,s",
                        po::value<std::filesystem::path>(&stat_file),
                        "file where the list of macros processed will be logged along with other details")(
                        "validator-file,s",
                        po::value<std::filesystem::path>(&validator_file)->default_value("Defined.h"),
                        "File which has been generated by the Validator script. It should contain all the macro-switches which needs to be finally replaced")(
                        "ignore-macros,p",
                        po::value<std::vector<std::string>>(&macros_preventing_demacrofication),
                        "those predefined macros if they are in the replacement text of any other macro will prevent it from being demacrofied")(
                        "global-macros-formatted", po::value<std::string>(&global_macros_formatted),
                        "file containing global macros formatted in following way\n<Macro-id>\n<Replacement-list>")(
                        "global-macros-raw", po::value<std::string>(&global_macros_raw),
                        "file containing global macros")("demacrofication-granularity",
                        po::value<std::string>(&demacrofication_granularity)
                                ->default_value("OneFileAtATime"),
                        "granularity of demacrofication, either OneFileAtATime or OneMacroAtATime")(
                        "input-directory",
                        po::value<std::filesystem::path>(&input_directory)->default_value("."),
                        "input directory")("output-directory",
                        po::value<std::filesystem::path>(&output_directory)->default_value("dm_dir"),
                        "the directory where translated files should be put to")(
                        "cleanup-directory",
                        po::value<std::filesystem::path>(&cleanup_directory)->default_value("demac_cleanup"),
                        "the directory where (finally) demacrofied files should be put to")(
                        "backup-directory",
                        po::value<std::filesystem::path>(&backup_directory)->default_value("demac_backup"),
                        "the directory where source files should be copied for backup, this directory will be deleted later")(
                        "make-command",
                        po::value<std::string>(&make_command)->default_value("make"),
                        "the command which needs to be invoked in the shell to compile the project");

        //po::positional_options_description p;
        //p.add("input-file", -1);

        po::variables_map vm;
        std::ifstream cfg(config_file, std::ios_base::in);
        if(!cfg.is_open())
        {
            std::cerr << "can not open config file: " << config_file << "\n";
            return -1;
        }


        po::store(po::parse_config_file(cfg, config_file_options), vm);
        po::notify(vm);


        /*
    po::store(po::command_line_parser(argc, argv).
              options(config_file_options).positional(p).run(), vm);
              */
        po::notify(vm);

        if(vm.count("help"))
        {
            fmt::print(
                    "usage: <Executable> [options]\natleast input file is a must.\n"
                    "If no output file is specified then the output "
                    "will be redirected to the standard output\n{}",
                    config_file_options);
            return 0;
        }

        if(vm.count("no-translate"))
        {
            if(no_translate)
            {
                fmt::print(
                        "Processing the macros just for analysis.\n"
                        "-- If you want to translate, set option no-translate to false.\n");
            }
            else
            {
                fmt::print(
                        "Processing and translating the macros.\n"
                        "-- if you don't want to translate, set option no-translate to true.\n");
            }
        }

        if(vm.count("cleanup"))
        {
            if(no_translate)
            {
                std::cerr
                        << "cleanup cannot proceed when no_translate is set to true.\n"
                        << "as cleanup also involves translation from macros to C++11 declarations\n"
                        << "-- set the no_translate to false, to proceed with cleanup\n\n\n";
                return 0;
            }
            if(cleanup)
            {
                std::cout << "Warning! finally replacing the macros with C++11 declarations\n"
                          << "-- this process cannot be reverted\n\n\n";
            }
            else
            {
                std::cout
                        << "Error! Set the cleanup to true in the config-file to finalize the translations\n";
                return 0;
            }
        }

        if(vm.count("input-file"))
        {
            /*
      input_files = vm["input-file"].as<std::vector<std::string> >();
      std::cout << "Input files are: "
                << vm["input-file"].as<std::vector<std::string> >()<< "\n";
      */
            /// @todo check if prefixing the output directory path to the output
            /// file name is useful or not
            std::for_each(input_files.begin(), input_files.end(),
                    [input_directory](std::filesystem::path& input_file) {
                        input_file = input_directory / input_file;
                    });
            fmt::print("Input files are:\n");

            for(auto&& input_file : input_files)
            {
                fmt::print("{}\n", input_file.string());
            }

            //fmt::print("Input files are:\n{}\n", input_files);
        }
        else
        {
            fmt::print(std::cerr, "Error: no input file specified. Exiting...\n{}",
                    config_file_options);
            return -1;
        }

        //during the cleanup process the output files are taken corresponding
        //to the cleanup files
        if(vm.count("output-file"))
        {
            //if output files are given then total number of output files
            //should be equal to the total number of input files
            if(!input_files.empty() && (input_files.size() != output_files.size()))
            {
                fmt::print(std::cerr, "Number of input files and output files are not equal.\n");
                return -1;
            }

            if(cleanup)
            {
                output_directory = cleanup_directory;
            }

            /// @todo check if prefixing the output directory path to the output
            /// file name is useful or not
            std::for_each(output_files.begin(), output_files.end(),
                    [output_directory](std::filesystem::path& output_file) {
                        output_file = output_directory / output_file;
                    });
                    
            fmt::print("Output files are:\n");
            for(auto&& output_file : output_files)
            {
                fmt::print("{}\n", output_file.string());
            }
            //fmt::print("Output files are:\n{}\n", output_files);
        }
        else
        {
            fmt::print("warning: no output filename specified."
                       "output shall be redirected to the standard output\n");
            output_files.resize(input_files.size());
        }

        if(vm.count("search-path"))
        {
            // FIXME: Overload for vector of paths.
            fmt::print("Search paths:\n");

            for(auto&& path : search_paths)
            {
                fmt::print("{}\n", path.string());
            }
            //fmt::print("Search paths:\n{}", search_paths);
        }
        else
        {
            fmt::print(std::cerr,
                    "No search path provided the tool will search only"
                    " in the present directory\n");
        }

        if(vm.count("macro-stat-file"))
        {
            macro_list_file = output_directory / macro_list_file;
            mac_stat_file.open(macro_list_file, std::ios_base::out);
            if(!mac_stat_file.is_open())
            {
                fmt::print(std::cerr,
                        "File to list macros: {} could not be opened."
                        "Redirecting the list to std::cerr.",
                        macro_list_file.string());
            }
        }

        if(vm.count("log-file"))
        {
            log_file = output_directory / log_file;
            plog_file.open(log_file, std::ios_base::out);
            if(plog_file.is_open())
            {
                err_outstream = &plog_file;
            }
            else
            {
                fmt::print(std::cerr,
                        "log-file: {} could not be opened."
                        "Redirecting the errors / warnings to std::cerr.",
                        log_file.string());
            }
        }

        if(vm.count("stat-file"))
        {
            stat_file = output_directory / stat_file;
            pstat_file.open(stat_file, std::ios_base::out);
            if(pstat_file.is_open())
            {
                stat_outstream = &pstat_file;
            }
            else
            {
                fmt::print(std::cerr,
                        "stat-file: {} could not be opened."
                        "Redirecting the errors / warnings to std::cerr.",
                        stat_file);
            }
        }

        if(vm.count("validator-file"))
        {
            // if reading of validator_file is successful
            fmt::print("The validator file is: {}\n", validator_file.string());
        }

        if(vm.count("global-macros-raw"))
        {
            //global_macros_raw = vm["global-macros-raw"].as<std::string>();
            fmt::print(
                    "File containing the global macros is: {}"
                    "\nThe global macros will be kept after being parsed in"
                    "file gMacros.dat in a formatted form. This file can be"
                    "used later to avoid parsing of global macros after every run"
                    "when no new global macro have been added.\n",
                    global_macros_raw);
        }

#ifdef BUILD_NEW_MACRO_LIST
        else
        {
            std::cerr << "when BUILD_NEW_MACRO_LIST is defined,"
                      << "a file containing global macros"
                      << "should be specified. exiting...\n";
            std::cerr << config_file_options;
            return -1;
        }
#endif

        if(vm.count("global-macros-formatted"))
        {
            //global_macros_formatted = vm["global-macros-formatted"].as<std::string>();
            fmt::print("File containing formatted global macros is: {}\n", global_macros_formatted);
        }

#ifndef BUILD_NEW_MACRO_LIST
        else
        {
            std::cerr << "when BUILD_NEW_MACRO_LIST is not defined,"
                      << "a file containing global formatted macros"
                      << "should be specified. exiting...\n";
            std::cerr << config_file_options;
            return -1;
        }
#endif

        if(vm.count("input-directory"))
        {
            //input_directory = vm["input-directory"].as<std::string>();
            fmt::print("Files are to be taken from directory: {}\n", input_directory.string());
        }

        if(vm.count("output-directory"))
        {
            //output_directory = vm["output-directory"].as<std::string>();
            fmt::print("Files will be placed in directory: {}\n", output_directory.string());
        }

        // FIXME: No implemented.
        // if(vm.count("verbose"))
        // {
        //     std::cout << "verbosity enabled.  Level is " << vm["verbose"].as<int>() << "\n";
        // }

        if(vm.count("ignore-macros"))
        {
            fmt::print("Macros to prevent demacrofication are:\n{}\n",
                    macros_preventing_demacrofication);
        }
        
        if(vm.count("backup-directory"))
        {
            fmt::print("Backup directory: {}\n", backup_directory.string());
        }

        if(vm.count("cleanup-directory"))
        {
            fmt::print("Cleanup directory: {}\n", cleanup_directory.string());
        }

        if(vm.count("make-command"))
        {
            fmt::print("Make command: {}\n", make_command);
        }
        else
        {
            fmt::print(
                    "No make command/script provided. "
                    "default command `make` will be used\n");
        }

        if(vm.count("demacrofication-granularity"))
        {
            if((demacrofication_granularity != "OneFileAtATime")
                    && (demacrofication_granularity != "OneMacroAtATime"))
            {
                fmt::print(std::cerr, "Error: invalid value for demacrofication granularity.\n{}",
                        config_file_options);
                return -1;
            }

            fmt::print("Demacrofying: {}\n", demacrofication_granularity);
        }

        /**     ***********     END PRORAM OPTIONS   ************/


        ConfigScheme pConfigScheme;

        //set file manager scheme should be called before
        //the set demcrofication scheme because the demacrofication scheme
        //uses the names of files (e.g. validator_file)
        pConfigScheme.SetFileManagerScheme(input_files, output_files, search_paths, input_directory,
                output_directory, backup_directory, cleanup_directory, validator_file,
                err_outstream, stat_outstream, &mac_stat_file);
        pConfigScheme.SetDemacroficationScheme(demacrofication_granularity,
                macros_preventing_demacrofication, enable_warning, global_macros_raw,
                global_macros_formatted, multiple_definitions, cleanup);
        pConfigScheme.SetBuildScheme(make_command);

        //step1. load the Overseer class with the configuration scheme
        auto pOverseer = Overseer(std::move(pConfigScheme));
        //step2. start processing
        pOverseer.StartProcessing(!no_translate);
        
        fmt::print("\nFinished...\n");

        ///@todo when global_macros_formatted is provided use that one
        plog_file.close();
        pstat_file.close();
    }
    catch(const char* s)
    {
        fmt::print(std::cerr, "Error: {}\n", s);
    }
    catch(ExceptionHandler& e)
    {
        fmt::print(std::cerr, "Exception Handler: {}", e.GetExMessage());
    }
    catch(const boost::program_options::multiple_occurrences& e)
    {
        fmt::print(std::cerr, "{} from option: {}\n", e.what(), e.get_option_name());
    }
    catch(std::exception& e)
    {
        fmt::print(std::cerr, "std::exception: {}", e.what());
    }
    catch(...)
    {
        fmt::print(std::cerr, "Unknown error in the program\n");
    }

    return 0;
}
