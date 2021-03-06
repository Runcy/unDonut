/**
 *	@file	DonutOptions.h
 *	@brief	オプション.
 */
#pragma once

#include "option/MainOption.h"
#include "option/SupressPopupOption.h"
#include "option/DLControlOption.h"
#include "option/ExplorerBarDialog.h"
#include "option/UrlSecurityOption.h"
#include "option/KeyBoardDialog.h"
#include "option/MenuDialog.h"
#include "option/RightClickMenuDialog.h"
#include "option/MouseDialog.h"
#include "option/MouseGestureDialog.h"
#include "option/PluginDialog.h"
#include "option/ProxyDialog.h"
#include "option/SkinOption.h"
#include "option/StartUpFinishOption.h"
#include "option/ToolBarDialog.h"
#include "option/AddressBarPropertyPage.h"
#include "option/SearchPropertyPage.h"
#include "option/ExecutableDialog.h"
#include "option/MDITabDialog.h"
#include "option/LinkBarPropertyPage.h"
#include "option/UserDefinedCSSOption.h"
#include "option/UserDefinedJavascriptOption.h"
#include "StyleSheetOption.h"
#include "option/BrowserEmulationOption.h"

//ここだとリンクの依存関係が面倒なんでやっぱりmainfrm.h側でinclude
//#include "option/AddressBarPropertyPage.h"	//+++ AddressBar.hより分離
//#include "option/SearchPropertyPage.h"		//+++ SearchBar.hより分離
//#include "option/LinkBarPropertyPage.h"		//+++
