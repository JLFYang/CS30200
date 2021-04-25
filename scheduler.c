/*


*/
#include <windows.h>
#include <stdio.h>
#include <math.h>

typedef struct processor_data {
   int affinityMask;                /* affinity mask of this processor (just one bit set) */
   PROCESS_INFORMATION processInfo; /* process currently running on this processor */
   int running;                     /* 1 when this processor is running a task, 0 otherwise */
} ProcessorData;

/* function prototypes */
void printError(char* functionName);

int main(int argc, char *argv[])
{
   int processorCount = 0;       /* the number of allocated processors */
   ProcessorData *processorPool; /* an array of ProcessorData structures */
   HANDLE *processHandles;       /* an array of handles to processes */
   int *handleIndexInPool;       /* where each handle comes from in the pool */
   int *times;                   /* array to hold job duration times */
   
   if (argc < 3)
   {
      fprintf(stderr, "usage, %s  SCHEDULE_TYPE  SECONDS...\n", argv[0]);
      fprintf(stderr, "Where: SCHEDULE_TYPE = 0 means \"first come first serve\"\n");
      fprintf(stderr, "       SCHEDULE_TYPE = 1 means \"shortest job first\"\n");
      fprintf(stderr, "       SCHEDULE_TYPE = 2 means \"longest job first\"\n");
      return 0;
   }

   /* Read the schedule type off the command-line. */
   int type = atoi(argv[1]);
   /* Read the job duration times off the command-line. */
   int arr[argc-2];
   times = arr;
   for(int i = 2; i < argc; i++)
   {
      times[i-2] = atoi(argv[i]);
   }
   
   switch(type)
   {
      case 0: 
         break;
      case 1: 
         for (int i = 0; i < (argc - 2); i++)
         {
            for (int j = 0; j < (argc - 2); j++)
            {
               if (times[j] > times[i])
               {  
                  int tmp = times[i];
                  times[i] = times[j];
                  times[j] = tmp;
               }  
            }
         }
         break;
      case 2: 
         for (int i = 0; i < (argc - 2); i++)
         {
            for (int j = 0; j < (argc - 2); j++)
            {
               if (times[j] < times[i])
               {  
                  int tmp = times[i];
                  times[i] = times[j];
                  times[j] = tmp;
               }  
            }
         }
         break;
      default:
         break;
   }
   for(int i = 0;i<(argc - 2);i++)
      printf("%d",times[i]);
   /* Get the processor affinity mask for this process. */
   DWORD_PTR processAffinityMask;
   DWORD_PTR systemAffinityMask;
   GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemAffinityMask);
   /* Count the number of processors set in the affinity mask. */
   int aMask[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int i = 0;
   
   while (processAffinityMask)
   {
      if (processAffinityMask & 1)
      {
         aMask[i] = 1 << i;
         processorCount++;
      }
      processAffinityMask >>= 1; 
      i++;
   }
   /* Create, and then initialize, the processor pool array of data structures. */
   processorPool = malloc(processorCount * sizeof(ProcessorData));
   int j = 0;
   for(int i = 0;i < 16;i++)
   {
      if(aMask[i] != 0)
      {
         processorPool[j].affinityMask = aMask[i];
         processorPool[j].running = 0;
         j++;
      }
   }
   
   /* Start the first group of processes. */
   STARTUPINFO startInfo;
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);
   
   char buffer [50];
   int pRan = 0;
   
   for(int i = 0;i<(argc - 2);i++)
   {
      sprintf(buffer, "computeProgram_64.exe %d", times[i]);
      LPSTR lpCommandLine = buffer;
      if(!CreateProcess(NULL,
                        lpCommandLine,
                        NULL,
                        NULL,
                        FALSE,
                        NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
                        NULL,
                        NULL,
                        &startInfo,
                        &processorPool[i].processInfo))
         {
            printError("CreateProcess");
         }
      else
         {
            SetProcessAffinityMask(processorPool[i].processInfo.hProcess, processorPool[i].affinityMask);
            ResumeThread(processorPool[i].processInfo.hThread);
            processorPool[i].running = 1;
            printf("Launched %d Job on Processor with Mask %#4.2x\n", times[i], processorPool[i].affinityMask);
            pRan++;
         }
      if(pRan == processorCount)
         break;
   }
   /* Repeatedly wait for a process to finish and then,
      if there are more jobs to run, run a new job on
      the processor that just became free. */
   while (1)
   {
      /* Get, from the processor pool, handles to the currently running processes. */
      /* Put those handles in an array. */
      /* Use a parallel array to keep track of where in the processor pool each handle came from. */
      /* Check that there are still processes running, if not, quit. */
      processHandles = malloc(processorCount * sizeof(HANDLE));
      handleIndexInPool = malloc(processorCount * sizeof(int));
      int handleCount = 0;
      
      int j = 0;
      for(int i = 0;i<processorCount;i++)
      {
         if(processorPool[i].running == 1)
         {
            processHandles[j] = processorPool[i].processInfo.hProcess;
            handleIndexInPool[j] = i;
            handleCount++;
            j++;
         }
      }
      
      if(handleCount == 0)
      { 
         return 0;
      }
      
      /* Wait for one of the running processes to end. */
      DWORD result;
      if(WAIT_FAILED == (result = WaitForMultipleObjects(handleCount, processHandles, FALSE, INFINITE)))
      {
         printError("WaitForMultipleObjects");
      }
      result = handleIndexInPool[result];
      CloseHandle(processorPool[result].processInfo.hProcess);
      CloseHandle(processorPool[result].processInfo.hThread);
      processorPool[result].running = 0;
      handleCount--;
      
      if(pRan < (argc - 2))
      {
         sprintf(buffer, "computeProgram_64.exe %d", times[pRan]);
         LPSTR lpCommandLine = buffer;
         if(!CreateProcess(NULL,
            lpCommandLine,
            NULL,
            NULL,
            FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
            NULL,
            NULL,
            &startInfo,
            &processorPool[result].processInfo))
         {
            printError("CreateProcess");
         }
         else
         {
            SetProcessAffinityMask(processorPool[result].processInfo.hProcess, processorPool[result].affinityMask);
            ResumeThread(processorPool[result].processInfo.hThread);
            processorPool[result].running = 1;
            printf("Launched %d Job on Processor with Mask %#4.2x\n",times[pRan],processorPool[result].affinityMask);
            pRan++;
         }
      }
      /* Translate result from an index in processHandles[] to an index in processorPool[]. */
      /* Close the handles of the finished process and update the processorPool array. */
      /* Check if there is another process to run on the processor that just became free. */
   }

   return 0;
}









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
