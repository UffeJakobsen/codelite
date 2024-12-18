//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// copyright            : (C) 2014 Eran Ifrah
// file name            : znSettingsDlg.h
//
// -------------------------------------------------------------------------
// A
//              _____           _      _     _ _
//             /  __ \         | |    | |   (_) |
//             | /  \/ ___   __| | ___| |    _| |_ ___
//             | |    / _ \ / _  |/ _ \ |   | | __/ _ )
//             | \__/\ (_) | (_| |  __/ |___| | ||  __/
//              \____/\___/ \__,_|\___\_____/_|\__\___|
//
//                                                  F i l e
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef ZNSETTINGSDLG_H
#define ZNSETTINGSDLG_H

#include "zoom_navigator.h"

wxDECLARE_EVENT(wxEVT_ZN_SETTINGS_UPDATED, wxCommandEvent);

class znSettingsDlg : public znSettingsDlgBase
{
public:
    explicit znSettingsDlg(wxWindow* parent);
    ~znSettingsDlg() override = default;

protected:
    void OnOK(wxCommandEvent& event) override;
};
#endif // ZNSETTINGSDLG_H
