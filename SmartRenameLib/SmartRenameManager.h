#pragma once
#include "SmartRenameInterfaces.h"
#include <vector>
#include "srwlock.h"

class CSmartRenameManager :
    public ISmartRenameManager,
    public ISmartRenameRegExEvents
{
public:
    // IUnknown
    IFACEMETHODIMP  QueryInterface(_In_ REFIID iid, _Outptr_ void** resultInterface);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ISmartRenameManager
    IFACEMETHODIMP Advise(_In_ ISmartRenameManagerEvents* renameOpEvent, _Out_ DWORD *cookie);
    IFACEMETHODIMP UnAdvise(_In_ DWORD cookie);
    IFACEMETHODIMP Start();
    IFACEMETHODIMP Stop();
    IFACEMETHODIMP Reset();
    IFACEMETHODIMP Shutdown();
    IFACEMETHODIMP AddItem(_In_ ISmartRenameItem* pItem);
    IFACEMETHODIMP GetItem(_In_ UINT index, _COM_Outptr_ ISmartRenameItem** ppItem);
    IFACEMETHODIMP GetItemCount(_Out_ UINT* count);
    IFACEMETHODIMP get_smartRenameRegEx(_COM_Outptr_ ISmartRenameRegEx** ppRegEx);
    IFACEMETHODIMP put_smartRenameRegEx(_In_ ISmartRenameRegEx* pRegEx);
    IFACEMETHODIMP get_smartRenameItemFactory(_COM_Outptr_ ISmartRenameItemFactory** ppItemFactory);
    IFACEMETHODIMP put_smartRenameItemFactory(_In_ ISmartRenameItemFactory* pItemFactory);

    // ISmartRenameRegExEvents
    IFACEMETHODIMP OnSearchTermChanged(_In_ PCWSTR searchTerm);
    IFACEMETHODIMP OnReplaceTermChanged(_In_ PCWSTR replaceTerm);
    IFACEMETHODIMP OnFlagsChanged(_In_ DWORD flags);

    static HRESULT s_CreateInstance(_COM_Outptr_ ISmartRenameManager** ppsrm);

private:
    CSmartRenameManager();
    ~CSmartRenameManager();

    HRESULT _Init();
    void _Cleanup();

    void _Cancel();

    void _OnItemAdded(_In_ ISmartRenameItem* renameItem);
    void _OnUpdate(_In_ ISmartRenameItem* renameItem);
    void _OnError(_In_ ISmartRenameItem* renameItem);
    void _OnRegExStarted();
    void _OnRegExCanceled();
    void _OnRegExCompleted();
    void _OnRenameStarted();
    void _OnRenameCompleted();

    void _ClearEventHandlers();
    void _ClearSmartRenameItems();

    bool _PathIsDotOrDotDot(_In_ PCWSTR path);
    bool _EnumeratePath(_In_ PCWSTR path, _In_ UINT depth);

    HRESULT _PerformRegExRename();
    HRESULT _PerformFileOperation();

    HRESULT _CreateRegExWorkerThread();
    HRESULT _CreateFileOpWorkerThread();

    // Thread proc for performing the regex rename of each item
    static DWORD WINAPI s_regexWorkerThread(_In_ void* pv);
    // Thread proc for performing the actual file operation that does the file rename
    static DWORD WINAPI s_fileOpWorkerThread(_In_ void* pv);

    static bool _ShouldRenameItem(_In_ ISmartRenameItem* item, _In_ DWORD flags);

    HANDLE m_regExWorkerThreadHandle = nullptr;
    HANDLE m_startRegExWorkerEvent = nullptr;
    HANDLE m_cancelRegExWorkerEvent = nullptr;

    HANDLE m_fileOpWorkerThreadHandle = nullptr;
    HANDLE m_startFileOpWorkerEvent = nullptr;

    CSRWLock m_lockEvents;
    CSRWLock m_lockItems;

    DWORD m_cookie = 0;

    struct SMART_RENAME_MGR_EVENT
    {
        ISmartRenameManagerEvents* pEvents;
        DWORD cookie;
    };

    CComPtr<ISmartRenameItemFactory> m_spItemFactory;
    CComPtr<ISmartRenameRegEx> m_spRegEx;

    _Guarded_by_(m_lockEvents) std::vector<SMART_RENAME_MGR_EVENT> m_SmartRenameManagerEvents;
    _Guarded_by_(m_lockItems) std::vector<ISmartRenameItem*> m_smartRenameItems;

    long m_refCount;
};