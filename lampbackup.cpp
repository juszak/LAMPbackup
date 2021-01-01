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

// Differentiating between Windows and Linux might be useful later
#ifdef _WIN32
  constexpr auto dirSeparator = '\\';
  constexpr auto tmpDir = "C:\\Windows\\Temp";
#else
  constexpr auto dirSeparator = '/';
  constexpr auto tmpDir = "/tmp";
#endif


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

int main(int argc, char *argv[])
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
  fs::path htmlPath;
  fs::path sslPath;
  fs::path tempPath(tmpDir);
  bool debug(false);

  // Supported options
  po::options_description desc("Usage");
  desc.add_options()
    ("help,h", "Print usage")
    ("dbHost",      po::value<string>()->default_value("localhost"), "Hostname of MySQL/MariaDB server")
    ("dbName",      po::value<string>(), "Name of MySQL/MariaDB database")
    ("dbUser",      po::value<string>(), "Username for MySQL/MariaDB database")
    ("dbPass",      po::value<string>(), "Password for MySQL/MariaDB database")
    ("websiteName", po::value<string>(), "Name of website (alphanumeric only, used in backup name)")
    ("htmlPath,",    po::value<string>(), "Root directory for PHP/HTML files")
    ("sslPath",      po::value<string>(), "Root directory for SSL files")
    ("tempPath",     po::value<string>()->default_value(tempPath.string()), "Temporary directory")
    ("debug", "Print additional debug info and do not delete staging area");

  //------------------------------------------------------------
  // Parse the command line values into program options and validate
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
  }
  catch(const exception &e)
  {
    cerr << "ERROR: " << e.what() << endl;
    cout << desc << "\n";
    return 1;
  }
  po::notify(vm);
  if (vm.count("help") || argc < 2)
  {
    cout << desc << "\n";
    return 0;
  }

  // TODO: Verify temp directory is writable. Alternatively, use current working directory (verify it is writable as well)?

  dbHost = vm["dbHost"].as<string>();

  if (vm.count("debug"))
  {
    debug = true;
    cout << "Debug option is set" << endl;
  }

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

  if (vm.count("htmlPath"))
  {
    try
    {
      htmlPath = fs::canonical(vm["htmlPath"].as<string>());
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could find HTML directory " << vm["htmlPath"].as<string>()
           << " (" << e.what() << ")" << endl;
      return 1;
    }
  }
  else
  {
    cout << "htmlPath was not set. Exiting." << endl;
    return 1;
  }

  if (vm.count("sslPath"))
  {
    try
    {
      sslPath = fs::canonical(vm["sslPath"].as<string>());
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could find SSL directory " << vm["sslPath"].as<string>()
           << " (" << e.what() << ")" << endl;
      return 1;
    }
  }
  else
  {
    cout << "sslPath was not set. Exiting." << endl;
    return 1;
  }

    if (vm.count("tempPath"))
  {
    try
    {
      tempPath = fs::canonical(vm["tempPath"].as<string>());
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could find temporary directory " << vm["tempPath"].as<string>()
           << " (" << e.what() << ")" << endl;
      return 1;
    }
  }

  // Archive file name
  string archiveName(websiteName + "_" + getDateTime());

  //------------------------------------------------------------
  // Print the program configuration
  if (debug)
  {
    cout << "\n--------------------------------------------------------------------------------"
         << "\n                  CONFIGURATION"
         << "\nDatabase server hostname: " << dbHost
         << "\n           Database Name: " << dbName
         << "\n       Database Username: " << dbUser
         << "\n       Database Password: " << dbPass
         << "\n            Website name: " << websiteName
         << "\n     HTML root directory: " << htmlPath
         << "\n           SSL directory: " << sslPath
         << "\n            Archive name: " << archiveName
         << "\n     Temporary directory: " << tempPath.string()
         << "\n--------------------------------------------------------------------------------"
         << endl;
  }

  // TODO:
  //   * Attempt to stop httpd?

  // Save the current directory as originalDir
  fs::path originalDir = fs::current_path();
  if (debug)
  {
    cout << "Original directory: " << originalDir << endl;
  }

  // Create a staging directory structure at tempDir/archiveName
  if (debug)
  {
    cout << "Changing directory to " << tempPath.string() << endl;
  }
  fs::current_path(tempPath);
  if (debug)
  {
    cout << "Current directory: " << fs::current_path().string() << '\n'
         << "Creating staging directory at " << fs::current_path().string() << "/" << archiveName
         << endl;
  }
  // If existing staging directory exists, remove it (this is probably never needed)
  fs::remove(archiveName);
  fs::create_directory(archiveName);
  fs::path stagingDir = fs::canonical(archiveName);
  if (debug)
  {
    cout << "Changing directory to " << stagingDir.string() << endl;
  }
  fs::current_path(stagingDir);
  if (debug)
  {
    cout << "Current directory: " << fs::current_path().string() << '\n'
         << "Creating html and ssl subdirectories" << endl;
  }
  fs::create_directory("html");
  fs::create_directory("ssl");

  //   * Copy files from htmlPath to tempdir/archiveName/html
  //     (e.g. cp -fr htmlPath/* tempdir/archiveName/html/)
  fs::path toPath = fs::canonical(stagingDir.string() + "/html/");
  try
  {
    cout << "Copying files from " << htmlPath.string() << " to " << toPath.string() << endl;
    fs::copy(htmlPath, toPath, fs::copy_options::recursive);
  }
  catch (const exception &e)
  {
    cerr << "ERROR: Could not copy files from " << htmlPath.string() << " to " << toPath.string()
         << " (" << e.what() << ")" << endl;
    return 1;
  }

  // TODO:
  //   * Check for hidden files to copy (e.g. .htaccess)
  //   * Copy files from sslPath to tempdir/archiveName/ssl
  //     (e.g. cp -fr sslPath/* tempdir/archiveName/ssl/)
  //   * Create mysql dump file of database in tempdir/archiveName/
  //     (e.g. cd tempdir/archiveName && mysqldump --password=dbPass --user=dbUser --host=dbHost --add-drop-table dbName > dbName.sql)
  //   * Archive and compress working directory
  //     (e.g. cd tempDir && tar -czf archiveName.tar.gz archiveName/)
  //   * Copy archiveName.tar.gz to originalWorkingDir
  //   * (e.g. cd tempDir && rm -fr archiveName)
  //   * Consider option to specify output directory

  if (!debug)
  {
    cout << "Removing staging directory at " << stagingDir.string() << endl;
    try
    {
      fs::remove_all(stagingDir);
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could remove staging directory " << stagingDir.string()
           << " (" << e.what() << ")" << endl;
      return 1;
    }
  }

  // End of main() function
  return 0;
}