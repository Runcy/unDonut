#pragma once

enum wm_t {
	wm_NULL 						= 0x0000,
	wm_CREATE						= 0x0001,
	wm_DESTROY						= 0x0002,
	wm_MOVE 						= 0x0003,
	wm_SIZE 						= 0x0005,
	wm_ACTIVATE 					= 0x0006,
	wm_SETFOCUS 					= 0x0007,
	wm_KILLFOCUS					= 0x0008,
	wm_ENABLE						= 0x000A,
	wm_SETREDRAW					= 0x000B,
	wm_SETTEXT						= 0x000C,
	wm_GETTEXT						= 0x000D,
	wm_GETTEXTLENGTH				= 0x000E,
	wm_PAINT						= 0x000F,
	wm_CLOSE						= 0x0010,
	wm_QUERYENDSESSION				= 0x0011,
	wm_QUERYOPEN					= 0x0013,
	wm_ENDSESSION					= 0x0016,
	wm_QUIT 						= 0x0012,
	wm_ERASEBKGND					= 0x0014,
	wm_SYSCOLORCHANGE				= 0x0015,
	wm_SHOWWINDOW					= 0x0018,
	wm_WININICHANGE 				= 0x001A,
	//wm_SETTINGCHANGE				= wm_WININICHANGE,
	wm_DEVMODECHANGE				= 0x001B,
	wm_ACTIVATEAPP					= 0x001C,
	wm_FONTCHANGE					= 0x001D,
	wm_TIMECHANGE					= 0x001E,
	wm_CANCELMODE					= 0x001F,
	wm_SETCURSOR					= 0x0020,
	wm_MOUSEACTIVATE				= 0x0021,
	wm_CHILDACTIVATE				= 0x0022,
	wm_QUEUESYNC					= 0x0023,
	wm_GETMINMAXINFO				= 0x0024,
	wm_PAINTICON					= 0x0026,
	wm_ICONERASEBKGND				= 0x0027,
	wm_NEXTDLGCTL					= 0x0028,
	wm_SPOOLERSTATUS				= 0x002A,
	wm_DRAWITEM 					= 0x002B,
	wm_MEASUREITEM					= 0x002C,
	wm_DELETEITEM					= 0x002D,
	wm_VKEYTOITEM					= 0x002E,
	wm_CHARTOITEM					= 0x002F,
	wm_SETFONT						= 0x0030,
	wm_GETFONT						= 0x0031,
	wm_SETHOTKEY					= 0x0032,
	wm_GETHOTKEY					= 0x0033,
	wm_QUERYDRAGICON				= 0x0037,
	wm_COMPAREITEM					= 0x0039,
	wm_GETOBJECT					= 0x003D,
	wm_COMPACTING					= 0x0041,
	wm_COMMNOTIFY					= 0x0044,
	wm_WINDOWPOSCHANGING			= 0x0046,
	wm_WINDOWPOSCHANGED 			= 0x0047,
	wm_POWER						= 0x0048,
	wm_COPYDATA 					= 0x004A,
	wm_CANCELJOURNAL				= 0x004B,
	wm_NOTIFY						= 0x004E,
	wm_INPUTLANGCHANGEREQUEST		= 0x0050,
	wm_INPUTLANGCHANGE				= 0x0051,
	wm_TCARD						= 0x0052,
	wm_HELP 						= 0x0053,
	wm_USERCHANGED					= 0x0054,
	wm_NOTIFYFORMAT 				= 0x0055,
	wm_CONTEXTMENU					= 0x007B,
	wm_STYLECHANGING				= 0x007C,
	wm_STYLECHANGED 				= 0x007D,
	wm_DISPLAYCHANGE				= 0x007E,
	wm_GETICON						= 0x007F,
	wm_SETICON						= 0x0080,
	wm_NCCREATE 					= 0x0081,
	wm_NCDESTROY					= 0x0082,
	wm_NCCALCSIZE					= 0x0083,
	wm_NCHITTEST					= 0x0084,
	wm_NCPAINT						= 0x0085,
	wm_NCACTIVATE					= 0x0086,
	wm_GETDLGCODE					= 0x0087,
	wm_SYNCPAINT					= 0x0088,
	wm_NCMOUSEMOVE					= 0x00A0,
	wm_NCLBUTTONDOWN				= 0x00A1,
	wm_NCLBUTTONUP					= 0x00A2,
	wm_NCLBUTTONDBLCLK				= 0x00A3,
	wm_NCRBUTTONDOWN				= 0x00A4,
	wm_NCRBUTTONUP					= 0x00A5,
	wm_NCRBUTTONDBLCLK				= 0x00A6,
	wm_NCMBUTTONDOWN				= 0x00A7,
	wm_NCMBUTTONUP					= 0x00A8,
	wm_NCMBUTTONDBLCLK				= 0x00A9,
	wm_NCXBUTTONDOWN				= 0x00AB,
	wm_NCXBUTTONUP					= 0x00AC,
	wm_NCXBUTTONDBLCLK				= 0x00AD,
	wm_INPUT						= 0x00FF,
	//wm_KEYFIRST					= 0x0100,
	wm_KEYDOWN						= 0x0100,
	wm_KEYUP						= 0x0101,
	wm_CHAR 						= 0x0102,
	wm_DEADCHAR 					= 0x0103,
	wm_SYSKEYDOWN					= 0x0104,
	wm_SYSKEYUP 					= 0x0105,
	wm_SYSCHAR						= 0x0106,
	wm_SYSDEADCHAR					= 0x0107,
	wm_UNICHAR						= 0x0109,
	//wm_KEYLAST					= 0x0109,
	//wm_KEYLAST					= 0x0108,
	wm_IME_STARTCOMPOSITION 		= 0x010D,
	wm_IME_ENDCOMPOSITION			= 0x010E,
	wm_IME_COMPOSITION				= 0x010F,
	wm_IME_KEYLAST					= 0x010F,
	wm_INITDIALOG					= 0x0110,
	wm_COMMAND						= 0x0111,
	wm_SYSCOMMAND					= 0x0112,
	wm_TIMER						= 0x0113,
	wm_HSCROLL						= 0x0114,
	wm_VSCROLL						= 0x0115,
	wm_INITMENU 					= 0x0116,
	wm_INITMENUPOPUP				= 0x0117,
	wm_MENUSELECT					= 0x011F,
	wm_MENUCHAR 					= 0x0120,
	wm_ENTERIDLE					= 0x0121,
	wm_MENURBUTTONUP				= 0x0122,
	wm_MENUDRAG 					= 0x0123,
	wm_MENUGETOBJECT				= 0x0124,
	wm_UNINITMENUPOPUP				= 0x0125,
	wm_MENUCOMMAND					= 0x0126,
	wm_CHANGEUISTATE				= 0x0127,
	wm_UPDATEUISTATE				= 0x0128,
	wm_QUERYUISTATE 				= 0x0129,
	wm_CTLCOLORMSGBOX				= 0x0132,
	wm_CTLCOLOREDIT 				= 0x0133,
	wm_CTLCOLORLISTBOX				= 0x0134,
	wm_CTLCOLORBTN					= 0x0135,
	wm_CTLCOLORDLG					= 0x0136,
	wm_CTLCOLORSCROLLBAR			= 0x0137,
	wm_CTLCOLORSTATIC				= 0x0138,
	wm_MOUSEMOVE					= 0x0200,
	wm_LBUTTONDOWN					= 0x0201,
	wm_LBUTTONUP					= 0x0202,
	wm_LBUTTONDBLCLK				= 0x0203,
	wm_RBUTTONDOWN					= 0x0204,
	wm_RBUTTONUP					= 0x0205,
	wm_RBUTTONDBLCLK				= 0x0206,
	wm_MBUTTONDOWN					= 0x0207,
	wm_MBUTTONUP					= 0x0208,
	wm_MBUTTONDBLCLK				= 0x0209,
	wm_MOUSEWHEEL					= 0x020A,
	//wm_MOUSEFIRST 				= 0x0200,
	//wm_MOUSELAST					= 0x0209,
	//wm_MOUSELAST					= 0x020A,
	//wm_MOUSELAST					= 0x020D,
	wm_XBUTTONDOWN					= 0x020B,
	wm_XBUTTONUP					= 0x020C,
	wm_XBUTTONDBLCLK				= 0x020D,
	wm_PARENTNOTIFY 				= 0x0210,
	wm_ENTERMENULOOP				= 0x0211,
	wm_EXITMENULOOP 				= 0x0212,
	wm_NEXTMENU 					= 0x0213,
	wm_SIZING						= 0x0214,
	wm_CAPTURECHANGED				= 0x0215,
	wm_MOVING						= 0x0216,
	wm_POWERBROADCAST				= 0x0218,
	wm_DEVICECHANGE 				= 0x0219,
	wm_MDICREATE					= 0x0220,
	wm_MDIDESTROY					= 0x0221,
	wm_MDIACTIVATE					= 0x0222,
	wm_MDIRESTORE					= 0x0223,
	wm_MDINEXT						= 0x0224,
	wm_MDIMAXIMIZE					= 0x0225,
	wm_MDITILE						= 0x0226,
	wm_MDICASCADE					= 0x0227,
	wm_MDIICONARRANGE				= 0x0228,
	wm_MDIGETACTIVE 				= 0x0229,
	wm_MDISETMENU					= 0x0230,
	wm_ENTERSIZEMOVE				= 0x0231,
	wm_EXITSIZEMOVE 				= 0x0232,
	wm_DROPFILES					= 0x0233,
	wm_MDIREFRESHMENU				= 0x0234,
	wm_IME_SETCONTEXT				= 0x0281,
	wm_IME_NOTIFY					= 0x0282,
	wm_IME_CONTROL					= 0x0283,
	wm_IME_COMPOSITIONFULL			= 0x0284,
	wm_IME_SELECT					= 0x0285,
	wm_IME_CHAR 					= 0x0286,
	wm_IME_REQUEST					= 0x0288,
	wm_IME_KEYDOWN					= 0x0290,
	wm_IME_KEYUP					= 0x0291,
	wm_MOUSEHOVER					= 0x02A1,
	wm_MOUSELEAVE					= 0x02A3,
	wm_NCMOUSEHOVER 				= 0x02A0,
	wm_NCMOUSELEAVE 				= 0x02A2,
	wm_WTSSESSION_CHANGE			= 0x02B1,
	wm_TABLET_FIRST 				= 0x02c0,
	wm_TABLET_LAST					= 0x02df,
	wm_CUT							= 0x0300,
	wm_COPY 						= 0x0301,
	wm_PASTE						= 0x0302,
	wm_CLEAR						= 0x0303,
	wm_UNDO 						= 0x0304,
	wm_RENDERFORMAT 				= 0x0305,
	wm_RENDERALLFORMATS 			= 0x0306,
	wm_DESTROYCLIPBOARD 			= 0x0307,
	wm_DRAWCLIPBOARD				= 0x0308,
	wm_PAINTCLIPBOARD				= 0x0309,
	wm_VSCROLLCLIPBOARD 			= 0x030A,
	wm_SIZECLIPBOARD				= 0x030B,
	wm_ASKCBFORMATNAME				= 0x030C,
	wm_CHANGECBCHAIN				= 0x030D,
	wm_HSCROLLCLIPBOARD 			= 0x030E,
	wm_QUERYNEWPALETTE				= 0x030F,
	wm_PALETTEISCHANGING			= 0x0310,
	wm_PALETTECHANGED				= 0x0311,
	wm_HOTKEY						= 0x0312,
	wm_PRINT						= 0x0317,
	wm_PRINTCLIENT					= 0x0318,
	wm_APPCOMMAND					= 0x0319,
	wm_THEMECHANGED 				= 0x031A,
	wm_HANDHELDFIRST				= 0x0358,
	wm_HANDHELDLAST 				= 0x035F,
	wm_AFXFIRST 					= 0x0360,
	wm_AFXLAST						= 0x037F,
	wm_PENWINFIRST					= 0x0380,
	wm_PENWINLAST					= 0x038F,
	wm_APP							= 0x8000,
	wm_USER 						= 0x0400,
	dm_GETDEFID 					= (WM_USER+0),
	dm_SETDEFID 					= (WM_USER+1),
	dm_REPOSITION					= (WM_USER+2),
};
