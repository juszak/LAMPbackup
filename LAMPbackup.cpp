#include "LAMPbackup.h"

#ifdef _WIN32
constexpr auto dirSeparator = '\\';
constexpr auto tmpDir = "C:\\Windows\\Temp";
#else
constexpr auto dirSeparator = '/';
constexpr auto tmpDir = "/tmp";
#endif

LAMPbackup::LAMPbackup()
{
  m_configOK = false;
  m_dbHost = "";
  m_dbName = "";
  m_dbUser = "";
  m_dbPass = "";
  m_websiteName = "";
  //fs::path m_htmlPath;
  //fs::path m_sslPath;
  m_tempPath = fs::path(tmpDir);
  m_debug = false;
  m_optionsDescription = make_shared<po::options_description>("Usage"); // not certain this is right

  // Supported options
  m_optionsDescription->add_options()
    ("help,h", "Print usage")
    ("dbHost", po::value<string>()->default_value("localhost"), "Hostname of MySQL/MariaDB server")
    ("dbName", po::value<string>(), "Name of MySQL/MariaDB database")
    ("dbUser", po::value<string>(), "Username for MySQL/MariaDB database")
    ("dbPass", po::value<string>(), "Password for MySQL/MariaDB database")
    ("websiteName", po::value<string>(), "Name of website (alphanumeric only, used in backup name)")
    ("htmlPath,", po::value<string>(), "Root directory for PHP/HTML files")
    ("sslPath", po::value<string>(), "Root directory for SSL files")
    ("tempPath", po::value<string>()->default_value(m_tempPath.string()), "Temporary directory")
    ("debug", "Print additional debug info and do not delete staging area");
}

bool LAMPbackup::parseUserConfig(int argc, char *argv[])
{
  //------------------------------------------------------------
  // Parse the command line values into program options and validate
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, *m_optionsDescription), vm); // not certain dereferencing is best decision here
  }
  catch (const exception &e)
  {
    cerr << "ERROR: " << e.what() << endl;
    cout << *m_optionsDescription << "\n";
    throw;
  }
  po::notify(vm);
  if (vm.count("help") || argc < 2)
  {
    cout << *m_optionsDescription << "\n";
    return false;
  }

  // Prepare config items

  m_dbHost = vm["dbHost"].as<string>();

  if (vm.count("debug"))
  {
    m_debug = true;
    cout << "Debug option is set" << endl;
  }

  if (vm.count("dbName"))
  {
    m_dbName = vm["dbName"].as<string>();
  }
  else
  {
    cout << "dbName was not set. Exiting." << endl;
    return false;
  }

  if (vm.count("dbUser"))
  {
    m_dbUser = vm["dbUser"].as<string>();
  }
  else
  {
    cout << "dbUser was not set. Exiting." << endl;
    return false;
  }

  if (vm.count("dbPass"))
  {
    m_dbPass = vm["dbPass"].as<string>();
  }
  else
  {
    cout << "dbPass was not set. Exiting." << endl;
    return false;
  }

  if (vm.count("websiteName"))
  {
    m_websiteName = vm["websiteName"].as<string>();
    // Removew characters that are not alphanumeric
    m_websiteName.erase(
        remove_if(
            m_websiteName.begin(),
            m_websiteName.end(),
            [](char c) { return (!isalpha(c) && !isdigit(c)); }),
        m_websiteName.end());
  }
  else
  {
    cout << "websiteName was not set. Exiting." << endl;
    return false;
  }

  if (vm.count("htmlPath"))
  {
    try
    {
      m_htmlPath = fs::canonical(vm["htmlPath"].as<string>());
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could find HTML directory " << vm["htmlPath"].as<string>()
           << " (" << e.what() << ")" << endl;
      return false;
    }
  }
  else
  {
    cout << "htmlPath was not set. Exiting." << endl;
    return false;
  }

  if (vm.count("sslPath"))
  {
    try
    {
      m_sslPath = fs::canonical(vm["sslPath"].as<string>());
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could find SSL directory " << vm["sslPath"].as<string>()
           << " (" << e.what() << ")" << endl;
      return false;
    }
  }
  else
  {
    cout << "sslPath was not set. Exiting." << endl;
    return false;
  }

  if (vm.count("tempPath"))
  {
    try
    {
      m_tempPath = fs::canonical(vm["tempPath"].as<string>());
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could find temporary directory " << vm["tempPath"].as<string>()
           << " (" << e.what() << ")" << endl;
      return false;
    }
  }

  // Archive file name
  m_archiveName = m_websiteName + "_" + getDateTime();

  //------------------------------------------------------------
  // Print the program configuration
  if (m_debug)
  {
    cout << "\n--------------------------------------------------------------------------------"
         << "\n                  CONFIGURATION"
         << "\nDatabase server hostname: " << m_dbHost
         << "\n           Database Name: " << m_dbName
         << "\n       Database Username: " << m_dbUser
         << "\n       Database Password: " << m_dbPass
         << "\n            Website name: " << m_websiteName
         << "\n     HTML root directory: " << m_htmlPath
         << "\n           SSL directory: " << m_sslPath
         << "\n            Archive name: " << m_archiveName
         << "\n     Temporary directory: " << m_tempPath.string()
         << "\n--------------------------------------------------------------------------------"
         << endl;
  }

  m_configOK = true;
  return true;
}

LAMPbackup::~LAMPbackup()
{
}

bool LAMPbackup::configOK()
{
  return m_configOK;
}

bool LAMPbackup::debug()
{
  if (!configOK())
  {
    throw logic_error("Attempt to use unconfigured LAMPbackup (LAMPbackup.debug())");
  }
  return m_debug;
}

fs::path LAMPbackup::tempPath()
{
  if (!configOK())
  {
    throw logic_error("Attempt to use unconfigured LAMPbackup (LAMPbackup.tempPath())");
  }
  return m_tempPath;
}

string LAMPbackup::archiveName()
{
  if (!configOK())
  {
    throw logic_error("Attempt to use unconfigured LAMPbackup (LAMPbackup.archiveName())");
  }
  return m_archiveName;
}

string LAMPbackup::stagingPath()
{
  if (!configOK())
  {
    throw logic_error("Attempt to use unconfigured LAMPbackup (LAMPbackup.stagingPath())");
  }
  return m_stagingPath.string();
}

string LAMPbackup::getDateTime()
{
  auto now = system_clock::now();
  auto nowC = system_clock::to_time_t(now);
  stringstream buffer;
  buffer << put_time(localtime(&nowC), "%Y-%m-%d-%H-%M-%S");
  return buffer.str();
}

bool LAMPbackup::prepStagingPath()
{
  if (!configOK())
  {
    throw logic_error("Attempt to use unconfigured LAMPbackup (LAMPbackup.prepStagingPath())");
  }
    // Save the current directory as originalDir
  fs::path originalDir = fs::current_path();
  if (debug())
  {
    cout << "Original directory: " << originalDir << endl;
  }

  // Create a staging directory structure at tempDir/archiveName
  if (debug())
  {
    cout << "Changing directory to " << tempPath().string() << endl;
  }
  fs::current_path(tempPath());
  if (debug())
  {
    cout << "Current directory: " << fs::current_path().string() << '\n'
         << "Creating staging directory at " << fs::current_path().string() << "/" << archiveName()
         << endl;
  }
  // If existing staging directory exists, remove it (this is probably never needed)
  fs::remove(archiveName());
  fs::create_directory(archiveName());
  m_stagingPath = fs::canonical(archiveName());
  if (debug())
  {
    cout << "Changing directory to " << m_stagingPath.string() << endl;
  }
  fs::current_path(m_stagingPath);
  if (debug())
  {
    cout << "Current directory: " << fs::current_path().string() << '\n'
         << "Creating html and ssl subdirectories" << endl;
  }
  fs::create_directory("html");
  fs::create_directory("ssl");
  return true;
}

bool LAMPbackup::copyHTMLfiles()
{
  return copyFiles(m_htmlPath, fs::canonical(m_stagingPath.string() + "/html/"));
}

bool LAMPbackup::copySSLfiles()
{
  return copyFiles(m_sslPath, fs::canonical(m_stagingPath.string() + "/ssl/"));
}

bool LAMPbackup::copyFiles(fs::path fromPath, fs::path toPath)
{
  try
  {
    if(debug())
    {
      cout << "Copying files from " << fromPath.string() << " to " << toPath.string() << endl;
    }
    fs::copy(fromPath, toPath, fs::copy_options::recursive);
  }
  catch (const exception &e)
  {
    cerr << "ERROR: Could not copy files from " << fromPath.string() << " to " << toPath.string()
         << " (" << e.what() << ")" << endl;
    return false;
  }
  return true;
}

bool LAMPbackup::copyDatabase()
{
  cerr << "LAMPbackup::copyDatabase() not implemented" << endl;
  return false;
}

bool LAMPbackup::archiveStagingPath()
{
  cerr << "LAMPbackup::archiveStagingPath() not implemented" << endl;
  return false;
}

bool LAMPbackup::removeStagingPath()
{
  if (!debug())
  {
    cout << "Removing staging directory at " << m_stagingPath.string() << endl;
    try
    {
      fs::remove_all(m_stagingPath);
    }
    catch (const exception &e)
    {
      cerr << "ERROR: Could remove staging directory " << m_stagingPath.string()
           << " (" << e.what() << ")" << endl;
      return false;
    }
  }
  else
  {
    cerr << "Skipping removal of staging path (in debug mode, staging path " 
         << m_stagingPath.string() << " preserved for inspection)" << endl;
    return false;
  }
  return true;
}

