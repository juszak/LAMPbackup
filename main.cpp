////////////////////////////////////////////////////////////////////////////////
// lampbackup - backup utility for sites using LAMP stack (Linux, Apache,
//              MySQL/MariaDB, & PHP)

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
#include "config.h"
#include "LAMPbackup.h"

using namespace std;

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

  LAMPbackup lampBackup;

  if(!lampBackup.parseUserConfig(argc, argv))
  {
    cerr << "Exiting (could not parse user config)" << endl;
    return 1;
  }

  cout << "Executing step 1: Preparing staging path" << endl;
  if(!lampBackup.prepStagingPath())
  {
    cerr << "Exiting (could not prepare staging path)" << endl;
    return 1;
  }
  cout << "Executing step 2: Copying HTML files" << endl;
  if(!lampBackup.copyHTMLfiles())
  {
    cerr << "Exiting (could not copy HTML files)" << endl;
    return 1;
  }
  cout << "Executing step 3: Copying SSL files" << endl;
  if(!lampBackup.copySSLfiles())
  {
    cerr << "Exiting (could not copy SSL files)" << endl;
    return 1;
  }
  cout << "Executing step 4: Copying database to dump file" << endl;
  if(!lampBackup.copyDatabase())
  {
    cerr << "Exiting (could not dump SQL)" << endl;
    return 1;
  }
  cout << "Executing step 5: Archiving staging path" << endl;
  if(!lampBackup.archiveStagingPath())
  {
    cerr << "Exiting (could not archive staging path)" << endl;
    return 1;
  }

  if(!lampBackup.debug())
  {
    cout << "Executing step 6: Removing staging path" << endl;
    if(!lampBackup.removeStagingPath())
    {
      cerr << "Exiting (could not remove staging path)" << endl;
      return 1;
    }
  }
  else
  {
     cout << "Skipping step 6: (staging path will not be removed)\n"
          << "Staging path: " << lampBackup.stagingPath() << endl;
  }
   
  // TODO: 
  //   * Attempt to stop/restart httpd?
  //   * Consider option to specify output directory

  // End of main() function
  return 0;
}
