//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// copyright            : (C) 2008 by Eran Ifrah
// file name            : frame.h
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
#ifndef LITEEDITOR_FRAME_H
#define LITEEDITOR_FRAME_H

#include "AsyncProcess/ZombieReaperPOSIX.h"
#include "EnvironmentVariablesDlg.h"
#include "Notebook.h"
#include "SecondarySideBar.hpp"
#include "clCaptionBar.hpp"
#include "clDockingManager.h"
#include "clInfoBar.h"
#include "clStatusBar.h"
#include "clToolBar.h"
#include "cl_command_event.h"
#include "cl_editor.h"
#include "cl_process.h"
#include "debuggerpane.h"
#include "generalinfo.h"
#include "macros.h"
#include "mainbook.h"
#include "output_pane.h"
#include "tags_options_dlg.h"
#include "wxCustomControls.hpp"

#include <set>
#include <wx/aui/aui.h>
#include <wx/choice.h>
#include <wx/cmndata.h>
#include <wx/combobox.h>
#include <wx/dcbuffer.h>
#include <wx/frame.h>
#include <wx/html/htmlwin.h>
#include <wx/infobar.h>
#include <wx/minifram.h>
#include <wx/process.h>
#include <wx/splash.h>
#include <wx/timer.h>

// forward decls
class OnSysColoursChanged;
class DebuggerToolBar;
class WebUpdateJob;
class CodeLiteApp;
class clSingleInstanceThread;
class wxCustomStatusBar;
class TagEntry;
class SideBar;
class wxToolBar;
class OpenWindowsPanel;
class WorkspaceTab;
class FileExplorer;
class OutputTabWindow;
class DockablePaneMenuManager;
class MyMenuBar;

//--------------------------------
// Helper class
//--------------------------------
extern const wxEventType wxEVT_LOAD_PERSPECTIVE;
extern const wxEventType wxEVT_REFRESH_PERSPECTIVE_MENU;
extern const wxEventType wxEVT_ACTIVATE_EDITOR;

struct StartPageData {
    wxString name;
    wxString file_path;
    wxString action;
};

class clMainFrame : public wxFrame
{
    enum class ePostBuildEndAction { kNone, kRunProject, kRebuildProject };

    MainBook* m_mainBook;
    static clMainFrame* m_theFrame;
    clDockingManager m_mgr;
    OutputPane* m_outputPane;
    SideBar* m_sidebar = nullptr;
    SecondarySideBar* m_secondary_sidebar = nullptr;
    wxArrayString m_files;
    wxTimer* m_timer;
    std::map<int, wxString> m_viewAsMap;
    TagsOptionsData m_tagsOptionsData;
    DebuggerPane* m_debuggerPane;
    ePostBuildEndAction m_postBuildEndAction;
    GeneralInfo m_frameGeneralInfo;
    std::map<int, wxString> m_panes;
    wxMenu* m_cppMenu;
    bool m_highlightWord;
    DockablePaneMenuManager* m_DPmenuMgr;
    wxPanel* m_mainPanel;
    wxString m_codeliteDownloadPageURL;
    wxString m_defaultLayout;
    bool m_workspaceRetagIsRequired;
    bool m_loadLastSession;
    wxMenuBar* m_mainMenuBar;
    wxMenu* m_bookmarksDropDownMenu;
    bool m_noSavePerspectivePrompt;

#ifndef __WXMSW__
    ZombieReaperPOSIX m_zombieReaper;
#else
    HMENU hMenu = nullptr; // Menu bar
#endif

#ifdef __WXGTK__
    bool m_isWaylandSession = false;
#endif

    // Maintain a set of core toolbars (i.e. toolbars not owned by any plugin)
    wxStringSet_t m_coreToolbars;
    clStatusBar* m_statusBar;
    clSingleInstanceThread* m_singleInstanceThread;
    bool m_toggleToolBar;

    // Printing
    wxPrintDialogData m_printDlgData;
    size_t m_debuggerFeatures = clDebugEvent::kAllFeatures;
    WebUpdateJob* m_webUpdate;
    wxToolBar* m_mainToolbar;
    clToolBarGeneric* m_pluginsToolbar;
    DebuggerToolBar* m_debuggerToolbar = nullptr;
    clInfoBar* m_infoBar = nullptr;
#if !wxUSE_NATIVE_CAPTION
    clCaptionBar* m_captionBar = nullptr;
#endif
    // the main tool default style
    int m_mainToolbarStyle = wxTB_FLAT | wxTB_NODIVIDER /* toolbar is hidden by default */;
    wxString m_mainFrameTitleTemplate;

public:
    static bool m_initCompleted;

protected:
    bool IsEditorEvent(wxEvent& event);
    void DoSysColoursChanged();
    void DoCreateBuildDropDownMenu(wxMenu* menu);
    void DoShowToolbars(bool show, bool update = true);
    void InitializeLogo();
    void DoShowMenuBar(bool show);
    void OnSysColoursChanged(clCommandEvent& event);
    void RestoreFrameSizeAndPosition();
    void UpdateMainToolbarOrientation(int newOrientation);

public:
    void Raise() override;

    static clMainFrame* Get();
    static void Initialize(bool loadLastSession);

    wxPanel* GetMainPanel() const { return m_mainPanel; }

    clInfoBar* GetMessageBar() { return m_infoBar; }

    /**
     * @brief return the menu bar
     * @return
     */
    wxMenuBar* GetMainMenuBar() const { return m_mainMenuBar; }

    /**
     * @brief goto anything..
     */
    void OnGotoAnything(wxCommandEvent& e);

    /**
     * @brief main book page navigation
     */
    void OnMainBookNavigating(wxCommandEvent& e);

    /**
     * @brief move the active tab right or left
     */
    void OnMainBookMovePage(wxCommandEvent& e);

    /**
     * @brief Return CodeLite App object
     */
    CodeLiteApp* GetTheApp();

    /**
     * @brief return the status bar
     */
    clStatusBar* GetStatusBar() { return m_statusBar; }

    /**
     * @brief update the parser (code completion) search paths using the
     * default compiler as the input provider
     */
    void UpdateParserSearchPathsFromDefaultCompiler();

    DockablePaneMenuManager* GetDockablePaneMenuManager() { return m_DPmenuMgr; }
    //--------------------- debuger---------------------------------
    //---------------------------------------------------------------

    /**
     * \brief update various AUI parameters like Output Pane View caption visible and other
     */
    void UpdateAUI();

    /**
     * @brief set the AUI manager flags as dictated from the configuration
     */
    void SetAUIManagerFlags();

    /**
     * @brief update the environment status message:
     * which displays to the user the current environment set used + the active builder
     */
    void SelectBestEnvSet();

    virtual ~clMainFrame();
    /**
     * @brief set frame option flag
     * @param set
     * @param flag
     */
    void SetFrameFlag(bool set, int flag);

    /**
     * @brief return the current tags options data
     * @return
     */
    TagsOptionsData& GetTagsOptions() { return m_tagsOptionsData; }

    /**
     * @brief return true if the word under the caret should be highlighted
     * @return
     */
    bool GetHighlightWord() { return m_highlightWord; }

    /**
     * @brief
     * @param editor
     */
    void SetFrameTitle(clEditor* editor);

    MainBook* GetMainBook() const { return m_mainBook; }

    /**
     * @return the output pane (the bottom pane)
     */
    OutputPane* GetOutputPane() { return m_outputPane; }

    /**
     * return the debugger pane
     * @return
     */
    DebuggerPane* GetDebuggerPane() { return m_debuggerPane; }

    /**
     * @return the workspace pane (the one that contained the Symbol view & class view)
     */
    SideBar* GetWorkspacePane() { return m_sidebar; }

    /**
     * @return the workspace pane (the one that contained the Symbol view & class view)
     */
    SecondarySideBar* GetSecondarySideBar() { return m_secondary_sidebar; }

    /**
     * return the workspace tab pane
     */
    WorkspaceTab* GetWorkspaceTab();

    /**
     * return the file explorer pane
     */
    FileExplorer* GetFileExplorer();

    /**
     * @return return AUI docking manager
     */
    wxAuiManager& GetDockingManager() { return m_mgr; }

    wxAuiManager* GetDockingManagerPtr() { return &m_mgr; }

    /**
     * Load session into LE
     */
    void LoadSession(const wxString& sessionName);

    /**
     * Compelete the main frame initialization
     * this method is called immediately after the
     * main frame construction is over.
     */
    void CompleteInitialization();

    void RegisterDockWindow(int menuItemId, const wxString& name);

    const GeneralInfo& GetFrameGeneralInfo() const { return m_frameGeneralInfo; }

    void OnSingleInstanceOpenFiles(clCommandEvent& e);
    void OnSingleInstanceRaise(clCommandEvent& e);

    /**
     * @brief build the given project
     * @param projectName
     */
    void BuildProject(const wxString& projectName);

    /**
     * @brief rebuild the given project
     * @param projectName
     */
    void RebuildProject(const wxString& projectName);

    /**
     * @brief execute a built program without attaching a debugger
     * @param promptToBuild
     */
    void ExecuteNoDebug(bool promptToBuild);

    /**
     * @brief handle custom build targets events
     */
    void OnBuildCustomTarget(wxCommandEvent& event);

    /**
     * @brief save the current IDE layout and session
     */
    bool SaveLayoutAndSession();

    /**
     * @brief save settings (such as whether to show the splashscreen) which should be saved
     * regardless of whether SaveLayoutAndSession is called
     */
    void SaveGeneralSettings();

    /**
     * @brief create the recently-opened-workspaces menu
     */
    void CreateRecentlyOpenedWorkspacesMenu();
    void DoSuggestRestart();

    void Bootstrap();

#ifdef __WXGTK__
    bool GetIsWaylandSession() const { return m_isWaylandSession; }
#endif

private:
    // make our frame's constructor private
    clMainFrame(wxWindow* pParent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
                long style = wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU |
                             wxRESIZE_BORDER | wxCLIP_CHILDREN);
    void AddKeyboardAccelerators();
    void StartTimer();

private:
    /**
     * Construct all the GUI controls of the main frame. this function is called
     * at construction time
     */
    void CreateGUIControls();

    /**
     * @brief construct the frame, we call this function in the first event loop after the application
     * is ready
     */
    void Construct();

    void PostConstruct();

    /**
     * Helper function that prompt user with a simple wxTextEntry dialog
     * @param msg message to display to user
     * @return user's string or wxEmptyString if 'Cancel' pressed.
     */
    void DispatchCommandEvent(wxCommandEvent& event);
    void DispatchUpdateUIEvent(wxUpdateUIEvent& event);

    void OnSplitSelection(wxCommandEvent& event);
    void OnSplitSelectionUI(wxUpdateUIEvent& event);

    /// Toolbar management
    void DoCreateToolBar(int toolSize);
    void ToggleToolBars(bool all);
    void ViewPaneUI(const wxString& paneName, wxUpdateUIEvent& event);
    void CreateRecentlyOpenedFilesMenu();
    void CreateWelcomePage();
    bool ReloadExternallyModifiedProjectFiles();
    /**
     * @brief show the startup wizard
     * @return true if a restart is needed
     */
    bool StartSetupWizard(bool firstTime);

    void DoShowCaptions(bool show);

public:
    void ViewPane(const wxString& paneName, bool checked);
    void ShowOrHideCaptions();
    clToolBarGeneric* GetPluginsToolBar() const { return m_pluginsToolbar; }
    void ShowBuildMenu(clToolBar* toolbar, wxWindowID buttonID);

protected:
    //----------------------------------------------------
    // event handlers
    //----------------------------------------------------
    void OnInfobarButton(wxCommandEvent& event);
    void OnDebugStarted(clDebugEvent& event);
    void OnDebugEnded(clDebugEvent& event);

    void OnRestoreDefaultLayout(wxCommandEvent& e);
    void OnIdle(wxIdleEvent& e);
    void OnBuildEnded(clBuildEvent& event);
    void OnQuit(wxCommandEvent& WXUNUSED(event));
    void OnClose(wxCloseEvent& event);
    void OnCustomiseToolbar(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnDuplicateTab(wxCommandEvent& event);
    void OnFileSaveUI(wxUpdateUIEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnFileReload(wxCommandEvent& event);
    void OnFileLoadTabGroup(wxCommandEvent& event);
    void OnNativeTBUnRedoDropdown(wxCommandEvent& event);
    void OnTBUnRedo(wxCommandEvent& event);
    void OnTBUnRedoMenu(wxCommandEvent& event);
    void OnCodeComplete(wxCommandEvent& event);
    void OnWordComplete(wxCommandEvent& event);
    void OnCompleteWordRefreshList(wxCommandEvent& event);
    void OnFunctionCalltip(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnReportIssue(wxCommandEvent& event);
    void OnCheckForUpdate(wxCommandEvent& e);
    void OnRunSetupWizard(wxCommandEvent& e);
    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileOpenFolder(wxCommandEvent& event);
    void OnFileClose(wxCommandEvent& event);
    void OnFileCloseUI(wxUpdateUIEvent& event);
    void OnFileSaveAll(wxCommandEvent& event);
    void OnFileSaveTabGroup(wxCommandEvent& event);
    void OnFileExistUpdateUI(wxUpdateUIEvent& event);
    void OnCopyFilePathRelativeToWorkspaceUI(wxUpdateUIEvent& event);
    void OnFileSaveAllUI(wxUpdateUIEvent& event);
    void OnCompleteWordUpdateUI(wxUpdateUIEvent& event);
    void OnFunctionCalltipUI(wxUpdateUIEvent& event);
    void OnIncrementalSearch(wxCommandEvent& event);
    void OnIncrementalReplace(wxCommandEvent& event);
    void OnIncrementalSearchUI(wxUpdateUIEvent& event);
    void OnPrint(wxCommandEvent& event);
    void OnPageSetup(wxCommandEvent& event);
    void OnRecentWorkspaceUI(wxUpdateUIEvent& e);
    void OnViewOptions(wxCommandEvent& event);
    void OnToggleMainTBars(wxCommandEvent& event);
    void OnTogglePluginTBars(wxCommandEvent& event);
    void OnTogglePanes(wxCommandEvent& event);
    void OnToggleMinimalView(wxCommandEvent& event);
    void OnToggleMinimalViewUI(wxUpdateUIEvent& event);
    void OnShowStatusBar(wxCommandEvent& event);
    void OnShowStatusBarUI(wxUpdateUIEvent& event);
    void OnShowToolbar(wxCommandEvent& event);
    void OnShowToolbarUI(wxUpdateUIEvent& event);
    void OnShowMenuBar(wxCommandEvent& event);
    void OnShowMenuBarUI(wxUpdateUIEvent& event);
    void OnShowTabBar(wxCommandEvent& event);
    void OnShowTabBarUI(wxUpdateUIEvent& event);
    void OnProjectNewWorkspace(wxCommandEvent& event);
    void OnProjectNewProject(wxCommandEvent& event);
    void OnReloadWorkspace(wxCommandEvent& event);
    void OnReloadWorkspaceUI(wxUpdateUIEvent& event);
    void OnSwitchWorkspace(wxCommandEvent& event);
    void OnSwitchWorkspaceUI(wxUpdateUIEvent& event);
    void OnNewWorkspaceUI(wxUpdateUIEvent& event);
    void OnCloseWorkspace(wxCommandEvent& event);
    void OnProjectAddProject(wxCommandEvent& event);
    void OnReconcileProject(wxCommandEvent& event);
    void OnWorkspaceOpen(wxUpdateUIEvent& event);
    void OnNewProjectUI(wxUpdateUIEvent& event);
    void OnRetagWorkspaceUI(wxUpdateUIEvent& event);
    void OnAddEnvironmentVariable(wxCommandEvent& event);
    void OnAdvanceSettings(wxCommandEvent& event);
    void OnCtagsOptions(wxCommandEvent& event);
    void OnBuildProject(wxCommandEvent& event);
    void OnBuildProjectOnly(wxCommandEvent& event);
    void OnShowBuildMenu(wxCommandEvent& e);
    void OnBuildAndRunProject(wxCommandEvent& event);
    void OnRebuildProject(wxCommandEvent& event);
    void OnRetagWorkspace(wxCommandEvent& event);
    void OnBuildProjectUI(wxUpdateUIEvent& event);
    void OnStopBuild(wxCommandEvent& event);
    void OnStopBuildUI(wxUpdateUIEvent& event);
    void OnStopExecutedProgram(wxCommandEvent& event);
    void OnStopExecutedProgramUI(wxUpdateUIEvent& event);
    void OnCleanProject(wxCommandEvent& event);
    void OnCleanProjectOnly(wxCommandEvent& event);
    void OnCleanProjectUI(wxUpdateUIEvent& event);
    void OnExecuteNoDebug(wxCommandEvent& event);
    void OnExecuteNoDebugUI(wxUpdateUIEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnFileCloseAll(wxCommandEvent& event);
    void OnQuickOutline(wxCommandEvent& event);
    void OnImportMSVS(wxCommandEvent& e);
    void OnDebugAttach(wxCommandEvent& event);
    void OnCopyFilePath(wxCommandEvent& event);
    void OnCopyFilePathRelativeToWorkspace(wxCommandEvent& event);
    void OnCopyFilePathOnly(wxCommandEvent& event);
    void OnCopyFileName(wxCommandEvent& event);
    void OnHighlightWord(wxCommandEvent& event);
    void OnHighlightWordUI(wxUpdateUIEvent& event);
    void OnShowNavBar(wxCommandEvent& e);
    void OnShowNavBarUI(wxUpdateUIEvent& e);
    void OnOpenShellFromFilePath(wxCommandEvent& e);
    void OnOpenFileExplorerFromFilePath(wxCommandEvent& e);
    void OnDetachEditor(wxCommandEvent& e);
    void OnMarkEditorReadonly(wxCommandEvent& e);
    void OnMarkEditorReadonlyUI(wxUpdateUIEvent& e);
    void OnDetachEditorUI(wxUpdateUIEvent& e);
    void OnQuickDebug(wxCommandEvent& e);
    void OnStartQuickDebug(clDebugEvent& e);
    void OnQuickDebugUI(wxUpdateUIEvent& e);
    void OnDebugCoreDump(wxCommandEvent& e);
    void OnNextFiFMatch(wxCommandEvent& e);
    void OnPreviousFiFMatch(wxCommandEvent& e);
    void OnNextFiFMatchUI(wxUpdateUIEvent& e);
    void OnPreviousFiFMatchUI(wxUpdateUIEvent& e);
    void OnGrepWord(wxCommandEvent& e);
    void OnGrepWordUI(wxUpdateUIEvent& e);
    void OnWebSearchSelection(wxCommandEvent& e);
    void OnWebSearchSelectionUI(wxUpdateUIEvent& e);
    void OnThemeChanged(wxCommandEvent& e);
    void OnEnvironmentVariablesModified(clCommandEvent& e);

    void OnRecentFile(wxCommandEvent& event);
    void OnRecentWorkspace(wxCommandEvent& event);
    void OnBackwardForward(wxCommandEvent& event);
    void OnBackwardForwardUI(wxUpdateUIEvent& event);

    void OnShowDebuggerWindow(wxCommandEvent& e);
    void OnShowDebuggerWindowUI(wxUpdateUIEvent& e);
    void OnDebug(wxCommandEvent& e);
    void OnDebugUI(wxUpdateUIEvent& e);
    void OnDebugRestart(wxCommandEvent& e);
    void OnDebugRestartUI(wxUpdateUIEvent& e);
    void OnDebugStop(wxCommandEvent& e);
    void OnDebugStopUI(wxUpdateUIEvent& e);
    void OnDebugManageBreakpointsUI(wxUpdateUIEvent& e);
    void OnDebugCmd(wxCommandEvent& e);
    void OnToggleReverseDebugging(wxCommandEvent& e);
    void OnToggleReverseDebuggingRecording(wxCommandEvent& e);
    void OnToggleReverseDebuggingUI(wxUpdateUIEvent& e);
    void OnToggleReverseDebuggingRecordingUI(wxUpdateUIEvent& e);
    void OnDebugCmdUI(wxUpdateUIEvent& e);

    void OnDebugRunToCursor(wxCommandEvent& e);
    void OnDebugJumpToCursor(wxCommandEvent& e);

    // Special UI handlers for debugger events
    void OnDebugStepInstUI(wxUpdateUIEvent& e);
    void OnDebugJumpToCursorUI(wxUpdateUIEvent& e);
    void OnDebugRunToCursorUI(wxUpdateUIEvent& e);
    void OnDebugInterruptUI(wxUpdateUIEvent& e);
    void OnDebugShowCursorUI(wxUpdateUIEvent& e);

    void OnDebuggerSettings(wxCommandEvent& e);
    void OnLinkClicked(wxHtmlLinkEvent& e);
    void OnLoadSession(wxCommandEvent& e);
    void OnAppActivated(wxActivateEvent& event);
    void OnReloadExternallModified(wxCommandEvent& e);
    void OnReloadExternallModifiedNoPrompt(wxCommandEvent& e);
    void OnCompileFile(wxCommandEvent& e);
    void OnCompileFileProject(wxCommandEvent& e);
    void OnCompileFileUI(wxUpdateUIEvent& e);
    void OnCloseAllButThis(wxCommandEvent& e);
    void OnCloseTabsToTheRight(wxCommandEvent& e);
    void OnWorkspaceMenuUI(wxUpdateUIEvent& e);
    void OnBuildWorkspace(wxCommandEvent& e);
    void OnBuildWorkspaceUI(wxUpdateUIEvent& e);
    void OnCleanWorkspace(wxCommandEvent& e);
    void OnCleanWorkspaceUI(wxUpdateUIEvent& e);
    void OnReBuildWorkspace(wxCommandEvent& e);
    void OnReBuildWorkspaceUI(wxUpdateUIEvent& e);

    // EOL
    void OnConvertEol(wxCommandEvent& e);
    void OnViewDisplayEOL(wxCommandEvent& e);
    void OnViewWordWrap(wxCommandEvent& e);
    void OnViewWordWrapUI(wxUpdateUIEvent& e);
    void OnViewDisplayEOL_UI(wxUpdateUIEvent& e);

    // Docking windows events
    void OnDockablePaneClosed(wxAuiManagerEvent& e);
    void OnViewPane(wxCommandEvent& event);
    void OnViewPaneUI(wxUpdateUIEvent& event);
    void OnDetachWorkspaceViewTab(wxCommandEvent& e);
    void OnHideWorkspaceViewTab(wxCommandEvent& e);
    void OnHideOutputViewTab(wxCommandEvent& e);
    void OnDetachDebuggerViewTab(wxCommandEvent& e);
    void OnNewDetachedPane(wxCommandEvent& e);
    void OnDestroyDetachedPane(wxCommandEvent& e);

    void OnManagePlugins(wxCommandEvent& e);
    void OnCppContextMenu(wxCommandEvent& e);

    void OnConfigureAccelerators(wxCommandEvent& e);
    void OnStartPageEvent(wxCommandEvent& e);
    void OnNewVersionAvailable(wxCommandEvent& e);
    void OnVersionCheckError(wxCommandEvent& e);
    void OnGotoCodeLiteDownloadPage(wxCommandEvent& e);
    void OnBatchBuild(wxCommandEvent& e);
    void OnBatchBuildUI(wxUpdateUIEvent& e);
    void OnSyntaxHighlight(wxCommandEvent& e);
    void OnShowWhitespaceUI(wxUpdateUIEvent& e);
    void OnShowWhitespace(wxCommandEvent& e);
    void OnNextTab(wxCommandEvent& e);
    void OnPrevTab(wxCommandEvent& e);
    void OnNextPrevTab_UI(wxUpdateUIEvent& e);
    void OnShowFullScreen(wxCommandEvent& e);
    void OnShowBuiltInTerminal(wxCommandEvent& e);
    void OnFindResourceXXX(wxCommandEvent& e);
    void OnShowActiveProjectSettings(wxCommandEvent& e);
    void OnShowActiveProjectSettingsUI(wxUpdateUIEvent& e);
    void OnWorkspaceSettings(wxCommandEvent& e);
    void OnWorkspaceEditorPreferences(wxCommandEvent& e);
    void OnSetActivePoject(wxCommandEvent& e);
    void OnSetActivePojectUI(wxUpdateUIEvent& e);

    void OnMainToolBarHide(wxCommandEvent& event);
    void OnMainToolBarHideUI(wxUpdateUIEvent& event);
    void OnMainToolBarPlaceTop(wxCommandEvent& event);
    void OnMainToolBarPlaceTopUI(wxUpdateUIEvent& event);
    void OnMainToolBarPlaceBottom(wxCommandEvent& event);
    void OnMainToolBarPlaceBottomUI(wxUpdateUIEvent& event);
    void OnMainToolBarPlaceLeft(wxCommandEvent& event);
    void OnMainToolBarPlaceLeftUI(wxUpdateUIEvent& event);
    void OnMainToolBarPlaceRight(wxCommandEvent& event);
    void OnMainToolBarPlaceRightUI(wxUpdateUIEvent& event);

    // Misc
    void OnActivateEditor(wxCommandEvent& e);
    void OnActiveEditorChanged(wxCommandEvent& e);
    void OnWorkspaceLoaded(clWorkspaceEvent& e);
    void OnWorkspaceClosed(clWorkspaceEvent& e);
    void OnChangeActiveBookmarkType(wxCommandEvent& e);
    void OnSettingsChanged(wxCommandEvent& e);
    void OnEditMenuOpened(wxMenuEvent& e);
    void OnProjectRenamed(clCommandEvent& event);

    // Search handlers
    void OnFindNext(wxCommandEvent& event);
    void OnFindPrevious(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif // LITEEDITOR_FRAME_H
