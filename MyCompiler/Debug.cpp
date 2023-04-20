#include "Debug.h"

void PrintStackTrace()
{
    void* pStack[MAX_STACK_FRAMES];
    static char szStackInfo[STACK_INFO_LEN * MAX_STACK_FRAMES];
    static char szFrameInfo[STACK_INFO_LEN];

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    WORD frames = CaptureStackBackTrace(0, MAX_STACK_FRAMES, pStack, NULL);

    for (WORD i = 0; i < frames; ++i) {
        DWORD64 address = (DWORD64)(pStack[i]);

        DWORD64 displacementSym = 0;
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        DWORD displacementLine = 0;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        if (SymFromAddr(process, address, &displacementSym, pSymbol) &&
            SymGetLineFromAddr64(process, address, &displacementLine, &line))
        {
            _snprintf_s(szFrameInfo, sizeof(szFrameInfo), "\t%s() at %s:%d(0x%x)\n",
                pSymbol->Name, line.FileName, line.LineNumber, pSymbol->Address);
        }
        else
        {
            _snprintf_s(szFrameInfo, sizeof(szFrameInfo), "\terror: %d\n", GetLastError());
        }
        strcat_s(szStackInfo, szFrameInfo);
    }

    printf("%s", szStackInfo); // 输出到控制台，也可以打印到日志文件中
}
