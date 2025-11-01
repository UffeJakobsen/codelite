#include "Scripting/CodeLiteLUA.hpp"

#include "ai/LLMManager.hpp"
#include "codelite_events.h"
#include "fileextmanager.h"
#include "globals.h"

extern "C" {
#include "lualib.h"
}

#include "FileManager.hpp"
#include "event_notifier.h"
#include "file_logger.h"
#include "fileutils.h"

#include <wx/msgdlg.h>
#include <wx/string.h>
#include <wx/xrc/xmlres.h>

namespace
{
enum class MessageType {
    kInfo,
    kWarn,
    kError,
};
}

CodeLiteLUA& CodeLiteLUA::Get()
{
    static CodeLiteLUA codelite_lua;
    return codelite_lua;
}

void CodeLiteLUA::Reset() { m_menu_items.clear(); }

void CodeLiteLUA::Initialise()
{
    auto& self = Get();
    self.m_state = luaL_newstate();
    luaL_openlibs(self.m_state);

    try {
        clDEBUG() << "Registering codelite with LUA" << endl;
        luabridge::getGlobalNamespace(self.m_state)
            .beginNamespace("codelite")
            .addFunction("message_box", &CodeLiteLUA::message_box)
            .addFunction("add_menu_item", &CodeLiteLUA::add_menu_item)
            .addFunction("editor_selection", &CodeLiteLUA::editor_selection)
            .addFunction("chat", &CodeLiteLUA::chat)
            .addFunction("editor_language", &CodeLiteLUA::editor_language)
            .addFunction("editor_text", &CodeLiteLUA::editor_text)
            .addFunction("editor_filepath", &CodeLiteLUA::editor_filepath)
            .addFunction("log_warn", &CodeLiteLUA::log_warn)
            .addFunction("log_error", &CodeLiteLUA::log_error)
            .addFunction("log_system", &CodeLiteLUA::log_system)
            .addFunction("log_debug", &CodeLiteLUA::log_debug)
            .addFunction("log_trace", &CodeLiteLUA::log_trace)
            .addFunction("str_replace_all", &CodeLiteLUA::str_replace_all)
            .endNamespace();

    } catch (const std::exception& e) {
        self.m_state = nullptr; // it's ok to have the memory leak
        clERROR() << "Error occurred while initialising LUA." << e.what() << endl;
        return;
    }
    self.InitialiseInternal();
    EventNotifier::Get()->Bind(wxEVT_FILE_SAVED, &CodeLiteLUA::OnFileSaved, &self);
}

void CodeLiteLUA::InitialiseInternal()
{
    clDEBUG() << "InitialiseInternal is called" << endl;
    Reset();

    // Load and compile CodeLite's main lua script.
    auto options = WriteOptions{.force_global = true};

    wxString codelite_lua = FileManager::GetSettingFileFullPath("codelite.lua", options);
    if (!wxFileName::FileExists(codelite_lua)) {
        clDEBUG() << "File: '" << codelite_lua << "' does not exist" << endl;
        return;
    }

    clDEBUG() << "Running file:" << codelite_lua << endl;
    auto status = LoadScriptFile(codelite_lua);
    if (!status.ok()) {
        wxString errmsg;
        errmsg << _("Failed to run LUA file: ") << codelite_lua << "\n" << status.message();
        ::wxMessageBox(errmsg, "CodeLite", wxICON_ERROR | wxOK | wxCENTER);
        return;
    }
    clDEBUG() << "Successfully executed script file:" << codelite_lua << endl;
}

CodeLiteLUA::CodeLiteLUA() {}

CodeLiteLUA::~CodeLiteLUA()
{
    // free the state
    m_menu_items.clear();
    if (m_state) {
        lua_close(m_state);
        m_state = nullptr;
    }
}

void CodeLiteLUA::message_box(const std::string& message, int type)
{
    switch (static_cast<MessageType>(type)) {
    case MessageType::kWarn:
        ::wxMessageBox(wxString::FromUTF8(message), "CodeLite Lua", wxOK | wxCENTER | wxICON_WARNING);
        break;
    case MessageType::kError:
        ::wxMessageBox(wxString::FromUTF8(message), "CodeLite Lua", wxOK | wxCENTER | wxICON_ERROR);
        break;
    default:
    case MessageType::kInfo:
        ::wxMessageBox(wxString::FromUTF8(message), "CodeLite Lua", wxOK | wxCENTER | wxICON_INFORMATION);
        break;
    }
}

void CodeLiteLUA::add_menu_item(const std::string& menu_name, const std::string& label, luabridge::LuaRef action)
{
    auto& self = Get();

    clDEBUG() << "Adding item:" << label << "for menu:" << menu_name << endl;
    LuaMenuItem menu_item{.label = label, .action = std::move(action)};
    if (!menu_item.IsOk()) {
        clWARNING() << "Failed to add menu item:" << label << "to menu:" << menu_name << ". Action is not a function"
                    << endl;
        return;
    }

    // If "menu_name" is missing a default entry is added.
    self.m_menu_items[menu_name].push_back(std::move(menu_item));
}

std::string CodeLiteLUA::editor_selection()
{
    auto editor = clGetManager()->GetActiveEditor();
    if (editor == nullptr) {
        return {};
    }

    return editor->GetSelection().ToStdString(wxConvUTF8);
}

std::string CodeLiteLUA::editor_language()
{
    auto editor = clGetManager()->GetActiveEditor();
    if (editor == nullptr) {
        return {};
    }
    auto lang = FileExtManager::GetLanguageFromType(FileExtManager::GetType(editor->GetRemotePathOrLocal()));
    return lang.ToStdString(wxConvUTF8);
}

std::string CodeLiteLUA::editor_text()
{
    auto editor = clGetManager()->GetActiveEditor();
    if (editor == nullptr) {
        return {};
    }
    return editor->GetEditorText().ToStdString(wxConvUTF8);
}

std::string CodeLiteLUA::editor_filepath()
{
    auto editor = clGetManager()->GetActiveEditor();
    if (editor == nullptr) {
        return {};
    }
    return editor->GetRemotePathOrLocal().ToStdString(wxConvUTF8);
}

void CodeLiteLUA::log_message(const std::string& msg, FileLogger::LogLevel level)
{
    switch (level) {
    case FileLogger::LogLevel::Developer:
        clDEBUG1() << wxString::FromUTF8(msg) << endl;
        break;
    case FileLogger::LogLevel::Dbg:
        clDEBUG() << wxString::FromUTF8(msg) << endl;
        break;
    case FileLogger::LogLevel::Warning:
        clWARNING() << wxString::FromUTF8(msg) << endl;
        break;
    case FileLogger::LogLevel::System:
        clSYSTEM() << wxString::FromUTF8(msg) << endl;
        break;
    case FileLogger::LogLevel::Error:
        clERROR() << wxString::FromUTF8(msg) << endl;
        break;
    }
}

std::string
CodeLiteLUA::str_replace_all(const std::string& str, const std::string& find_what, const std::string& replace_with)
{
    if (find_what.empty()) {
        return str;
    }

    std::string result;
    result.reserve(str.size()); // Pre-allocate to reduce reallocations

    size_t pos = 0;
    size_t last_pos = 0;

    while ((pos = str.find(find_what, last_pos)) != std::string::npos) {
        result.append(str, last_pos, pos - last_pos);
        result.append(replace_with);
        last_pos = pos + find_what.length();
    }

    result.append(str, last_pos, std::string::npos);
    return result;
}

void CodeLiteLUA::chat(const std::string& prompt)
{
    // send an event requesting to initiate a chat
    wxCommandEvent event_chat{wxEVT_MENU, XRCID("ai_show_chat_window")};
    event_chat.SetString(wxString::FromUTF8(prompt));
    EventNotifier::Get()->TopFrame()->GetEventHandler()->AddPendingEvent(event_chat);
}

clStatus CodeLiteLUA::LoadScriptString(const wxString& script)
{
    if (m_state == nullptr) {
        return StatusInavalidArgument("Could not run LUA script. Engine is not initialised.");
    }

    std::string script_content = script.ToStdString(wxConvUTF8);
    clDEBUG() << "Loading LUA script:\n" << script << endl;
    if (luaL_loadstring(m_state, script.c_str()) != LUA_OK) {
        return StatusOther(wxString::FromUTF8(lua_tostring(m_state, -1)));
    }

    if (lua_pcall(m_state, 0, 0, -2) != LUA_OK) {
        return StatusOther(wxString::FromUTF8(lua_tostring(m_state, -1)));
    }

    return StatusOk();
}

clStatus CodeLiteLUA::LoadScriptFile(const wxString& path)
{
    wxString content;
    if (!FileUtils::ReadFileContent(path, content)) {
        return StatusNotFound(path);
    }
    return LoadScriptString(content);
}

void CodeLiteLUA::UpdateMenu(const wxString& menu_name, wxMenu* menu)
{
    std::string name = menu_name.ToStdString(wxConvUTF8);
    if (!m_menu_items.contains(name)) {
        return;
    }

    auto& menu_items = m_menu_items[name];
    for (auto& item : menu_items) {
        wxString label = wxString::FromUTF8(item.label);
        auto menu_id = wxXmlResource::GetXRCID(label);
        menu->Append(menu_id, label);
        menu->Bind(wxEVT_MENU, [&item](wxCommandEvent&) { item.RunAction(); }, menu_id);
    }
}

void CodeLiteLUA::OnFileSaved(clCommandEvent& event)
{
    event.Skip(); // Always call this.

    CHECK_PTR_RET(clGetManager()->GetActiveEditor());

    const WriteOptions opts{.converter = nullptr, .force_global = true};
    wxString codelite_lua = FileManager::GetSettingFileFullPath("codelite.lua", opts);

    wxString filepath = clGetManager()->GetActiveEditor()->GetRemotePathOrLocal();
    if (filepath != codelite_lua) {
        return;
    }

    InitialiseInternal();
}
