/*


*/
/*
     This program accepts commands that cause it to perform virtual memory
     operations. The commands are read from standard input, but it is better
     to put the commands in a "script file" and use the operating system's
     command line to redirect the script file to this program's standard input
     (as in "C:\VMdriver < VMcmds.txt").

     The commands that this program accepts are of the form

        time vmOp vmAddress units access

     The five parameters have the following meanings:

     time - Seconds to wait after reading the command before performing the VM operation.
     vmOp - Code that represents the VM operation to perform (see list below).
     vmAddress - Virtual memory address (in hex) where the VM operation is to be performed.
                 For commits, address should be a multiple of 0x00001000 (on a page boundary).
                 For reserve, address should be a multiple of 0x00010000 (on a allocation boundary).
     units - The number of units to use in the VM operation.
             For reserving memory, each unit represents 65,536 bytes of memory.
             For committing memory, each unit represents 4,096 bytes of memory.
     access - Code that represents the access protection (see list below).

     The vmOp codes and their meanings are:
     1 - Reserve a region of virtual memory.
     2 - Commit a block of pages.
     3 - Touch pages in a block.
     4 - Lock a block of pages.
     5 - Unlock a block of pages.
     6 - Commit one guard page.
     7 - Decommit a block of pages.
     8 - Release a region.

     The access codes and their meanings are:
     1 - PAGE_READONLY
     2 - PAGE_READWRITE
     3 - PAGE_EXECUTE
     4 - PAGE_EXECUTE_READ
     5 - PAGE_EXECUTE_READWRITE
     6 - PAGE_NOACCESS

     Most of the commands are described in the file
        "Virtual Memory from 'Beginning Windows NT Programming' by Julian Templeman.pdf".
     The only command not mentioned there is the "Touch pages in a block" command. This means
     that you should access (read) a memory location from each page in a specified block.

     Be absolutely sure that you check for any errors caused by the Windows functions that
     implement the VM operations since you will be trying to cause errors.

     This program should create a process that runs the program VMmapper.exe so that
     you can observe the memory operations as they happen. The program VMmapper takes
     a PID on its command line and then it repeatedly maps and displays (once a second)
     the virtual memory space of the process with that PID. This program should pass
     its own PID on the command line to the VMmapper.exe program.

     When this program has completed all of its operations, it goes into an infinite
     loop so that you can continue to examine its virtual memory map.
*/
#include <windows.h>
#include <stdio.h>
#include <string.h>

// prototype for the function, defined below, that prints err messages
void printError(char* functionName);

int main( )
{
   int time, vmOp, units, access_code;
   LPVOID vmAddress;
   int access_bit;
   
   // You need to provide the code that starts up the
   // VMmapper.exe program with the PID of this program
   // on VMmapper's command line. Use the Windows function
   // GetCurrentProcessId() to get this program's PID at
   // runtime. Use CreateProcess() to start VMmapper.exe.
   
   char buffer [100];
   sprintf(buffer,"%s %d","VMmapper.exe",GetCurrentProcessId());
   
   PROCESS_INFORMATION processInfo;
   
   STARTUPINFO startInfo;
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);
   
   if(!CreateProcess(NULL,
                     buffer,
                     NULL,
                     NULL,
                     TRUE,
                     HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
                     NULL,
                     NULL,
                     &startInfo,
                     &processInfo))
         {
            printError("CreateProcess");
         }
   
   Sleep(5000);  // give VMmapper.exe time to start

   // Command loop.
   printf("next VM command: ");
   while(scanf("%d%d%p%d%d", &time, &vmOp, &vmAddress, &units, &access_code) == 5)
   {
      // Wait until it is time to execute the command.
      Sleep(time*1000);
      
      switch(access_code)
      {
         case 1:
            access_bit = PAGE_READONLY;
            break;
         case 2:
            access_bit = PAGE_READWRITE;
            break;
         case 3:
            access_bit = PAGE_EXECUTE;
            break;
         case 4:
            access_bit = PAGE_EXECUTE_READ;
            break;
         case 5:
            access_bit = PAGE_EXECUTE_READWRITE;
            break;
         case 6:
            access_bit = PAGE_NOACCESS;
            break;
      }
      
      // Parse the command and execute it.
      switch (vmOp)
      {
         case 1:  // Reserve a region.
            if(!VirtualAlloc( vmAddress,
                          units * 65536,
                          MEM_RESERVE,
                          access_bit))
               printError("VirtualAlloc");
            break;
         case 2:  // Commit a block of pages.
            if(!VirtualAlloc( vmAddress,
                          units * 4096,
                          MEM_COMMIT,
                          access_bit))
               printError("VirtualAlloc");
            break;
         case 3:  // Touch pages in a block.
            for(int i = 0;i < units;i++)
            {
               printf("Touching Page %d\n",(int *)vmAddress + i);
            }
            break;
         case 4:  // Lock a block of pages.
            VirtualLock( vmAddress,
                         units);
            break;
         case 5:  // Unlock a block of pages.
            VirtualUnlock( vmAddress,
                         units);
            break;
         case 6:  // Create a guard page.
            if(!VirtualAlloc( vmAddress,
                          units * 4096,
                          MEM_COMMIT,
                          access_bit | PAGE_GUARD))
               printError("VirtualAlloc");
            break;
         case 7:  // Decommit a block of pages.
            if(!VirtualFree(vmAddress,
                       0,
                       MEM_DECOMMIT))
               printError("VirtualFree");
            break;
         case 8:  // Release a region.
            if(!VirtualFree(vmAddress,
                       0,
                       MEM_RELEASE))
               printError("VirtualFree");
            break;
      }//switch
      printf("Processed %d %d 0x%p %d %d\n", time, vmOp, vmAddress, units, access_code);
      printf("next VM command: ");
   }//while

   while (1)
   {
      Sleep(1000); // sleep for one second
   }

   return 0;
}//main()



/*******************************************************************
   This function prints out "meaningful" error messages. If you call
   a Windows function and it returns with an error condition, then
   call this function right away and pass it a string containing the
   name of the Windows function that failed. This function will print
   out a reasonable text message explaining the error.
*/
void printError(char* functionName)
{
   LPSTR lpMsgBuf = NULL;
   int error_no;
   error_no = GetLastError();
   FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      error_no,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
      (LPTSTR)&lpMsgBuf,
      0,
      NULL
   );
   /* Display the string. */
   fprintf(stderr, "\n%s failed on error %d: %s", functionName, error_no, lpMsgBuf);
   //MessageBox(NULL, lpMsgBuf, functionName, MB_OK);
   /* Free the buffer. */
   LocalFree( lpMsgBuf );
}
