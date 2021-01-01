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

// TODO: Verify temp directory is writable. Alternatively, use current working directory (verify it is writable as well)?

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include "config.h"
#include "LAMPprep.h"

using namespace std;
namespace fs = std::filesystem;

void printVersion()
{
  cout << "lampbackup Version "
       << lampbackup_VERSION_MAJOR << "."
       << lampbackup_VERSION_MINOR << "."
       << lampbackup_VERSION_PATCH << endl;
}

int main(int argc, char *argv[])
{
  printVersion();

  LAMPprep lampPrep;

  if(!lampPrep.parseUserConfig(argc, argv))
  {
    cerr << "Exiting (could not parse user config)" << endl;
    return 1;
  }

  cout << "Executing step 1: Preparing staging path" << endl;
  if(!lampPrep.prepStagingPath())
  {
    cerr << "Exiting (could not prepare staging path)" << endl;
    return 1;
  }
  cout << "Executing step 2: Copying HTML files" << endl;
  if(!lampPrep.copyHTMLfiles())
  {
    cerr << "Exiting (could not copy HTML files)" << endl;
    return 1;
  }
  cout << "Executing step 3: Copying SSL files" << endl;
  if(!lampPrep.copySSLfiles())
  {
    cerr << "Exiting (could not copy SSL files)" << endl;
    return 1;
  }
  cout << "Executing step 4: Copying database to dump file" << endl;
  if(!lampPrep.copyDatabase())
  {
    cerr << "Exiting (could not dump SQL)" << endl;
    return 1;
  }
  cout << "Executing step 5: Archiving staging path" << endl;
  if(!lampPrep.archiveStagingPath())
  {
    cerr << "Exiting (could not archive staging path)" << endl;
    return 1;
  }

  if(!lampPrep.debug())
  {
    cout << "Executing step 6: Removing staging path" << endl;
    if(!lampPrep.removeStagingPath())
    {
      cerr << "Exiting (could not remove staging path)" << endl;
      return 1;
    }
  }
  else
  {
     cout << "Skipping step 6: (staging path will not be removed)\n"
          << "Staging path: " << lampPrep.stagingPath() << endl;
  }
   
  // TODO: 
  //   * Verify presence of prerequisite tools
  //     - tar (proabably assumed on Linux systems)
  //     - gzip (might just skip compression)
  //     - mysqldump
  //     Can probably meet this requirement via RPM packaging with dependencies
  //   * Attempt to stop httpd?
  //   * Copy files from sslPath to tempdir/archiveName/ssl
  //     (e.g. cp -fr sslPath/* tempdir/archiveName/ssl/)
  //   * Create mysql dump file of database in tempdir/archiveName/
  //     (e.g. cd tempdir/archiveName && mysqldump --password=dbPass --user=dbUser --host=dbHost --add-drop-table dbName > dbName.sql)
  //   * Archive and compress working directory
  //     (e.g. cd tempDir && tar -czf archiveName.tar.gz archiveName/)
  //   * Copy archiveName.tar.gz to originalWorkingDir
  //   * (e.g. cd tempDir && rm -fr archiveName)
  //   * Consider option to specify output directory

  // End of main() function
  return 0;
}