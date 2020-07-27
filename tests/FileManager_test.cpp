#include <catch2/catch.hpp>

#include "cpp2cxx/FileManager.h"

TEST_CASE( "Manages the files that will be demacrofied.", "[FileManager]" )
{
    SECTION( "Verify correct behavior for invalid file inputs." )
    {
        SECTION( "Invalid log file." )
        {
            // Log is read-only.
            // Log doesn't exist.
        }

        SECTION( "Invalid Demacrofied Macro Stat file." )
        {

        }

        SECTION( "Invalid Macro Stat file." )
        {

        }

        SECTION( "Invalid input files." )
        {

        }

        SECTION( "Invalid output files." )
        {

        }

        SECTION( "Invalid search paths." )
        {

        }

        SECTION( "Invalid input directory." )
        {

        }

        SECTION( "Invalid output directory." )
        {

        }

        SECTION( "Invalid backup directory." )
        {

        }

        SECTION( "Invalid cleanup directory." )
        {

        }

        SECTION( "Invalid validator file." )
        {

        }
    }

    SECTION( "Verify file and directory creation mechanisms." )
    {

    }
}