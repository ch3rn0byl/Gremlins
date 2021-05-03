#pragma once
#include "typedefs.h"

typedef union _EX_FAST_REF
{
    PVOID64 Object = nullptr;
    union
    {
        UINT64 RefCnt : 4; /// bits 0 - 3
    } Value;
} EX_FAST_REF, * PEX_FAST_REF;

typedef struct _RTL_AVL_TREE
{
    PRTL_BALANCED_NODE Root;
} RTL_AVL_TREE, * PRTL_AVL_TREE;

typedef struct _KSTACK_COUNT
{
    union
    {
        UINT32 State : 3;       /// bits 0 - 2
        UINT32 StackCount : 29; /// bits 3 - 31
    } Value;
} KSTACK_COUNT, * PKSTACK_COUNT;

typedef struct _KEXECUTE_OPTIONS
{
    union
    {
        UINT8 ExecuteDisable : 1;                   /// bit 0
        UINT8 ExecuteEnable : 1;                    /// bit 1
        UINT8 DisableThunkEmulation : 1;            /// bit 2
        UINT8 Permanent : 1;                        /// bit 3
        UINT8 ExecuteDispatchEnable : 1;            /// bit 4
        UINT8 ImageDispatchEnable : 1;              /// bit 5
        UINT8 DisableExceptionChainValidation : 1;  /// bit 6
        UINT8 Spare : 1;                            /// bit 7
        union
        {
            UINT8 ExecuteOptions;
            UINT8 ExecuteOptionsNV;
        };
    };
} KEXECUTE_OPTIONS, * PKEXECUTE_OPTIONS;

typedef struct _KAFFINITY_EX
{
    UINT16 Count = 0;
    UINT16 Size = 0;
    UINT32 Reserved = 0;
    UINT64 Bitmap[20] = { 0 };
} KAFFINITY_EX, * PKAFFINITY_EX;

typedef struct _KPROCESS
{
    DISPATCHER_HEADER Header;
    LIST_ENTRY ProfileListHead;
    UINT64 DirectoryTableBase = 0;
    LIST_ENTRY ThreadListHead;
    UINT32 ProcessLock = 0;
    UINT32 ProcessTimerDelay = 0;
    UINT64 DeepFreezeStartTime = 0;
    KAFFINITY_EX Affinity;
    UINT64 AffinityPadding[12] = { 0 };
    LIST_ENTRY ReadyListHead;
    SINGLE_LIST_ENTRY SwapListEntry;
    KAFFINITY_EX ActiveProcessors;
    UINT64 ActiveProcessorsPadding[12];

    union
    {
        UINT32 AutoAlignment : 1;           /// bit 0
        UINT32 DisableBoost : 1;            /// bit 1
        UINT32 DisableQuantum : 1;          /// bit 2
        UINT32 DeepFreeze : 1;              /// bit 3
        UINT32 TimerVirtualization : 1;     /// bit 4
        UINT32 CheckStackExtents : 1;       /// bit 5
        UINT32 CacheIsolationEnabled : 1;   /// bit 6
        UINT32 PpmPolicy : 3;               /// bits 7 - 9
        UINT32 VaSpaceDeleted : 1;          /// bit 10
        UINT32 ReservedFlags : 21;          /// bits 11 - 31
    } ProcessFlags;

    UINT32 ActiveGroupsMask = 0;
    UINT8 BasePriority = 0;
    UINT8 QuantumReset = 0;
    UINT8 Visited = 0;
    KEXECUTE_OPTIONS Flags;
    UINT16 ThreadSeed[20] = { 0 };
    UINT16 ThreadSeedPadding[12] = { 0 };
    UINT16 IdealProcessor[20] = { 0 };
    UINT16 IdealProcessorPadding[12] = { 0 };
    UINT16 IdealNode[20] = { 0 };
    UINT16 IdealNodePadding[12] = { 0 };
    UINT16 IdealGlobalNode = 0;
    UINT16 Spare1 = 0;
    KSTACK_COUNT StackCount;
    LIST_ENTRY ProcessListEntry;
    UINT64 CycleTime = 0;
    UINT64 ContextSwitches = 0;

    /// This one was wayy too boring to break out, no way foo
    PVOID64 SchedulingGroup = nullptr;
    UINT32 FreezeCount = 0;
    UINT32 KernelTime = 0;
    UINT32 UserTime = 0;
    UINT32 ReadyTime = 0;
    UINT64 UserDirectoryTableBase = 0;
    UINT8 AddressPolicy = 0;
    UINT8 Spare2[71] = { 0 };
    PVOID64 InstrumentationCallback = nullptr;

    ///
    /// This one is named anonmous tag, but the offset to next entry is +8
    /// 
    PVOID64 SecureState;
    UINT64 KernelWaitTime = 0;
    UINT64 UserWaitTime = 0;
    UINT64 EndPadding[8] = { 0 };
} KPROCESS, * PKPROCESS;

///==================================================================================
///==================================================================================
typedef struct _LEAP_SECOND_DATA
{
    UINT8 Enabled = 0;
    UINT32 Count = 0;
    LARGE_INTEGER Data[1];
} LEAP_SECOND_DATA, * PLEAP_SECOND_DATA;

///
/// All the structs for _RTL_*
/// 
typedef struct _RTL_CRITICAL_SECTION_DEBUG
{
    UINT16 Type = 0;
    UINT16 CreatorBackTraceIndex = 0;
    PVOID64 CriticalSection = nullptr; /// type: _RTL_CRITICAL_SECTION
    LIST_ENTRY ProcessLocksList;
    UINT32 EntryCount = 0;
    UINT32 ContentionCount = 0;
    UINT32 Flags = 0;
    UINT16 CreatorBackTraceIndexHigh = 0;
    UINT16 SpareUSHORT = 0;
} RTL_CRITICAL_SECTION_DEBUG, * PRTL_CRITICAL_SECTION_DEBUG;

typedef struct _RTL_CRITICAL_SECTION
{
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    UINT32 LockCount = 0;
    UINT32 RecursionCount = 0;
    PVOID64 OwningThread = nullptr;
    PVOID64 LockSemaphore = nullptr;
    UINT64 SpinCount = 0;
} RTL_CRITICAL_SECTION, * PRTL_CRITICAL_SECTION;

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
    UINT16 Flags = 0;
    UINT16 Length = 0;
    UINT32 TimeStamp = 0;
    STRING DosPath = { 0 };
} RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;

typedef struct _CURDIR
{
    UNICODE_STRING DosPath = { 0 };
    PVOID64 Handle = nullptr;
} CURDIR, * PCURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    UINT32 MaximumLength = 0;
    UINT32 Length = 0;
    UINT32 Flags = 0;
    UINT32 DebugFlags = 0;
    PVOID64 ConsoleHandle = nullptr;
    UINT32 ConsoleFlags = 0;
    PVOID64 StandardInput = nullptr;
    PVOID64 StandardOutput = nullptr;
    PVOID64 StandardError = nullptr;
    CURDIR CurrentDirectory;
    UNICODE_STRING DllPath = { 0 };
    UNICODE_STRING ImagePathName = { 0 };
    UNICODE_STRING CommandLine = { 0 };
    PVOID64 Environment = nullptr;
    UINT32 StartingX = 0;
    UINT32 StartingY = 0;
    UINT32 CountX = 0;
    UINT32 CountY = 0;
    UINT32 CountCharsX = 0;
    UINT32 CountCharsY = 0;
    UINT32 FillAttribute = 0;
    UINT32 WindowsFlags = 0;
    UINT32 ShowWindowsFlags = 0;
    UNICODE_STRING WindowTitle = { 0 };
    UNICODE_STRING DesktopInfo = { 0 };
    UNICODE_STRING ShellInfo = { 0 };
    UNICODE_STRING RuntimeData = { 0 };
    RTL_DRIVE_LETTER_CURDIR CurrentDirectores[32] = { 0 };
    UINT64 EnvironmentSize = 0;
    UINT64 EnvironmentVersion = 0;
    PVOID64 PackageDependencyData = nullptr;
    UINT32 ProcessGroupId = 0;
    UINT32 LoaderThreads = 0;
    UNICODE_STRING RedirectionDllName = { 0 };
    UNICODE_STRING HeapPartitionName = { 0 };
    UINT64* DefaultThreadpoolCpuSetMasks = nullptr;
    UINT32 DefaultThreadpoolCpuSetMaskCount = 0;
    UINT32 DefaultThreadpoolThreadMaximum = 0;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

///
/// ALAS, structs for the PEB as shown in WinDbg
///
typedef struct _PEB_LDR_DATA
{
    UINT32 Length = 0;
    UINT32 Initialized = 0;
    PVOID64 SsHandle = nullptr;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID64 EntryInProgress = nullptr;
    UINT64 ShutdownInProgress = 0;
    PVOID64 ShutdownThreadId = nullptr;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB_WINDBG
{
    UINT8 InheritedAddressSpace = 0;
    UINT8 ReadImageFileExecOptions = 0;
    UINT8 BeingDebugged = 0;

    union
    {
        struct
        {
            UINT8 ImageUsesLargePages : 1;            /// bit 0
            UINT8 IsProtectedProcess : 1;             /// bit 1
            UINT8 IsImageDynamicallyRelocated : 1;    /// bit 2
            UINT8 SkipPatchingUser32Forwarders : 1;   /// bit 3
            UINT8 IsPackagedProcess : 1;              /// bit 4
            UINT8 IsAppContainer : 1;                 /// bit 5
            UINT8 IsProtectedProcessLight : 1;        /// bit 6
            UINT8 IsLongPathAwareProcess : 1;         /// bit 7
        } flags;
    } BitField;

    UINT8 Padding0[4] = { 0 };
    PVOID64 Mutant = nullptr;
    PVOID64 ImageBaseAddress = nullptr;
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID64 SubSystemData = nullptr;
    PVOID64 ProcessHeap = nullptr;
    PRTL_CRITICAL_SECTION FastPebLock = nullptr;
    PSLIST_HEADER AtlThunkSListPtr = nullptr;
    PVOID64 IFEOKey = nullptr;

    union
    {
        struct
        {
            UINT32 ProcessInJob : 1;              /// bit 0
            UINT32 ProcessInitializing : 1;       /// bit 1
            UINT32 ProcessUsingVEH : 1;           /// bit 2
            UINT32 ProcessUsingVCH : 1;           /// bit 3
            UINT32 ProcessUsingFTH : 1;           /// bit 4
            UINT32 ProcessPreviouslyThrottled : 1;/// bit 5
            UINT32 ProcessCurrentlyThrottled : 1; /// bit 6
            UINT32 ProcessImageHotPatched : 1;    /// bit 7
            UINT32 ReservedBits0 : 24;            /// bits 8 - 32
        } flags;
    } CrossProcessFlags;

    UINT8 Padding1[4] = { 0 };

    union
    {
        PVOID64 KernelCallbackTable;
        PVOID64 UserSharedInfoPtr;
    };

    UINT32 SystemReserved = 0;
    UINT32 AtlThunkSListPtr32 = 0;
    PVOID64 ApiSetMap = nullptr;
    UINT32 TlsExpansionCounter = 0;
    UINT8 Padding2[4] = { 0 };
    PVOID64 TlsBitmap = nullptr;
    UINT32 TlsBitmapBits[2] = { 0 };
    PVOID64 ReadOnlySharedMemoryBase = nullptr;
    PVOID64 SharedData = nullptr;
    PVOID64* ReadOnlyStaticServerData = nullptr;
    PVOID64 AnsiCodePageData = nullptr;
    PVOID64 OemCodePageData = nullptr;
    PVOID64 UnicodeCaseTableData = nullptr;
    UINT32 NumberOfProcessors = 0;
    UINT32 NtGlobalFlag = 0;
    LARGE_INTEGER CriticalSectionTimeout = { 0 };
    UINT64 HeapSegmentReserve = 0;
    UINT64 HeapSegmentCommit = 0;
    UINT64 HeapDeCommitTotalFreeThreshold = 0;
    UINT64 HeapDeCommitFreeBlockThreshold = 0;
    UINT32 NumberOfHeaps = 0;
    UINT32 MaximumNumberOfHeaps = 0;
    PVOID64* ProcessHeaps = nullptr;
    PVOID64 GdiSharedHandleTable = nullptr;
    PVOID64 ProcessStarterHelper = nullptr;
    UINT32 GdiDCAttributeList = 0;
    UINT8 Padding3[4] = { 0 };
    PRTL_CRITICAL_SECTION LoaderLock = nullptr;
    UINT32 OSMajorVersion = 0;
    UINT32 OSMinorVersion = 0;
    UINT16 OSBuildNumber = 0;
    UINT16 OSCSDVersion = 0;
    UINT32 OSPlatformId = 0;
    UINT32 ImageSubsystem = 0;
    UINT32 ImageSubsystemMajorVersion = 0;
    UINT32 ImageSubsystemMinorVersion = 0;
    UINT8 Padding4[4] = { 0 };
    UINT64 ActiveProcessAffinityMask = 0;
    UINT32 GdiHandleBuffer[60] = { 0 };
    PVOID64 PostProcessInitRoutine = nullptr;
    PVOID64 TlsExpansionBitmap = nullptr;
    UINT32 TlsExpansionBitmapBits[32] = { 0 };
    UINT32 SessionId = 0;
    UINT8 Padding5[4] = { 0 };
    ULARGE_INTEGER AppCompatFlags = { 0 };
    ULARGE_INTEGER AppCompatFlagsUser = { 0 };
    PVOID64 pShimData = nullptr;
    PVOID64 AppCompatInfo = nullptr;
    UNICODE_STRING CSDVersion = { 0 };
    PVOID64 ActivationContextData = nullptr;                /// type: PACTIVATION_CONTEXT_DATA
    PVOID64 ProcessAssemblyStorageMap = nullptr;            /// type: PASSEMBLY_STORAGE_MAP
    PVOID64 SystemDefaultAcitvationContextData = nullptr;   /// type: PACTIVATION_CONTEXT_DATA
    PVOID64 SystemAssemblyStorageMap = nullptr;             /// type: PASSEMBLY_STORAGE_MAP
    UINT64 MinimumStackCommit = 0;
    PVOID64 SparePointers[4] = { 0 };
    UINT32 SpareUlongs[5] = { 0 };
    PVOID64 WerRegistrationData = nullptr;
    PVOID64 WerShipAssertPtr = nullptr;
    PVOID64 pUnused = nullptr;
    PVOID64 pImageHeaderHash = nullptr;

    union
    {
        struct
        {
            UINT32 HeapTracingEnabled : 1;        /// bit 0
            UINT32 CritSecTracingEnabled : 1;     /// bit 1
            UINT32 LibLoaderTracingEnabled : 1;   /// bit 2
            UINT32 SpareTracingBits : 29;         /// bits 3 - 32
        } flags;
    } TracingFlags;

    UINT8 Padding6[4] = { 0 };
    UINT64 CsrServerReadOnlySharedMemoryBase = 0;
    UINT64 TppWorkerpListLock = 0;
    LIST_ENTRY TppWorkerpList;
    PVOID64 WaitOnAddressHashTable[128] = { 0 };
    PVOID64 TelemetryCoverageHeader = nullptr;
    UINT32 CloudFileFlags = 0;
    UINT32 CloudFileDiagFlags = 0;
    INT8 PlaceholderCompatibilityMode = 0;
    INT8 PlaceholderCompatibilityModeReserved[7] = { 0 };
    PLEAP_SECOND_DATA LeapSecondData = nullptr;

    union
    {
        struct
        {
            UINT32 SixtySecondEnabled : 1;/// bit 0
            UINT32 Reserved : 31;         /// bits 1 - 31
        } flags;
    } LeapSecondFlags;

    UINT32 NtGlobalFlag2 = 0;
} PEB_WINDBG, * PPEB_WINDBG;

///==================================================================================
///==================================================================================

typedef struct _EPROCESS_WINDBG
{
    KPROCESS Pcb;
    EX_PUSH_LOCK ProcessLock;
    PVOID64 UniqueProcessId = nullptr;
    LIST_ENTRY ActiveProcessLinks;
    EX_RUNDOWN_REF RundownProtect;

    union
    {
        UINT32 JobNotReallyActive : 1;              /// bit 0
        UINT32 AccountingFolded : 1;                /// bit 1
        UINT32 NewProcessReported : 1;              /// bit 2
        UINT32 ExitProcessReported : 1;             /// bit 3
        UINT32 ReportCommitChanges : 1;             /// bit 4
        UINT32 LastReportMemory : 1;                /// bit 5
        UINT32 ForceWakeCharge : 1;                 /// bit 6
        UINT32 CrossSessionCreate : 1;              /// bit 7
        UINT32 NeedsHandleRundown : 1;              /// bit 8
        UINT32 RefTraceEnabled : 1;                 /// bit 9
        UINT32 PicoCreated : 1;                     /// bit 10
        UINT32 EmptyJobEvaluated : 1;               /// bit 11
        UINT32 DefaultPagePriority : 3;             /// bits 12 - 14
        UINT32 PrimaryTokenFrozen : 1;              /// bit 15
        UINT32 ProcessVerifierTarget : 1;           /// bit 16
        UINT32 RestrictSetThreadContext : 1;        /// bit 17
        UINT32 AffinityPermanent : 1;               /// bit 18
        UINT32 AffinityUpdateEnabled : 1;           /// bit 19
        UINT32 PropagateNode : 1;                   /// bit 20
        UINT32 ExplicitAffinity : 1;                /// bit 21
        UINT32 ProcessExecutionState : 2;           /// bits 22 - 23
        UINT32 EnableReadVmLogging : 1;             /// bit 24
        UINT32 EnableWriteVmLogging : 1;            /// bit 25
        UINT32 FatalAccessTerminationRequested : 1; /// bit 26
        UINT32 DisableSystemAllowedCputSet : 1;     /// bit 27
        UINT32 ProcessStateChangeRequest : 1;       /// bit 28
        UINT32 ProcessStateChangeInProgress : 2;    /// bits 29 - 30
        UINT32 InPrivate : 1;                       /// bit 31
    } Flags2;

    union
    {
        UINT32 CreateReported : 1;              /// bit 0
        UINT32 NoDebugInherit : 1;              /// bit 1
        UINT32 ProcessExiting : 1;              /// bit 2
        UINT32 ProcessDelete : 1;               /// bit 3
        UINT32 ManageExecutableMemoryWrites : 1;/// bit 4
        UINT32 VmDeleted : 1;                   /// bit 5
        UINT32 OutswapEnabled : 1;              /// bit 6
        UINT32 Outswapped : 1;                  /// bit 7
        UINT32 FailFastOnCommitFail : 1;        /// bit 8
        UINT32 Wow64VaSpace4Gb : 1;             /// bit 9
        UINT32 AddressSpaceInitialized : 2;     /// bits 10 - 11
        UINT32 SetTimerResolution : 1;          /// bit 12
        UINT32 BreakOnTermination : 1;          /// bit 13
        UINT32 DeprioritizeViews : 1;           /// bit 14
        UINT32 WriteWatch : 1;                  /// bit 15
        UINT32 ProcessInSession : 1;            /// bit 16
        UINT32 OverrideAddressSpace : 1;        /// bit 17
        UINT32 HasAddressSpace : 1;             /// bit 18
        UINT32 LaunchPrefetched : 1;            /// bit 19
        UINT32 Background : 1;                  /// bit 20
        UINT32 VmTopDown : 1;                   /// bit 21
        UINT32 ImageNotifyDone : 1;             /// bit 22
        UINT32 PdeUpdateNeeded : 1;             /// bit 23
        UINT32 VdmAllowed : 1;                  /// bit 24
        UINT32 ProcessRundown : 1;              /// bit 25
        UINT32 ProcessInserted : 1;             /// bit 26
        UINT32 DefaultIoPriority : 3;           /// bits 27 - 29
        UINT32 ProcessSelfDelete : 1;           /// bit 30
        UINT32 SetTimerResolutionLink : 1;      /// bit 31
    } Flags;

    LARGE_INTEGER CreateTime;
    UINT64 ProcessQuotaUsage[2] = { 0 };
    UINT64 ProcessQuotaPeak[2] = { 0 };
    UINT64 PeakVirtualSize = 0;
    UINT64 VirtualSize = 0;
    LIST_ENTRY SessionProcessLInks;

    union
    {
        PVOID64 ExceptionPortData = nullptr;
        union
        {
            UINT64 ExceptionPortState : 3;  /// bits 0 - 2
        } ExceptionPortValue;
    };

    EX_FAST_REF Token;
    UINT64 MmReserved = 0;
    EX_PUSH_LOCK AddressCreationLock;
    EX_PUSH_LOCK PageTableCommitmentLock;
    PVOID64 RotateInProgress = nullptr;
    PVOID64 ForkInProgress = nullptr;
    PVOID64 CommitChargeJob = nullptr;
    RTL_AVL_TREE CloneRoot;
    UINT64 NumberOfPrivatePages = 0;
    UINT64 NumberOfLockedPages = 0;
    PVOID64 Win32Process = nullptr;
    PVOID64 Job = nullptr;
    PVOID64 SectionObject = nullptr;
    PVOID64 SectionBaseAddress = nullptr;
    UINT32 Cookie = 0;
    PVOID64 WorkingSetWatch = nullptr;
    PVOID64 Win32WindowStation = nullptr;
    PVOID64 InheritedFromUniqueProcessId = 0;
    UINT64 OwnerProcessId = 0;
    PPEB_WINDBG Peb;
} EPROCESS_WINDBG, * PEPROCESS_WINDBG;


/// EOF