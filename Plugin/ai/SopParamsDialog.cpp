#include "ai/SopParamsDialog.hpp"

#include "globals.h"

#include <wx/sizer.h>
#include <wx/stattext.h>

SopParamsDialog::SopParamsDialog(wxWindow* parent, const SopInfo& sop)
    : SopParamsDialogBase(parent, wxID_ANY, wxString() << _("Enter SOP Parameters (") << sop.title << ")")
{
    for (const auto& param : sop.params) {
        auto label = new wxStaticText(m_mainPanel, wxID_ANY, param.name + ":");
        m_flexGridSizer->Add(label, wxSizerFlags().Border(wxALL, 5).Right().CentreVertical());

        auto value = new wxTextCtrl(m_mainPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize{200, -1});
        value->SetHint(param.desc);
        m_flexGridSizer->Add(value, wxSizerFlags().Expand().Border(wxALL, 5));
        m_textEntries.push_back(std::make_pair(param.name, value));
    }

    if (!m_textEntries.empty()) {
        m_textEntries[0].second->CallAfter(&wxWindow::SetFocus);
    }

    GetSizer()->Fit(this);

    // Set the window size
    wxWindow* parent_tlw = EventNotifier::Get()->TopFrame();

    if (!parent_tlw) {
        return;
    }

    wxRect frameSize = parent_tlw->GetSize();
    frameSize.Deflate(100);
    wxSize sz{frameSize.GetSize().x, wxNOT_FOUND};
    SetMinSize(sz);
    SetSize(sz);
    CentreOnParent();

#if defined(__WXMAC__) || defined(__WXMSW__)
    Move(wxNOT_FOUND, parent_tlw->GetPosition().y);
#endif
    PostSizeEvent();
}

std::vector<std::pair<wxString, wxString>> SopParamsDialog::GetParams() const
{
    std::vector<std::pair<wxString, wxString>> params;
    for (const auto& [name, text_entry] : m_textEntries) {
        if (text_entry->GetValue().IsEmpty())
            continue;
        params.push_back(std::make_pair(name, text_entry->GetValue()));
    }
    return params;
}
