// Microsoft Visual C++ generated resource script.
//
#include "resource.h"

#define APSTUDIO_READONLY_SYMBOLS
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 2 resource.
//
#include "afxres.h"

/////////////////////////////////////////////////////////////////////////////
#undef APSTUDIO_READONLY_SYMBOLS

/////////////////////////////////////////////////////////////////////////////
// English (United States) resources

#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENU)
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
#pragma code_page(1252)

#ifdef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// TEXTINCLUDE
//

1 TEXTINCLUDE 
BEGIN
    "resource.h\0"
END

2 TEXTINCLUDE 
BEGIN
    "#include ""afxres.h""\r\n"
    "\0"
END

3 TEXTINCLUDE 
BEGIN
    "\r\n"
    "\0"
END

#endif    // APSTUDIO_INVOKED


/////////////////////////////////////////////////////////////////////////////
//
// Menu
//

ID_MAIN_MENU MENU
BEGIN
    POPUP "File"
    BEGIN
        MENUITEM "&Load Image",                 ID_FILE_LOADI
        MENUITEM SEPARATOR
        MENUITEM "&Quit",                       ID_FILE_QUIT
    END
    POPUP "Display"
    BEGIN
        MENUITEM "Show pixel coordinates",      ID_DISPLAY_SHOWPIXELCOORDINATES
    END
    POPUP "Region Growing"
    BEGIN
        MENUITEM "Activate Region Growing",     ID_DISPLAY_REGIONGROWING
        MENUITEM "Predicates Values",           ID_PREDICATES
        POPUP "Mode"
        BEGIN
            MENUITEM "Play",                        ID_MODE_PLAY
            MENUITEM "Step",                        ID_MODE_STEP
            MENUITEM "Restore",                     ID_MODE_RESTORE
        END
        POPUP "Color "
        BEGIN
            MENUITEM "Red",                         ID_COLOR_RED
            MENUITEM "Green",                       ID_COLOR_GREEN
            MENUITEM "Blue",                        ID_COLOR_BLUE
        END
    END
END



ID_PLUS_ICON            ICON                    "icon1.ico"



IDD_DIALOG1 DIALOGEX 0, 0, 310, 177
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Predicates"
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    LTEXT           "Predicate 1 (Absolute difference between average intensity and pixel intensity)",IDC_PREDICATE1,26,48,164,19
    EDITTEXT        IDC_EDIT1,211,48,57,17,ES_AUTOHSCROLL
    LTEXT           "Predicate 2 (Distance between pixel and centroid of region)",IDC_PREDICATE2,25,78,165,22
    EDITTEXT        IDC_EDIT2,211,79,57,17,ES_AUTOHSCROLL
    DEFPUSHBUTTON   "OK",IDOK,199,156,50,14
    PUSHBUTTON      "Cancel",IDCANCEL,253,156,50,14
END


/////////////////////////////////////////////////////////////////////////////
//
// DESIGNINFO
//

#ifdef APSTUDIO_INVOKED
GUIDELINES DESIGNINFO
BEGIN
    IDD_DIALOG1, DIALOG
    BEGIN
        LEFTMARGIN, 7
        RIGHTMARGIN, 303
        TOPMARGIN, 7
        BOTTOMMARGIN, 170
    END
END
#endif    // APSTUDIO_INVOKED


/////////////////////////////////////////////////////////////////////////////
//
// AFX_DIALOG_LAYOUT
//

IDD_DIALOG1 AFX_DIALOG_LAYOUT
BEGIN
    0
END

#endif    // English (United States) resources
/////////////////////////////////////////////////////////////////////////////



#ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//


/////////////////////////////////////////////////////////////////////////////
#endif    // not APSTUDIO_INVOKED

