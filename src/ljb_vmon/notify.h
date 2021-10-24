/*++
Copyright (c) 1990-2000    Microsoft Corporation All Rights Reserved

Module Name:

    notify.h

Abstract:


Author:

     Eliyas Yakub   Nov 23, 1999

Environment:


Revision History:


--*/

#ifndef __NOTIFY_H
#define __NOTIFY_H


typedef enum {

    PLUGIN = 1,
    UNPLUG,
    EJECT

} USER_ACTION_TYPE;

typedef struct _DIALOG_RESULT
{
    ULONG    SerialNum;
    PWCHAR  DeviceId;
} DIALOG_RESULT, *PDIALOG_RESULT;

#define ID_EDIT 1

#define  IDM_OPEN       100
#define  IDM_CLOSE      101
#define  IDM_EXIT       102
#define  IDM_HIDE       103
#define  IDM_PLUGIN     104
#define  IDM_UNPLUG     105
#define  IDM_EJECT      106
#define  IDM_ENABLE     107
#define  IDM_DISABLE    108
#define  IDM_CLEAR      109
#define  IDM_IOCTL      110
#define  IDM_VERBOSE    111

#define IDD_DIALOG                     115
#define IDD_DIALOG1                    116
#define IDD_DIALOG2                    117
#define ID_OK                          118
#define ID_CANCEL                      119

#define ID_FUNCTION_ADDFREQ             120
#define ID_FUNCTION_SUBFREQ             121

#define ID_FUNCTION_ADDSNDFREQ             122
#define ID_FUNCTION_SUBSNDFREQ             123

#define IDC_SERIALNO                   1000
#define IDC_DEVICEID                   1001
#define IDC_STATIC                      -1

#define IDI_CLASS_ICON                 200

LRESULT FAR PASCAL
WndProc (
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

LRESULT
HandleCommands(
    HWND     hWnd,
    UINT     uMsg,
    WPARAM   wParam,
    LPARAM   lParam
    );

BOOLEAN Cleanup(
    HWND hWnd
    );

INT_PTR CALLBACK
DlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

void
SendIoctlToFilterDevice();

// VMON functions
DWORD
LJB_VMON_Main(
    __in LPVOID     lpThreadParameter
    );

#endif


