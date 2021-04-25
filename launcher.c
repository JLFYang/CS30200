/*
  Course: CS 30200
  Name: Jacob Yang
  Assignment: 2
*/

#include <windows.h>
#include <stdio.h>

// function prototype
void printError(char* functionName);


int main(void)
{
    int i;
    DWORD dwCode;
    DWORD dwCreate;
    char buffer [50];
    
    #define NUMBER_OF_PROCESSES 6
    LPTSTR lpCommandLine[NUMBER_OF_PROCESSES];
    PROCESS_INFORMATION processInfo[NUMBER_OF_PROCESSES];
    
    STARTUPINFO startInfo;
    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    
    /* set up the command lines */
    lpCommandLine[1] = "C:\\Windows\\notepad.exe";
    lpCommandLine[2] = "C:\\Windows\\system32\\cmd.exe";
    lpCommandLine[3] = "C:\\Windows\\System32\\nslookup.exe";
    lpCommandLine[4] = "C:\\Windows\\system32\\charmap.exe";
    lpCommandLine[5] = "C:\\Windows\\System32\\write.exe";
    
    do{
      printf("Please make a choice from the following list.\n");
      printf("0: Quit\n");
      printf("1: Run Notepad\n");
      printf("*2: Run CMD Shell\n");
      printf("#3: Run NS LookUp\n");
      printf("4: Run Character Map\n");
      printf("5: Run Wordpad\n");
      printf("Make your Choice Now: ");
      scanf("%d",&i);
      
      switch(i)
      {
         case 1:
            if((sprintf(buffer, "%s\\notepad.exe", getenv("SYSTEMROOT"))) < 0)
            {
               printf("file not found\n");
            }
            dwCreate = NORMAL_PRIORITY_CLASS;
            break;
         case 2:
            if((sprintf(buffer, "%s", getenv("COMSPEC"))) < 0)
            {
               printf("file not found\n");
            }
            dwCreate = CREATE_NEW_CONSOLE;
            startInfo.dwFlags = 0x00000014;
            startInfo.dwFillAttribute = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY|BACKGROUND_RED;
            startInfo.dwX = _putenv("PROMPT=Speak Up:>");
            break; 
         case 3:
            if((sprintf(buffer, "%s\\Windows\\System32\\nslookup.exe", getenv("HOMEDRIVE"))) < 0)
            {
               printf("file not found\n");
            }
            dwCreate = NORMAL_PRIORITY_CLASS;
            break;
         case 4:
            if((sprintf(buffer, "%s\\Windows\\System32\\charmap.exe", getenv("HOMEDRIVE"))) < 0)
            {
               printf("file not found\n");
            }
            dwCreate = NORMAL_PRIORITY_CLASS;
            break;
          case 5:
            if((sprintf(buffer, "%s\\write.exe", getenv("SYSTEMROOT"))) < 0)
            {
               printf("file not found\n");
            }
            dwCreate = NORMAL_PRIORITY_CLASS;
            break;
          default:
               break;
      }
      if(i != 0 && i > 0 && i < 6)
      {
         CreateProcess(NULL,
            buffer,
            NULL,
            NULL,
            FALSE,
            dwCreate,
            NULL,
            NULL,
            &startInfo,
            &processInfo[i]);
            printf("Started program %d with pid = %d\n\n", i, (int)processInfo[i].dwProcessId);
      }
      
      switch(i)
      {
         case 2:
            if(0xFFFFFFFF == WaitForSingleObject(processInfo[i].hProcess, INFINITE))
            {
               printError("WaitForSingleObject");
            }
            GetExitCodeProcess(processInfo[i].hProcess, &dwCode);
            printf("code = %i\n", dwCode);
            break;
         case 3:
            if(0xFFFFFFFF == WaitForSingleObject(processInfo[i].hProcess, INFINITE))
            {
               printError("WaitForSingleObject");
            }
            break;
          default:
            break;
      }
    }while(i != 0);

   for (i = 0; i < NUMBER_OF_PROCESSES; i++)
   {
      CloseHandle(processInfo[i].hThread);
      CloseHandle(processInfo[i].hProcess);
   }

   return 0;
}

/****************************************************************
   The following function can be used to print out "meaningful"
   error messages. If you call a Win32 function and it returns
   with an error condition, then call this function right away and
   pass it a string containing the name of the Win32 function that
   failed. This function will print out a reasonable text message
   explaining the error and then (if chosen) terminate the program.
*/
void printError(char* functionName)
{
   LPVOID lpMsgBuf;
   int error_no;
   error_no = GetLastError();
   FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         error_no,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         (LPTSTR) &lpMsgBuf,
         0,
         NULL
   );
   // Display the string.
   fprintf(stderr, "\n%s failed on error %d: ", functionName, error_no);
   fprintf(stderr, (char*)lpMsgBuf);
   // Free the buffer.
   LocalFree( lpMsgBuf );
   //ExitProcess(1);  // terminate the program
}//printError
