#include "PCH.h"
#include "FolderChangeWatcher.h"
#include <Foundation/IO/OSFile.h>

FolderChangeWatcher::FolderChangeWatcher(const ezString& sDirectory) :
  m_pWatchThread(NULL)
{
  #if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  m_pWatchThread = EZ_DEFAULT_NEW(FolderWatchThread)(sDirectory, *this);
  m_pWatchThread->Start();
  #endif
}

FolderChangeWatcher::~FolderChangeWatcher(void)
{
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  if(m_pWatchThread)
  {
    m_pWatchThread->Stop();
    m_pWatchThread->Join();
  }
  EZ_DEFAULT_DELETE(m_pWatchThread);
#endif
}

ezString FolderChangeWatcher::PopChangedFile()
{
  ezString output = "";

  m_queueMutex.Acquire();
  if(!m_changedFileQueue.IsEmpty())
  {
    output = m_changedFileQueue.PeekFront();
    m_changedFileQueue.PopFront();
  }
  m_queueMutex.Release();

  return output;
}

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)

FolderChangeWatcher::FolderWatchThread::FolderWatchThread(const ezString& sWatchedDirectory, FolderChangeWatcher& parent) :
  ezThread(sWatchedDirectory.GetData()),
  m_parent(parent),
  m_sWatchedDirectory(sWatchedDirectory),
  m_bRun(true)
{
  m_changeHandle = FindFirstChangeNotificationW( 
    ezStringWChar(sWatchedDirectory.GetData()).GetData(),      // directory to watch 
    TRUE,                         // watch subtree 
    FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes 

  if (m_changeHandle == INVALID_HANDLE_VALUE) 
    ezLog::Error("FindFirstChangeNotification function failed.");

  // get current filetime
  SYSTEMTIME systemTime;
  GetSystemTime(&systemTime);
  SystemTimeToFileTime(&systemTime, reinterpret_cast<LPFILETIME>(&m_lastTrackedModification));
}

ezUInt32 FolderChangeWatcher::FolderWatchThread::Run()
{
  while(m_bRun)
  {
    DWORD waitStatus = WaitForMultipleObjects(1, &m_changeHandle, TRUE, 200);

    if(waitStatus == WAIT_OBJECT_0)
    {
      ezStringBuilder wildcardPath(m_sWatchedDirectory.GetData());
      wildcardPath.AppendPath("*");

      // iterate over each file
      WIN32_FIND_DATAW ffd;
      HANDLE hFind = FindFirstFileW(ezStringWChar(wildcardPath.GetData()).GetData(), &ffd);
      if (hFind == INVALID_HANDLE_VALUE)
      {
        ezLog::Error("Error iterating over directory \"%s\"", m_sWatchedDirectory);
        continue;
      }

      ezUInt64 lastTrackedModificationNew = m_lastTrackedModification;

      do {
        // ignore directories
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
          ezUInt64 fileTime = *reinterpret_cast<ezUInt64*>(&ffd.ftLastWriteTime);
          if(fileTime > m_lastTrackedModification)
          {
            lastTrackedModificationNew = std::max(lastTrackedModificationNew, fileTime);
            
            ezStringUtf8 changedFile(ffd.cFileName);
            m_parent.m_queueMutex.Acquire();
            m_parent.m_changedFileQueue.PushBack(changedFile.GetData());
            m_parent.m_queueMutex.Release();

            ezLog::Info("File was changed: \"%s\"", changedFile.GetData());
          }
        }
      } while (FindNextFile(hFind, &ffd) != 0);

      m_lastTrackedModification = lastTrackedModificationNew;
    }
  }

  return 0;
}

#endif