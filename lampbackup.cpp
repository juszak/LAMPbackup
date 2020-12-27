////////////////////////////////////////////////////////////////////////////////
// lampbackup - backup utility for sites using LAMP stack (Linux, Apache, 
//              MySQL/MariaDB, & PHP)
// Attempting to build for Windows as well (not just Linux), but Windows is not
// a requirement

// This is more-or-less an exercise being used as a backdrop to test out several
// other things right now (some CI/CD related capabilties). I have an existing 
// shell script that does what this C++ app will do. I'm not sure if 
// there's any advantage to writing this in C++, but it provides the backdrop I
// need for now.

#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <boost/program_options.hpp>
#include "config.h"

using namespace std;
using namespace std::chrono;
namespace po = boost::program_options;

string getDateTime()
{
  auto now = system_clock::now();
  auto nowC = system_clock::to_time_t(now);
  stringstream buffer;
  buffer << put_time(localtime(&nowC), "%Y-%m-%d-%H-%M-%S");
  return buffer.str();
}

void printVersion()
{
    cout << "lampbackup Version " 
      << lampbackup_VERSION_MAJOR << "."
      << lampbackup_VERSION_MINOR << "." 
      << lampbackup_VERSION_PATCH << endl;
}

int main(int argc, char* argv[])
{
  //------------------------------------------------------------
  // Print application information
  printVersion();

  // TODO: Verify presence of prerequisite tools
  //  * tar (proabably assumed on Linux systems)
  //  * gzip (might just skip compression)
  //  * mysqldump
  // Can probably meet this requirement via RPM packaging with dependencies
 
  //------------------------------------------------------------
  // Define variables needed for operation

  // MySQL/MariaDB database connection information
  string dbHost("");
  string dbName("");
  string dbUser("");
  string dbPass("");
  string websiteName("");
  string htmlDir("");
  string sslDir("");

  // Differentiating between Windows and Linux might be useful later
#ifdef _WIN32
  string tempDir("C:\\Windows\\Temp");
#else
  string tempDir("/tmp");
#endif

  // Supported options
  po::options_description desc("Usage");
  desc.add_options()
    ("help", "Print usage")
    ("dbHost", po::value<string>()->default_value("localhost"), "Hostname of MySQL/MariaDB server")
    ("dbName", po::value<string>(), "Name of MySQL/MariaDB database")
    ("dbUser", po::value<string>(), "Username for MySQL/MariaDB database")
    ("dbPass", po::value<string>(), "Password for MySQL/MariaDB database")
    ("websiteName", po::value<string>(), "Name of website (alphanumeric only, used in backup name)")
    ("htmlDir", po::value<string>(), "Root directory for PHP/HTML files")
    ("sslDir", po::value<string>(), "Root directory for SSL files")
    ("tempDir", po::value<string>()->default_value(tempDir), "Temporary directory")
    ;

  //------------------------------------------------------------
  // Parse the command line values into program options and validate

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help") || argc < 2) 
  {
    cout << desc << "\n";
    return 0;
  }

  // TODO: Verify temp directory is writable. Alternatively, use current working directory (verify it is writable as well)?

  dbHost = vm["dbHost"].as<string>();
 
  if (vm.count("dbName"))
  {
    dbName = vm["dbName"].as<string>();
  }
  else
  {
    cout << "dbName was not set. Exiting.";
    return 1;
  }

  if (vm.count("dbUser"))
  {
    dbUser = vm["dbUser"].as<string>();
  }
  else
  {
    cout << "dbUser was not set. Exiting.";
    return 1;
  }

  if (vm.count("dbPass"))
  {
    dbPass = vm["dbPass"].as<string>();
  }
  else
  {
    cout << "dbPass was not set. Exiting.";
    return 1;
  }

  if (vm.count("websiteName"))
  {
    websiteName = vm["websiteName"].as<string>();
    // Removew characters that are not alphanumeric
    websiteName.erase(std::remove_if(websiteName.begin(), websiteName.end(),
      [](char c) { return (!std::isalpha(c) && !std::isdigit(c)); }),
      websiteName.end());
  }
  else
  {
    cout << "websiteName was not set. Exiting.";
    return 1;
  }

  if (vm.count("htmlDir"))
  {
    htmlDir = vm["htmlDir"].as<string>();
  }
  else
  {
    cout << "htmlDir was not set. Exiting.";
    return 1;
  }

  if (vm.count("sslDir"))
  {
    sslDir = vm["sslDir"].as<string>();
  }
  else
  {
    cout << "sslDir was not set. Exiting.";
    return 1;
  }

  // Archive file name
  string archiveName(websiteName + "_" + getDateTime() + ".tar");

  //------------------------------------------------------------
  // Print the program configuration

  cout << "\n--------------------------------------------------------------------------------"
       << "\n                  CONFIGURATION"
       << "\nDatabase server hostname: " << dbHost
       << "\n           Database Name: " << dbName
       << "\n       Database Username: " << dbUser
       << "\n       Database Password: " << dbPass
       << "\n            Website name: " << websiteName
       << "\n     HTML root directory: " << htmlDir
       << "\n           SSL directory: " << sslDir
       << "\n       Archive file name: " << archiveName
       << "\n     Temporary directory: " << tempDir
    << "\n--------------------------------------------------------------------------------"
       << endl;

  // TODO:
  //   * Attempt to stop httpd?
  //   * Save the current working directory as originalWorkingDir 
  //   * Create a root working backup directory structure at tempDir/archiveName
  //     (e.g. mkdir -p tempdir/archiveName/html tempdir/archiveName/ssl)
  //   * Copy files from htmlDir to tempdir/archiveName/html
  //     (e.g. cp -fr htmlDir/* tempdir/archiveName/html/)
  //     Copy hidden files too (e.g. .htaccess)
  //   * Copy files from sslDir to tempdir/archiveName/ssl
  //     (e.g. cp -fr sslDir/* tempdir/archiveName/ssl/)
  //   * Create mysql dump file of database in tempdir/archiveName/
  //     (e.g. cd tempdir/archiveName && mysqldump --password=dbPass --user=dbUser --host=dbHost --add-drop-table dbName > dbName.sql)
  //   * Archive and compress working directory
  //     (e.g. cd tempDir && tar -czf archiveName.tar.gz archiveName/)
  //   * Copy archiveName.tar.gz to originalWorkingDir
  //   * (e.g. cd tempDir && rm -fr archiveName)
  //   * Consider option to specify output directory
 }