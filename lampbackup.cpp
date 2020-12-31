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
#include <filesystem>
#include <boost/program_options.hpp>
#include "config.h"

using namespace std;
using namespace std::chrono;
namespace po = boost::program_options;
namespace fs = std::filesystem;

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
  fs::path htmlDir;
  fs::path sslDir;

  // Differentiating between Windows and Linux might be useful later
#ifdef _WIN32
  fs::path tempDir = fs::canonical("C:\\Windows\\Temp");
#else
  fs::path tempDir = fs::canonical("/tmp");
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
    ("tempDir", po::value<string>()->default_value(tempDir.string()), "Temporary directory")
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
    cout << "dbName was not set. Exiting." << endl;
    return 1;
  }

  if (vm.count("dbUser"))
  {
    dbUser = vm["dbUser"].as<string>();
  }
  else
  {
    cout << "dbUser was not set. Exiting." << endl;
    return 1;
  }

  if (vm.count("dbPass"))
  {
    dbPass = vm["dbPass"].as<string>();
  }
  else
  {
    cout << "dbPass was not set. Exiting." << endl;
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
    cout << "websiteName was not set. Exiting." << endl;
    return 1;
  }

  if (vm.count("htmlDir"))
  {
    try 
    {
      htmlDir = fs::canonical(vm["htmlDir"].as<string>());
    }
    catch(const exception & e)
    {
      cerr << "ERROR: Could find HTML directory " << vm["htmlDir"].as<string>() 
           << " (" << e.what() << ")" << endl;
      return 1;
    }
  }
  else
  {
    cout << "htmlDir was not set. Exiting." << endl;
    return 1;
  }

  if (vm.count("sslDir"))
  {
    try
    {
      sslDir = fs::canonical(vm["sslDir"].as<string>());
    }
    catch(const exception & e)
    {
      cerr << "ERROR: Could find SSL directory " << vm["sslDir"].as<string>() 
           << " (" << e.what() << ")" << endl;
      return 1;
    }
  }
  else
  {
    cout << "sslDir was not set. Exiting." << endl;
    return 1;
  }

  // Archive file name
  string archiveName(websiteName + "_" + getDateTime());

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
       << "\n            Archive name: " << archiveName
       << "\n     Temporary directory: " << tempDir
       << "\n--------------------------------------------------------------------------------"
       << endl;

  // TODO:
  //   * Attempt to stop httpd?

  // Save the current directory as originalDir
  fs::path originalDir = fs::current_path();
  cout << "Original directory: " << originalDir << endl;

  // Create a staging directory structure at tempDir/archiveName
  cout << "Changing directory to " << tempDir << endl;
  fs::current_path(tempDir);
  cout << "Current directory: " << fs::current_path().string() << endl;
  cout << "Creating staging at " 
       << fs::current_path().string() << "/" << archiveName << endl;
  fs::remove(archiveName); // This is probably never needed
  fs::create_directory(archiveName);
  fs::path stagingDir = fs::canonical(archiveName);
  cout << "Changing directory to " << stagingDir.string() << endl;
  fs::current_path(stagingDir);
  cout << "Current directory: " << fs::current_path().string() << endl;
  cout << "Creating html and ssl subdirectories" << endl;
  fs::create_directory("html");
  fs::create_directory("ssl");

  //   * Copy files from htmlDir to tempdir/archiveName/html
  //     (e.g. cp -fr htmlDir/* tempdir/archiveName/html/)
  //string fromPath(originalDir.string() + "/" + htmlDir + "/");
  fs::path toPath = fs::canonical(stagingDir.string() + "/html/");
  try
  {
    cerr << "Copying files from " << htmlDir.string() << " to " << toPath.string() << endl;
    fs::copy(htmlDir, toPath, fs::copy_options::recursive);
  }
  catch(const exception & e)
  {
    cerr << "ERROR: Could not copy files from " << htmlDir.string() << " to " << toPath.string()
         << " (" << e.what() << ")" << endl;
    return 1;
  }

  // TODO:
  //    * Check for hidden files to copy (e.g. .htaccess)
  //   * Copy files from sslDir to tempdir/archiveName/ssl
  //     (e.g. cp -fr sslDir/* tempdir/archiveName/ssl/)
  //   * Create mysql dump file of database in tempdir/archiveName/
  //     (e.g. cd tempdir/archiveName && mysqldump --password=dbPass --user=dbUser --host=dbHost --add-drop-table dbName > dbName.sql)
  //   * Archive and compress working directory
  //     (e.g. cd tempDir && tar -czf archiveName.tar.gz archiveName/)
  //   * Copy archiveName.tar.gz to originalWorkingDir
  //   * (e.g. cd tempDir && rm -fr archiveName)
  //   * Consider option to specify output directory

  cout << "Removing staging directory at " << stagingDir.string() << endl;
  try
  {
    fs::remove_all(stagingDir);
  }
  catch(const exception & e)
  {
    cerr << "ERROR: Could remove staging directory " << stagingDir.string() 
        << " (" << e.what() << ")" << endl;
    return 1;
  }
   
  // End of main() function
  return 0;
}