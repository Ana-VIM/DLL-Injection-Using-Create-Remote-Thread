// DLL Injection 1 - Create Remote Thread.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    StatusCode status = STATUS_ERROR;
    if (MIN_NUM_OF_ARGUMENTS > argc)
    {
        printf("<Please Enter 2 arguments: DLL Path to inject and PID>");
        status = INVALID_NUMBER_OF_ARGUMENTS;
        goto cleanup;
    }
    /* Initialization */
    DWORD target_process_pid = INITIAL_PID;
    HANDLE target_process_handle = INVALID_PROCESS_HANDLE;
    LPVOID remote_process_memmory_space = INVALID_BASE_MEMMORY_ADDRESS;
    SIZE_T size_to_allocate = INVALID_MEMMORY_SIZE_TO_RESERVE;
    BOOL memmory_write_status = MEMMORY_WRITING_FAILED;
    SIZE_T bytes_written = 0;
    HMODULE kernel32dll_handle = INVALID_MODULE_HANDLE;
    FARPROC load_library_address = INVALID_FUNCTION_ADDRESS;
    HANDLE remote_thread_handle = INVALID_THREAD_HANDLE;

    /* Converting the PID to int */
    target_process_pid = atoi(argv[PID]);

    if (INITIAL_PID == target_process_pid)
    {
        printf("<INVALID PID>");
        status = INVALID_PID;
        goto cleanup;
    }

    /* Getting the Size of the DLL Path to Inject */
    size_to_allocate = strnlen_s(argv[DLL_TO_RUN], MAXIMUM_PATH_SIZE) + 1; // +1 for Null character
    if (MAXIMUM_PATH_SIZE == size_to_allocate || INVALID_MEMMORY_SIZE_TO_RESERVE == size_to_allocate)
    {
        printf("<INVALID DLL PATH SIZE>");
        status = INVALID_DLL_PATH_SIZE;
        goto cleanup;
    }

    /* Get Process Handle */
    target_process_handle = OpenProcess(
        PROCESS_ALL_ACCESS,
        FALSE,
        target_process_pid);

    if (INVALID_PROCESS_HANDLE == target_process_handle)
    {
        printf("<Could'nt open process handle>");
        status = FAILED_TO_GET_PROCESS_HANDLE;
        goto cleanup;
    }

    /* Aloocating Memmory in the Remote Process */
    remote_process_memmory_space = VirtualAllocEx(
        target_process_handle, 
        NULL,
        size_to_allocate,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);

    if (INVALID_BASE_MEMMORY_ADDRESS == remote_process_memmory_space)
    {
        printf("<ALLOCATION FAILED>");
        status = FAILED_TO_ALLOCATE_REMOTE_MEMMORY;
        goto cleanup;
    }

    /* Writing Memmory in the Remote Process */
    memmory_write_status = WriteProcessMemory(
        target_process_handle,
        remote_process_memmory_space,
        argv[DLL_TO_RUN],
        size_to_allocate,
        &bytes_written);
    if (MEMMORY_WRITING_FAILED == memmory_write_status || bytes_written != size_to_allocate)
    {
        printf("<MEMMORY WRITING FAILED>");
        status = FAILED_TO_WRITE_MEMMORY;
        goto cleanup;
    }

    /* Get the DLL Handle */
    kernel32dll_handle = GetModuleHandle(
        "kernel32.dll");

    if (INVALID_MODULE_HANDLE == kernel32dll_handle)
    {
        printf("<FAILED TO GET DLL HANDLE>");
        status = FAILED_TO_GET_MODULE_HANDLE;
        goto cleanup;
    }

    /* Get the Base Address of LoadLibraryA */
    load_library_address = GetProcAddress(
        kernel32dll_handle,
        "LoadLibraryA");
    if (INVALID_FUNCTION_ADDRESS == load_library_address)
    {
        printf("<FAILED TO GET FUNCTION ADDRESS>");
        status = FAILED_TO_GET_FUNCTION_ADDRESS;
        goto cleanup;
    }

    /* Creating a Remote Thread */
    remote_thread_handle = CreateRemoteThread(
        target_process_handle,
        NULL,
        0,
        load_library_address,
        (LPCSTR) remote_process_memmory_space,
        0,
        NULL);

    if (INVALID_THREAD_HANDLE == remote_thread_handle)
    {
        printf("<FAILED TO CREATE REMOTE THREAD>");
        status = FAILED_TO_CREATE_REMOTE_THREAD;
        goto cleanup;
    }
    status = STATUS_SUCCESS;

cleanup:

    return status;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file