#ifndef CLCODELITEREMOTEPROCESS_HPP
#define CLCODELITEREMOTEPROCESS_HPP

#include "AsyncProcess/asyncprocess.h"
#include "cl_command_event.h"
#include "codelite_exports.h"
#include "ssh/ssh_account_info.h"

#include <deque>
#include <functional>
#include <memory>
#include <vector>
#include <wx/arrstr.h>
#include <wx/event.h>
#include <wx/string.h>

class WXDLLIMPEXP_SDK clCodeLiteRemoteProcess : public wxEvtHandler
{
protected:
    typedef void (clCodeLiteRemoteProcess::*CallbackFunc)(const wxString&, bool);
    typedef std::function<void(const wxString&)> UserCallback;
    struct CallbackOptions {
        CallbackFunc func = nullptr;
        IProcess* handler = nullptr;
        UserCallback user_callback = nullptr;

        // When user_callback is used, we aggregate the output here until "is_completed"
        // is true, only then we call the user_callback
        wxString aggregated_output;
        CallbackOptions(CallbackFunc func, IProcess* handler, UserCallback user_callback)
        {
            this->func = func;
            this->handler = handler;
            this->user_callback = user_callback;
        }
    };

protected:
    std::unique_ptr<IProcess> m_process;
    std::deque<CallbackOptions> m_completionCallbacks;
    wxString m_outputRead;
    size_t m_fif_matches_count = 0;
    size_t m_fif_files_scanned = 0;
    bool m_going_down = false;
    wxString m_context;
    SSHAccountInfo m_account;
    wxString m_scriptPath;

protected:
    void OnProcessOutput(clProcessEvent& e);
    void OnProcessTerminated(clProcessEvent& e);
    void Cleanup();
    void ProcessOutput();
    bool GetNextBuffer(wxString& raw_input_buffer, wxString& output, bool& is_completed);
    void ResetStates();

    // prepare an event from list command output
    void OnListFilesOutput(const wxString& output, bool is_completed);
    void OnListLSPsOutput(const wxString& output, bool is_completed);
    void OnFindOutput(const wxString& buffer, bool is_completed);
    void OnReplaceOutput(const wxString& buffer, bool is_completed);
    void OnLocateOutput(const wxString& buffer, bool is_completed);
    void OnFindPathOutput(const wxString& buffer, bool is_completed);
    void OnExecOutput(const wxString& buffer, bool is_completed);
    bool DoExec(const wxString& cmd,
                const wxString& working_directory,
                const clEnvList_t& env,
                IProcess* handler = nullptr,
                UserCallback cb = nullptr);

    template <typename Container>
    wxString GetCmdString(const Container& args) const
    {
        if (args.empty()) {
            return wxEmptyString;
        }

        wxString cmdstr;
        for (auto arg : args) {
            if (arg.Contains(" ")) {
                // escape any " before we start escaping
                arg.Replace("\"", "\\\"");
                // now wrap with double quotes
                arg.Prepend("\"").Append("\"");
            }
            cmdstr << arg << " ";
        }
        cmdstr.RemoveLast();
        return cmdstr;
    }

public:
    clCodeLiteRemoteProcess();
    virtual ~clCodeLiteRemoteProcess();

    /**
     * @brief start the process using the same arguments used in the last call to StartInteractive. If the process is
     * running, do nothing
     */
    void StartIfNotRunning();

    const wxString& GetContext() const { return m_context; }
    /**
     * @brief start remote process in an interactive mode
     */
    void StartInteractive(const SSHAccountInfo& account, const wxString& scriptPath, const wxString& contextString);
    /**
     * @brief save as above, but with the account name
     */
    void StartInteractive(const wxString& account, const wxString& scriptPath, const wxString& contextString);

    void Stop();
    bool IsRunning() const { return m_process != nullptr; }

    // API

    /**
     * @brief find all files on a remote machine from a given directory that matches the extensions list
     * @extensions a comma/semi colon separate list of patterns to include from the file list (e.g. "*.cpp")
     * @exclude_extensions a comma/semi colon separate list of patterns to exclude from the file list (e.g. "*.pyc")
     * @exclude_patterns a comma/semi colon separate list of patterns to exclude from the file list (e.g. "build-debug")
     */
    void ListFiles(const wxString& root_dir,
                   const wxString& extensions,
                   const wxString& exclude_extensions,
                   const wxString& exclude_patterns);

    /**
     * @brief list all configured LSPs on the remote machine
     * the configuration is read from `codelite-remote.json` config file
     */
    void ListLSPs();

    /**
     * @brief find in files on a remote machine
     */
    void Search(const wxString& root_dir,
                const wxString& extensions,
                const wxString& exclude_patterns,
                const wxString& find_what,
                bool whole_word,
                bool icase);

    /**
     * @brief replace in file on a remote machine
     */
    void Replace(const wxString& root_dir,
                 const wxString& extensions,
                 const wxString& exclude_patterns,
                 const wxString& find_what,
                 const wxString& replace_with,
                 bool whole_word,
                 bool icase);

    /**
     * @brief execute a command on the remote machine
     */
    void Exec(const wxArrayString& args, const wxString& working_directory, const clEnvList_t& env);

    /**
     * @brief execute a command on the remote machine trigger "cb" when output arrives
     */
    void ExecWithCallback(const wxArrayString& args,
                          UserCallback cb,
                          const wxString& working_directory = wxEmptyString,
                          const clEnvList_t& env = {});

    /**
     * @brief attempt to locate a file on the remote machine with possible version number
     */
    void Locate(const wxString& path, const wxString& name, const wxString& ext, const std::vector<wxString>& = {});

    /**
     * @brief execute a command on the remote machine
     */
    void Exec(const wxString& cmd, const wxString& working_directory, const clEnvList_t& env);

    /**
     * @brief find a path from. if path does not exist, check the parent folder
     * going up until we hit the root path
     */
    void FindPath(const wxString& path);

    /**
     * @brief call 'exec' and return an instance of IProcess. This method is for compatibility with the
     * CreateAsyncProcess family of functions
     * @note it is up to the caller to delete the return process object
     */
    IProcess* CreateAsyncProcess(wxEvtHandler* handler,
                                 const wxString& cmd,
                                 const wxString& working_directory,
                                 const clEnvList_t& env);
    /**
     * @brief call 'exec' with callback
     */
    void CreateAsyncProcessCB(const wxString& cmd,
                              std::function<void(const wxString&)> callback,
                              const wxString& working_directory,
                              const clEnvList_t& env);
    /**
     * @brief call 'exec' and return an instance of IProcess. This method is for compatibility with the
     * CreateAsyncProcess family of functions
     * @note it is up to the caller to delete the return process object
     */
    template <typename Container>
    IProcess* CreateAsyncProcess(wxEvtHandler* handler,
                                 const Container& cmd,
                                 const wxString& working_directory,
                                 const clEnvList_t& env)
    {
        wxString cmdstr = GetCmdString(cmd);
        return CreateAsyncProcess(handler, cmdstr, working_directory, env);
    }
    /**
     * @brief execute remote process `cmd` and wait for its output
     */
    bool SyncExec(const wxString& cmd, const wxString& working_directory, const clEnvList_t& env, wxString* output);
    /**
     * @brief write string to the running process's stdin
     */
    void Write(const wxString& str);
};

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_RESTARTED, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_LIST_FILES, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_LIST_FILES_DONE, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_FIND_RESULTS, clFindInFilesEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_FIND_RESULTS_DONE, clFindInFilesEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_REPLACE_RESULTS, clFindInFilesEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_REPLACE_DONE, clFindInFilesEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_EXEC_OUTPUT, clProcessEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_EXEC_DONE, clProcessEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_LOCATE, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_LOCATE_DONE, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_FINDPATH, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_FINDPATH_DONE, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_LIST_LSPS, clCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_SDK, wxEVT_CODELITE_REMOTE_LIST_LSPS_DONE, clCommandEvent);
#endif // CLCODELITEREMOTEPROCESS_HPP
