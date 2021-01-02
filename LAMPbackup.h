#pragma once
#include <iostream>
#include <iomanip>
#include <memory>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <boost/program_options.hpp>

using namespace std;
using namespace std::chrono;
namespace po = boost::program_options;
namespace fs = std::filesystem;

class LAMPbackup
{
public:
  LAMPbackup();
  ~LAMPbackup();

  bool parseUserConfig(int argc, char *argv[]);
  bool prepStagingPath();
  bool copyHTMLfiles();
  bool copySSLfiles();
  bool copyDatabase();
  bool archiveStagingPath();
  bool removeStagingPath();

  bool debug();
  string stagingPath();

private:
  //------------------------------------------------------------
  // Define variables needed for operation

  bool m_configOK;
  string m_dbHost;
  string m_dbName;
  string m_dbUser;
  string m_dbPass;
  string m_websiteName;
  fs::path m_htmlPath;
  fs::path m_sslPath;
  fs::path m_stagingPath;
  fs::path m_tempPath;
  fs::path m_origPath;
  string m_archiveName;
  bool m_debug;
  shared_ptr<po::options_description> m_optionsDescription;

  bool configOK();
  fs::path tempPath();
  string archiveName();
  string getDateTime();
  bool copyFiles(fs::path fromPath, fs::path toPath);
};