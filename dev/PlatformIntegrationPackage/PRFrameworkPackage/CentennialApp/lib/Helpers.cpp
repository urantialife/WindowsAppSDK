//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <pch.h>

#include <Shobjidl.h>
#include <wil/com.h>                                // for CoCreateInstance<>

#include <wrl.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <Windows.Foundation.h>
#include <Windows.h>

#include <msxml.h>
#include <appmodel.h>

#include <Helpers.h>
#include <BasicLogFile.h>

#define RETURN_IF_NOT_FOUND_OR_ERROR(hr, ptr) \
    RETURN_HR_IF_EXPECTED(S_OK, (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) || (SUCCEEDED(hr) && (ptr == nullptr))); \
    RETURN_IF_FAILED(hr);

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

namespace Microsoft::Reunion::Sidecar
{
    static PCWSTR extensionsContainerName = L"Extensions";
    static WCHAR doubleQuoteChar = L'\"';
    static PCWSTR initializationFlag = L"--Init";

    HRESULT EnsureInitialized(_In_ PCWSTR instanceString);
    HRESULT IsContractRegisteredByManifest(_In_ PCWSTR contractName, _Inout_ bool& registered);
    bool IsInitializationFlag(_In_ PCWSTR argument);
    HRESULT GetLongValueByName(_In_ PCWSTR valueName, _Inout_ long& value);
    std::vector<std::wstring> GetArgsVector(std::wstring args);
    HRESULT GetPackageFullName(_Out_ PWSTR* fullName);
    HRESULT GetPackageInstalledPath(_Out_ PWSTR* installedPath);

    HRESULT InitializeExtensionContainer(_In_ winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer);
    HRESULT ParseSidecarManifest(
        _In_ PCWSTR packageInstallPath,
        _In_ winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer,
        _In_ winrt::Windows::Storage::ApplicationDataContainer extensionsContainer);
    HRESULT ParseExtensionNode(
        _In_ ComPtr<IXMLDOMNode> extensionNode,
        _In_ winrt::Windows::Storage::ApplicationDataContainer extensionsContainer);
    HRESULT ParseApplicationNode(
        _In_ ComPtr<IXMLDOMNode> applicationNode,
        _In_ winrt::Windows::Storage::ApplicationDataContainer extensionsContainer);
    HRESULT CreateSubContainerIfNeeded(
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer,
        _In_ PCWSTR subContainerName,
        _Inout_ winrt::Windows::Storage::ApplicationDataContainer& subContainer);
    HRESULT CreateStringValue(
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer,
        _In_ PCWSTR valueName,
        _In_ PCWSTR valueData,
        _In_ bool overWrite);
    HRESULT ProcessAttributeOfNode(
        _In_ ComPtr<IXMLDOMNode> thisNode,
        _In_ PCWSTR attributeName,
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer,
        _Inout_ winrt::Windows::Storage::ApplicationDataContainer& subContainer);
    HRESULT WriteInitializedMarker(
        _In_ PCWSTR markerName,
        _Inout_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*>& settingsCollection);
    HRESULT ParsePropertyNode(
        _In_ ComPtr<IXMLDOMNode> propertyNode,
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer);
    HRESULT ProcessNodeAndGetGrandChildrenNodes(
        _In_ ComPtr<IXMLDOMNode> currentNode,
        _In_ PCWSTR attributeName,
        _In_ PCWSTR childNodeXPath,
        _In_ PCWSTR grandChildrenNodesXPath,
        _In_ winrt::Windows::Storage::ApplicationDataContainer thisContainer,
        _Inout_ ComPtr<IXMLDOMNodeList>& nodesList,
        _Inout_ winrt::Windows::Storage::ApplicationDataContainer& subContainer);
    HRESULT PersistExecutablePath(_In_ PCWSTR executablePath);
    HRESULT GetLongValueByNameInCollection(
        _In_ PCWSTR valueName,
        _In_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*> settingsCollection,
        _Inout_ long& value);
    HRESULT ProcessLifetimeManagerCLSIDIfNeeded(_In_ ComPtr<IXMLDOMNode> extensionNode);
    HRESULT GetStringValueByNameInCollection(
        _In_ PCWSTR valueName,
        _In_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*> settingsCollection,
        _Inout_ wil::unique_process_heap_string & value);

    static winrt::Windows::Storage::IApplicationData applicationData{};
    static BasicLogFile* localLogFile = nullptr;
    static PCWSTR SYSTEM_APP_DATA_KEY_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\SystemAppData";
    static PCWSTR LOCAL_SETTINGS_KEY_PATH = L"SOFTWARE\\Classes\\Local Settings";
    static PCWSTR CLIENT_EXECUTABLE_PATH_VALUE_NAME = L"ClientExecutablePath";
    static PCWSTR lifetimeManagerClsidValueName = L"LifetimeManagerClsid";
    static wil::unique_process_heap_string lifetimeManagerClsid;

    static bool lifetimeManagerClsidFound = false;

    void LogDebugMessage(_In_ PCWSTR format, ...)
    {
        va_list arglist;
        va_start(arglist, format);

        WCHAR message[MAX_PATH];
        const HRESULT hr = StringCchVPrintf(message, ARRAYSIZE(message), format, arglist);

        // We don't care if the message is truncated, ignore "insufficient buffer" error
        if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER))
        {
            // There is error in string formatting. Just give generic message.
            (void)LOG_IF_FAILED(StringCchCopy(message, ARRAYSIZE(message), L"Error happened\n"));
        }
        if (localLogFile != nullptr)
        {
            (void)LOG_IF_FAILED_MSG(localLogFile->WriteMsg(message), "WriteMsg %ls", message);
        }
        OutputDebugString(message);
        return;
    }

    HRESULT EnsureLogFileInitialized(_In_ PCWSTR instanceString)
    {
        LOG_MESSAGE(L"Log file status: %u.", static_cast<UINT>(localLogFile != nullptr));
        RETURN_HR_IF_EXPECTED(S_OK, localLogFile != nullptr);

        PCWSTR myPath = applicationData.TemporaryFolder().Path().c_str();

        localLogFile = new BasicLogFile();
        RETURN_IF_NULL_ALLOC(localLogFile);

        const HRESULT hrOpen = localLogFile->OpenLogFileAlways(myPath, instanceString);
        if (FAILED_LOG(hrOpen))
        {
            delete localLogFile;
            localLogFile = nullptr;
        }
        return S_OK;
    }

    HRESULT GetPackageFullName(_Out_ PWSTR* fullName)
    {
        UINT32 length = 0;
        LONG rc = GetCurrentPackageFullName(&length, nullptr);
        RETURN_HR_IF(HRESULT_FROM_WIN32(APPMODEL_ERROR_NO_PACKAGE), rc == APPMODEL_ERROR_NO_PACKAGE);
        RETURN_HR_IF(E_UNEXPECTED, rc == ERROR_SUCCESS);
        RETURN_HR_IF(HRESULT_FROM_WIN32(rc), rc != ERROR_INSUFFICIENT_BUFFER);

        *fullName = (PWSTR)malloc(length * sizeof(**fullName));
        RETURN_IF_NULL_ALLOC(*fullName);

        rc = GetCurrentPackageFullName(&length, *fullName);
        if (rc != ERROR_SUCCESS)
        {
            free(*fullName);
            *fullName = nullptr;
            RETURN_WIN32(rc);
        }
        LOG_MESSAGE(L"PackageFullName %ls.", *fullName);
        return S_OK;
    }

    HRESULT GetPackageInstalledPath(_Out_ PWSTR* installedPath)
    {
        PWSTR fullName;
        LOG_AND_RETURN_IF_FAILED(GetPackageFullName(&fullName), L"GetPackageFullName");
        RETURN_IF_NULL_ALLOC(fullName);

        auto unloadUserProfile = wil::scope_exit([&]()
            {
                free(fullName);
            });

        UINT32 length = 0;
        LONG rc = GetPackagePathByFullName(fullName, &length, nullptr);
        RETURN_HR_IF(HRESULT_FROM_WIN32(APPMODEL_ERROR_NO_PACKAGE), rc == APPMODEL_ERROR_NO_PACKAGE);
        RETURN_HR_IF(E_UNEXPECTED, rc == ERROR_SUCCESS);
        RETURN_HR_IF(HRESULT_FROM_WIN32(rc), rc != ERROR_INSUFFICIENT_BUFFER);

        *installedPath = (PWSTR)malloc(length * sizeof(**installedPath));
        RETURN_IF_NULL_ALLOC(*installedPath);

        rc = GetPackagePathByFullName(fullName, &length, *installedPath);
        if (rc != ERROR_SUCCESS)
        {
            free(*installedPath);
            *installedPath = nullptr;
            LOG_MESSAGE(L"GetPackagePathByFullName %u failed, %lu.", length, rc);
            RETURN_WIN32(rc);
        }
        return S_OK;
    }

    HRESULT EnsureAppDataOpened()
    {
        LOG_MESSAGE(L"applicationData status: %u.", static_cast<UINT>(applicationData != nullptr));
        RETURN_HR_IF_EXPECTED(S_OK, applicationData != nullptr);

        applicationData = winrt::Windows::Storage::ApplicationData::Current();
        return S_OK;
    }

    HRESULT IsContractRegisteredForApp(
        _In_ PCWSTR contractName,
        _In_ winrt::Windows::Storage::ApplicationDataContainer appContainer,
        _Inout_ bool& registered)    
    {
        auto extensionContainers = appContainer.Containers();

        // Is this search case sensitive??
        registered = extensionContainers.HasKey(winrt::hstring(contractName));
        LOG_MESSAGE(L"Contract %ls registered: %u.", contractName, static_cast<UINT>(registered));
        return S_OK;
    }

    // Returns true if the extension contract is registered via appxmanifest.xml or an associated manifest.
    // TODO: perf improvement by caching some pointers.
    HRESULT IsContractRegisteredByManifest(_In_ PCWSTR contractName, _Inout_ bool& registered)
    {
        registered = false;

        winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer = applicationData.LocalSettings();
        RETURN_IF_NULL_ALLOC(localSettingsContainer);

        auto subContainers = localSettingsContainer.Containers();
        RETURN_IF_NULL_ALLOC(subContainers);

        auto extensionsContainer = subContainers.Lookup(winrt::hstring(extensionsContainerName));

        auto applicationContainers = extensionsContainer.Containers();
        RETURN_IF_NULL_ALLOC(applicationContainers);

        const unsigned int containersCount = applicationContainers.Size();
        RETURN_HR_IF_EXPECTED(S_OK, containersCount == 0);

        LOG_MESSAGE(L"%u subcontainers under %ls.", containersCount, extensionsContainerName);

        winrt::Windows::Foundation::Collections::IIterable<
            winrt::Windows::Foundation::Collections::IKeyValuePair<winrt::hstring, winrt::Windows::Storage::ApplicationDataContainer*>*>
            iterableSubContainer =
            applicationContainers.as<winrt::Windows::Foundation::Collections::IIterable<
            winrt::Windows::Foundation::Collections::IKeyValuePair<winrt::hstring, winrt::Windows::Storage::ApplicationDataContainer*>*>>();
        RETURN_IF_NULL_ALLOC(iterableSubContainer);

        auto subContainerIterator = iterableSubContainer.First();
        RETURN_IF_NULL_ALLOC(subContainerIterator);

        bool hasCurrent = subContainerIterator.HasCurrent();
        while (hasCurrent)
        {
            auto spPair = subContainerIterator.Current();
            RETURN_IF_NULL_ALLOC(spPair);

            winrt::hstring strItemKey = spPair->Key();
            LOG_MESSAGE(L"Looking under app %ls.", strItemKey.c_str());

            auto appContainer = spPair->Value();
            LOG_AND_RETURN_IF_FAILED(IsContractRegisteredForApp(contractName, *appContainer, registered), L"IsContractRegisteredForApp");

            RETURN_HR_IF_EXPECTED(S_OK, registered);

            hasCurrent = subContainerIterator.MoveNext();
        }
        return S_OK;
    }

    // Create a new sub-container with the prescribed name if it's not found; otherwise, return the existing sub-container.
    HRESULT CreateSubContainerIfNeeded(
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer,
        _In_ PCWSTR subContainerName,
        _Inout_ winrt::Windows::Storage::ApplicationDataContainer& subContainer)
    {
        auto subContainers = parentContainer.Containers();
        RETURN_IF_NULL_ALLOC(subContainers);

        const winrt::hstring subContainerNameString = subContainerName;
        const boolean containerFound = subContainers.HasKey(subContainerNameString);

        if (!containerFound)
        {
            subContainer = parentContainer.CreateContainer(subContainerNameString,
                winrt::Windows::Storage::ApplicationDataCreateDisposition::Always);
        }
        else
        {
            subContainer = subContainers.Lookup(subContainerNameString);
        }

        LOG_MESSAGE(L"Settings subcontainer %ls existence %u.", subContainerName, static_cast<UINT>(containerFound));
        RETURN_IF_NULL_ALLOC(subContainer);

        return S_OK;
    }

    // Create string type value with valueName and data valueData under parentContainer in Settings.
    HRESULT CreateStringValue(
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer,
        _In_ PCWSTR valueName,
        _In_ PCWSTR valueData,
        _In_ bool overWrite)
    {
        auto propertySet = parentContainer.Values();
        RETURN_IF_NULL_ALLOC(propertySet);

        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*> values =
            propertySet.as<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*>>();

        const winrt::hstring valueNameString = valueName;
        const boolean valueFound = values.HasKey(valueNameString);

        // Return from here if the value already existed.
        if (valueFound && !overWrite)
        {
            LOG_MESSAGE(L"Value %ls already exists?!", valueName);
            return S_OK;
        }

        // Create the prescribed value.
        auto newValue = winrt::Windows::Foundation::PropertyValue::CreateString(winrt::hstring(valueData));
        RETURN_IF_NULL_ALLOC(newValue);

        const boolean replaced = values.Insert(valueNameString, &newValue);
        LOG_MESSAGE(L"Inserted %ls with %ls, replaced: %u.", valueName, valueData, static_cast<UINT>(replaced));
        return S_OK;
    }

    // Load the sidecar manifest from under packageInstallPath into IXMLDOMDocument.
    HRESULT LoadSidecarManifest(_In_ PCWSTR packageInstallPath, _Inout_ ComPtr<IXMLDOMDocument>& xmlDoc)
    {
        static const WCHAR sidecarManifestName[] = L"PIPAppxManifest.xml";

        WCHAR sidecarManifestPath[MAX_PATH];
        LOG_AND_RETURN_IF_FAILED(StringCchPrintf(sidecarManifestPath, ARRAYSIZE(sidecarManifestPath), L"%ls\\%ls", packageInstallPath,
            sidecarManifestName), L"StringCchPrintf");

        LOG_AND_RETURN_IF_FAILED(CoCreateInstance(CLSID_DOMDocument, nullptr, CLSCTX_ALL, __uuidof(IXMLDOMDocument), (LPVOID*)&xmlDoc),
            L"CoCreateInstance");
        RETURN_IF_NULL_ALLOC(xmlDoc.Get());

        LOG_AND_RETURN_IF_FAILED(xmlDoc.Get()->put_async(VARIANT_FALSE), L"put_async");
        LOG_AND_RETURN_IF_FAILED(xmlDoc.Get()->put_validateOnParse(VARIANT_FALSE), L"put_validateOnParse");
        LOG_AND_RETURN_IF_FAILED(xmlDoc.Get()->put_resolveExternals(VARIANT_FALSE), L"put_resolveExternals");
        LOG_AND_RETURN_IF_FAILED(xmlDoc.Get()->put_preserveWhiteSpace(VARIANT_FALSE), L"put_preserveWhiteSpace");

        wil::unique_variant filePath;
        filePath.vt = VT_BSTR;
        filePath.bstrVal = wil::make_bstr(sidecarManifestPath).release();

        VARIANT_BOOL isSuccessful = VARIANT_FALSE;
        const HRESULT hrLoad = LOG_IF_FAILED_MSG(xmlDoc.Get()->load(filePath, &isSuccessful), "Load %ws", sidecarManifestPath);
        LOG_AND_RETURN_IF_FAILED(hrLoad, L"load");

        if (isSuccessful == VARIANT_TRUE)
        {
            // Return from here if XML loading was successful.
            LOG_MESSAGE(L"Loaded manifest %ls.", filePath.bstrVal);
            return S_OK;
        }

        ComPtr<IXMLDOMParseError> xmlParseError;
        xmlDoc.Get()->get_parseError(&xmlParseError);

        long xmlLine = 0;
        xmlParseError.Get()->get_line(&xmlLine);

        long xmlLinePos = 0;
        xmlParseError.Get()->get_linepos(&xmlLinePos);

        wil::unique_bstr errorReason;
        xmlParseError.Get()->get_reason(&errorReason);

        LOG_MESSAGE(L"Error parsing %ls, Line: %d, Col: %d, Reason: %ls", sidecarManifestPath, xmlLine, xmlLinePos,
            errorReason.get());
        RETURN_HR_MSG(E_FAIL, "load %ws, Line: %d, Col: %d, Reason: %ws", sidecarManifestPath, xmlLine, xmlLinePos,
            errorReason.get());
    }

    // Extract thisNode's attribute attributeName and persist it as a parentContainer under parentContainer.
    HRESULT ProcessAttributeOfNode(
        _In_ ComPtr<IXMLDOMNode> thisNode, 
        _In_ PCWSTR attributeName,
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer,
        _Inout_ winrt::Windows::Storage::ApplicationDataContainer& subContainer)
        //       _Inout_ ComPtr<winrt::Windows::Storage::IApplicationDataContainer& subContainer)
    {
        ComPtr<IXMLDOMElement> thisElement;
        RETURN_IF_FAILED(thisNode.As(&thisElement));
        RETURN_IF_NULL_ALLOC(thisElement.Get());

        wil::unique_bstr queryAttribute = wil::make_bstr_nothrow(attributeName);
        RETURN_IF_NULL_ALLOC_MSG(queryAttribute.get(), "make_bstr_nothrow %ws", attributeName);

        wil::unique_variant thisAttribute;
        LOG_AND_RETURN_IF_FAILED(thisElement.Get()->getAttribute(queryAttribute.get(), &thisAttribute), L"getAttribute");
        RETURN_HR_IF(E_INVALIDARG, (thisAttribute.vt != VT_BSTR) || (thisAttribute.bstrVal == nullptr));

        LOG_MESSAGE(L"Found attribute %ls.", thisAttribute.bstrVal);
        LOG_AND_RETURN_IF_FAILED(CreateSubContainerIfNeeded(parentContainer, thisAttribute.bstrVal, subContainer),
            L"CreateSubContainerIfNeeded");
        return S_OK;
    }

    HRESULT ParsePropertyNode(
        _In_ ComPtr<IXMLDOMNode> propertyNode, 
        _In_ winrt::Windows::Storage::ApplicationDataContainer parentContainer)
    {
        wil::unique_bstr nodeName;
        LOG_AND_RETURN_IF_FAILED(propertyNode->get_nodeName(&nodeName), L"get_nodeName");

        wil::unique_bstr nodeText;
        LOG_AND_RETURN_IF_FAILED(propertyNode->get_text(&nodeText), L"get_text");

        // Persist the property as a Settings value.
        const HRESULT hrCreate = CreateStringValue(parentContainer, nodeName.get(), nodeText.get(), false);
        LOG_MESSAGE(L"Processed property %ls with data %ls, 0x%0x.", nodeName.get(), nodeText.get(), hrCreate);
        RETURN_IF_FAILED(hrCreate);

        return S_OK;
    }

    HRESULT ProcessNodeAndGetGrandChildrenNodes(
        _In_ ComPtr<IXMLDOMNode> currentNode,
        _In_ PCWSTR attributeName,
        _In_ PCWSTR childNodeXPath,
        _In_ PCWSTR grandChildrenNodesXPath,
        _In_ winrt::Windows::Storage::ApplicationDataContainer thisContainer,
        _Inout_ ComPtr<IXMLDOMNodeList>& nodesList,
        _Inout_ winrt::Windows::Storage::ApplicationDataContainer& subContainer)
    {
        // Extract the current node's prescribed attribute and persist it as a Settings container.
        LOG_AND_RETURN_IF_FAILED(ProcessAttributeOfNode(currentNode, attributeName, thisContainer, subContainer),
            L"ProcessAttributeOfNode");
            
        // Check if the prescribe child node is present under the current node.
        wil::unique_bstr childNodeQuery = wil::make_bstr_nothrow(childNodeXPath);
        RETURN_IF_NULL_ALLOC_MSG(childNodeQuery.get(), "make_bstr_nothrow %ws", childNodeXPath);

        ComPtr<IXMLDOMNode> childNode;
        HRESULT hrSelect = currentNode.Get()->selectSingleNode(childNodeQuery.get(), &childNode);
        RETURN_IF_NOT_FOUND_OR_ERROR(hrSelect, childNode.Get());

        LOG_MESSAGE(L"Query %ls found results.", childNodeXPath);    

        // Enumerate the grand children nodes under the child node.
        wil::unique_bstr grandChildrenNodesQuery = wil::make_bstr_nothrow(grandChildrenNodesXPath);
        RETURN_IF_NULL_ALLOC_MSG(grandChildrenNodesQuery.get(), "make_bstr_nothrow %ws", grandChildrenNodesXPath);

        hrSelect = childNode.Get()->selectNodes(grandChildrenNodesQuery.get(), &nodesList);
        RETURN_IF_NOT_FOUND_OR_ERROR(hrSelect, nodesList.Get());

        LOG_MESSAGE(L"Query %ls found results.", grandChildrenNodesXPath);    
        return S_OK;
    }

    HRESULT ProcessLifetimeManagerCLSIDIfNeeded(_In_ ComPtr<IXMLDOMNode> extensionNode)
    {
        static const WCHAR categoryAttributeName[] = L"Category";
        static const WCHAR executableAttributeName[] = L"Executable";
        static const WCHAR idAttributeName[] = L"Id";
        static const WCHAR comServerCategoryValue[] = L"windows.comServer";
        static const WCHAR exeServerExecutableValue[] = L"SidecarCentennialApp.exe";
        static const WCHAR exeServerElementXPath[] =
            L"./*[local-name()='ComServer']/*[local-name()='ExeServer']";
        static const WCHAR classElementXPath[] =
            L"./*[local-name()='Class']";

        // TODO: Currently assume there is a single instance of the CLSID in the manifest.
        LOG_MESSAGE(L"ProcessLifetimeManagerCLSIDIfNeeded %u %p.", static_cast<UINT>(lifetimeManagerClsidFound),
            lifetimeManagerClsid.get());
        RETURN_HR_IF_EXPECTED(S_OK, lifetimeManagerClsidFound);

        // Is the extension of Category "windows.comServer"?
        ComPtr<IXMLDOMElement> thisElement;
        RETURN_IF_FAILED(extensionNode.As(&thisElement));
        RETURN_IF_NULL_ALLOC(thisElement.Get());

        wil::unique_bstr queryCategoryAttribute = wil::make_bstr_nothrow(categoryAttributeName);
        RETURN_IF_NULL_ALLOC_MSG(queryCategoryAttribute.get(), "make_bstr_nothrow %ws", categoryAttributeName);

        wil::unique_variant thisAttribute;
        LOG_AND_RETURN_IF_FAILED(thisElement.Get()->getAttribute(queryCategoryAttribute.get(), &thisAttribute),
            L"getAttribute category");
        RETURN_HR_IF(E_INVALIDARG, (thisAttribute.vt != VT_BSTR) || (thisAttribute.bstrVal == nullptr));

        LOG_MESSAGE(L"Found Category attribute %ls.", thisAttribute.bstrVal);

        RETURN_HR_IF_EXPECTED(S_OK,
            CompareStringOrdinal(thisAttribute.bstrVal, -1, comServerCategoryValue, -1, TRUE) != CSTR_EQUAL);

        // This is a "windows.comServer" extension. Try to get at the ComServer\ExeServer element under it.
        wil::unique_bstr exeServerNodeQuery = wil::make_bstr_nothrow(exeServerElementXPath);
        RETURN_IF_NULL_ALLOC_MSG(exeServerNodeQuery.get(), "make_bstr_nothrow %ws", exeServerElementXPath);

        // TODO: Today only a single node is expected. In the future when more nodes become possible we'll need to
        // somehow filter out other nodes.
        ComPtr<IXMLDOMNode> exeServerNode;
        HRESULT hrSelect = extensionNode.Get()->selectSingleNode(exeServerNodeQuery.get(), &exeServerNode);
        RETURN_IF_NOT_FOUND_OR_ERROR(hrSelect, exeServerNode.Get());

        // We got to the ExeServer element. Is it associated with the Centennnial main app?
        RETURN_IF_FAILED(exeServerNode.As(&thisElement));

        wil::unique_bstr queryExecutableAttribute = wil::make_bstr_nothrow(executableAttributeName);
        RETURN_IF_NULL_ALLOC_MSG(queryExecutableAttribute.get(), "make_bstr_nothrow %ws", executableAttributeName);

        LOG_AND_RETURN_IF_FAILED(thisElement.Get()->getAttribute(queryExecutableAttribute.get(), &thisAttribute),
            L"getAttribute ExeServer");

        RETURN_HR_IF(E_INVALIDARG, (thisAttribute.vt != VT_BSTR) || (thisAttribute.bstrVal == nullptr));

        LOG_MESSAGE(L"Found Executable attribute %ls.", thisAttribute.bstrVal);

        RETURN_HR_IF_EXPECTED(S_OK,
            CompareStringOrdinal(thisAttribute.bstrVal, -1, exeServerExecutableValue, -1, TRUE) != CSTR_EQUAL);

        // This ExeServer element is associated with the Centennnial main app. Extract the CLSID.
        wil::unique_bstr classNodeQuery = wil::make_bstr_nothrow(classElementXPath);
        RETURN_IF_NULL_ALLOC_MSG(classNodeQuery.get(), "make_bstr_nothrow %ws", classElementXPath);

        // TODO: Today only a single node is expected. In the future when more nodes become possible we'll need to
        // somehow filter out other nodes.
        ComPtr<IXMLDOMNode> classNode;
        hrSelect = exeServerNode.Get()->selectSingleNode(classNodeQuery.get(), &classNode);
        RETURN_IF_NOT_FOUND_OR_ERROR(hrSelect, classNode.Get());

        // Found a Class node under "windows.comServer", try to extract its Id attribute.
        RETURN_IF_FAILED(classNode.As(&thisElement));

        wil::unique_bstr queryIdAttribute = wil::make_bstr_nothrow(idAttributeName);
        RETURN_IF_NULL_ALLOC_MSG(queryIdAttribute.get(), "make_bstr_nothrow %ws", idAttributeName);

        LOG_AND_RETURN_IF_FAILED(thisElement.Get()->getAttribute(queryIdAttribute.get(), &thisAttribute),
            L"getAttribute Id");

        RETURN_HR_IF(E_INVALIDARG, (thisAttribute.vt != VT_BSTR) || (thisAttribute.bstrVal == nullptr));

        LOG_MESSAGE(L"Found Id attribute %ls.", thisAttribute.bstrVal);

        lifetimeManagerClsid = wil::make_process_heap_string_nothrow(thisAttribute.bstrVal);
        RETURN_IF_NULL_ALLOC_MSG(lifetimeManagerClsid, "make_process_heap_string_nothrow: %ws", thisAttribute.bstrVal);

        // Not persisting the CLSID into AppData at this point just yet, but sets a flag instead, because at this point
        // in the call stack we don't have direct access to the top level LocalSettings container under which we want
        // to persist the CLSID.
        lifetimeManagerClsidFound = true;
        return S_OK;
    }

    HRESULT ParseExtensionNode(
        _In_ ComPtr<IXMLDOMNode> extensionNode, 
        _In_ winrt::Windows::Storage::ApplicationDataContainer thisApplicationContainer)
    {
        static const WCHAR propertiesElementXPath[] = L"./*[local-name()='Properties']";
        static const WCHAR propertyElementsXPath[] = L"./*";
        static const WCHAR categoryAttributeName[] = L"Category";

        LOG_AND_RETURN_IF_FAILED(ProcessLifetimeManagerCLSIDIfNeeded(extensionNode), L"ProcessLifetimeManagerCLSIDIfNeeded");

        // Enumerate the Property elements under the Properties element, which is in turn under the extension node.
        winrt::Windows::Storage::ApplicationDataContainer thisExtensionContainer = nullptr;
        ComPtr<IXMLDOMNodeList> propertyNodesList;
        const HRESULT hrSelect = ProcessNodeAndGetGrandChildrenNodes(extensionNode,
            categoryAttributeName,
            propertiesElementXPath,
            propertyElementsXPath,
            thisApplicationContainer,
            propertyNodesList,
            thisExtensionContainer);
        RETURN_IF_NOT_FOUND_OR_ERROR(hrSelect, propertyNodesList.Get());

        long nodesCount = 0;
        LOG_AND_RETURN_IF_FAILED(propertyNodesList.Get()->get_length(&nodesCount), L"get_length");

        LOG_MESSAGE(L"Found %d property elements in manifest.", nodesCount);

        for (int index = nodesCount - 1; index >= 0; index--)
        {
            ComPtr<IXMLDOMNode> propertyNode;
            LOG_AND_RETURN_IF_FAILED(propertyNodesList.Get()->get_item(index, &propertyNode), L"get_item");
            LOG_AND_RETURN_IF_FAILED(ParsePropertyNode(propertyNode, thisExtensionContainer), L"ParsePropertyNode");
        }
        return S_OK;
    }

    HRESULT ParseApplicationNode(
        _In_ ComPtr<IXMLDOMNode> applicationNode, 
        _In_ winrt::Windows::Storage::ApplicationDataContainer extensionsContainer)
    {
        static const WCHAR extensionsElementXPath[] = L"./*[local-name()='Extensions']";
        static const WCHAR extensionElementsXPath[] = L"./*[local-name()='Extension']";
        static const WCHAR idAttributeName[] = L"Id";

        // Enumerate the Extension elements under the Extensions element, which is in turn under the application node.
        winrt::Windows::Storage::ApplicationDataContainer thisApplicationContainer = nullptr;
        ComPtr<IXMLDOMNodeList> extensionNodesList;        
        const HRESULT hrSelect = ProcessNodeAndGetGrandChildrenNodes(applicationNode,
            idAttributeName, 
            extensionsElementXPath, 
            extensionElementsXPath, 
            extensionsContainer, 
            extensionNodesList, 
            thisApplicationContainer);
        RETURN_IF_NOT_FOUND_OR_ERROR(hrSelect, extensionNodesList.Get());

        long nodesCount = 0;
        LOG_AND_RETURN_IF_FAILED(extensionNodesList.Get()->get_length(&nodesCount), L"get_length");

        LOG_MESSAGE(L"Found %d extension elements in manifest.", nodesCount);        

        for (int index = nodesCount - 1; index >= 0; index--)
        {
            ComPtr<IXMLDOMNode> extensionNode;
            LOG_AND_RETURN_IF_FAILED(extensionNodesList.Get()->get_item(index, &extensionNode), L"get_item");
            LOG_AND_RETURN_IF_FAILED(ParseExtensionNode(extensionNode, thisApplicationContainer), L"ParseExtensionNode");
        }
        return S_OK;
    }

    HRESULT ParseSidecarManifest(
        _In_ PCWSTR packageInstallPath,
        _In_ winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer,
        _In_ winrt::Windows::Storage::ApplicationDataContainer extensionsContainer)
    {
        static const WCHAR applicationElementXPath[] = 
            L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']";
        static const WCHAR packageRootValueName[] = L"PackageRoot";

        LOG_AND_RETURN_IF_FAILED(CreateStringValue(localSettingsContainer, packageRootValueName, packageInstallPath, false),
            L"CreateStringValue");

        ComPtr<IXMLDOMDocument> xmlDoc;
        LOG_AND_RETURN_IF_FAILED(LoadSidecarManifest(packageInstallPath, xmlDoc), L"LoadSidecarManifest");
        
        // Expect to find at least one Aplication element.
        wil::unique_bstr applicationElementsQuery = wil::make_bstr_nothrow(applicationElementXPath);
        RETURN_IF_NULL_ALLOC_MSG(applicationElementsQuery.get(), "make_bstr_nothrow %ws", applicationElementXPath);

        ComPtr<IXMLDOMNodeList> applicationNodesList;  
        LOG_AND_RETURN_IF_FAILED(xmlDoc.Get()->selectNodes(applicationElementsQuery.get(), &applicationNodesList), L"selectNodes");
        RETURN_IF_NULL_ALLOC(applicationNodesList.Get());

        long nodesCount = 0;
        LOG_AND_RETURN_IF_FAILED(applicationNodesList.Get()->get_length(&nodesCount), L"get_length");
        RETURN_HR_IF(E_INVALIDARG, nodesCount < 1);

        LOG_MESSAGE(L"Found %d apps in manifest.", nodesCount);        

        // Enumerate application elements.
        for (int index = nodesCount - 1; index >= 0; index--)
        {
            ComPtr<IXMLDOMNode> applicationNode;
            LOG_AND_RETURN_IF_FAILED(applicationNodesList.Get()->get_item(index, &applicationNode), L"get_item");
            LOG_AND_RETURN_IF_FAILED(ParseApplicationNode(applicationNode, extensionsContainer), L"ParseApplicationNode");
        }

        LOG_AND_RETURN_IF_FAILED(lifetimeManagerClsidFound ? S_OK : HRESULT_FROM_WIN32(ERROR_NOT_FOUND),
            L"lifetimeManagerClsid NOT FOUND");
        LOG_AND_RETURN_IF_FAILED(CreateStringValue(localSettingsContainer, lifetimeManagerClsidValueName,
            lifetimeManagerClsid.get(), false), L"CreateStringValue");
        return S_OK;
    }

    // Get Extensions container under LocalSettings. If it does not exist, then create one. The hierarchy looks like:
    // LocalState/
    //     Initialized	        DWORD	0x1	
    //     PackageRoot          String  ...
    //     LifetimeManagerClsid String ...
    //     <...Other Values...>
    //     Extensions/
    //         <ApplicationId1>/
    //             ExecutablePath String ...
    //             BackgroundTask/
    //             SharedTarget/
    //                 Prop1	String	...
    //                 prop2	String	...
    //         <ApplicationId2>/
    //             ...
    // RoamingState/
    //
    // The "/" suffix denotes containers.
    // If Prop1 contains the relative path to the win32 app's executable, then the full path to the win32 app's executable
    // can be constructed by concatenating PackageRoot and Prop1, with a '\' as delimiter.
    //
    // Although this Sidecar main app (SidecarMainApp) does not seem to have a tangible need to see the extension contracts
    // declared in the Sidecar manifest for other apps in the package, but because AppData of the package is shared by all 
    // apps in the package, in case other apps also want to see the extension contracts declared for their own apps, we 
    // might as well just centrally parse extensions for all apps in the manifest into AppData in one shot.
    HRESULT InitializeExtensionContainer(_In_ winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer)
    {
        winrt::Windows::Storage::ApplicationDataContainer extensionsContainer = nullptr;
        LOG_AND_RETURN_IF_FAILED(CreateSubContainerIfNeeded(localSettingsContainer, extensionsContainerName, extensionsContainer),
            L"CreateSubContainerIfNeeded");

        PWSTR packageInstallPath = nullptr;
        LOG_AND_RETURN_IF_FAILED(GetPackageInstalledPath(&packageInstallPath), L"GetPackageInstalledPath");
        RETURN_IF_NULL_ALLOC(packageInstallPath);

        LOG_MESSAGE(L"PackageInstallPath %ls.", packageInstallPath);

        const HRESULT hrParse = ParseSidecarManifest(packageInstallPath, localSettingsContainer, extensionsContainer);
        free(packageInstallPath);
        LOG_AND_RETURN_IF_FAILED(hrParse, L"ParseSidecarManifest");
        return S_OK;
    }

    // Create the "initialized" boolean Settings value.
    HRESULT WriteInitializedMarker(
        _In_ PCWSTR markerName,
        _Inout_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*>& settingsCollection)
    {
        auto booleanValue = winrt::Windows::Foundation::PropertyValue::CreateBoolean(true);
        RETURN_IF_NULL_ALLOC(booleanValue);

        const boolean replaced = settingsCollection.Insert(winrt::hstring(markerName), &booleanValue);
        LOG_MESSAGE(L"Marker %ls written, replaced: %u.", markerName, static_cast<UINT>(replaced));
        return S_OK;
    }

    // If a boolean "initialized" flag has been set, then skip initialization; otherwise, trigger full initialization.
    HRESULT EnsureAppDataPopulated(/* _Inout_ long& numOfTrials */)
    {
        static const WCHAR initializedValueName[] = L"Initialized";

        // Check if the "initialized" value exists directly under the LocalSettings container.
        winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer = applicationData.LocalSettings();
        RETURN_IF_NULL_ALLOC(localSettingsContainer);

        auto propertySet = localSettingsContainer.Values();
        RETURN_IF_NULL_ALLOC(propertySet);

        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*> settingsCollection =
            propertySet.as<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*>>();

        const boolean keyExists = settingsCollection.HasKey(winrt::hstring(initializedValueName));

        LOG_MESSAGE(L"Presence of Initialized marker %u", static_cast<UINT>(keyExists));
        if (keyExists)
        {
            // Retrieve the Lifetime Manager CLSID from AppData into runtime variable. We might reach here more than once,
            // but that's ok.
            LOG_AND_RETURN_IF_FAILED(GetStringValueByNameInCollection(lifetimeManagerClsidValueName, settingsCollection,
                lifetimeManagerClsid), L"GetStringValueByNameInCollection");

            // Return from here if the key already exists, AppData presumbly has been populated.
            return S_OK;
        }

        // Trigger initialization.
        LOG_AND_RETURN_IF_FAILED(InitializeExtensionContainer(localSettingsContainer), L"InitializeExtensionContainer");
        LOG_AND_RETURN_IF_FAILED(WriteInitializedMarker(initializedValueName, settingsCollection), L"WriteInitializedMarker");
        return S_OK;
    }

    HRESULT EnsureInitialized(/* _Inout_ long& numOfTrials, */ _In_ PCWSTR instanceString)
    {
        LOG_AND_RETURN_IF_FAILED(EnsureAppDataOpened(), "EnsureAppDataOpened");
        (void)LOG_IF_FAILED(EnsureLogFileInitialized(instanceString));
        LOG_AND_RETURN_IF_FAILED(EnsureAppDataPopulated(/* numOfTrials */ ), L"EnsureAppDataPopulated");
        return S_OK;
    }

    HRESULT UninitializeLogFile()
    {
        LOG_MESSAGE(L"Uninitializing log file.");        

        if (localLogFile != nullptr)
        {
            delete localLogFile;
            localLogFile = nullptr;
        }
        return S_OK;
    }

    bool IsInitializationFlag(_In_ PCWSTR argument)
    {
        return (CompareStringOrdinal(argument, -1, initializationFlag, -1, TRUE) == CSTR_EQUAL);
    }

    HRESULT GetLongValueByNameInCollection(
        _In_ PCWSTR valueName,
        _In_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*> settingsCollection,
        _Inout_ long& value)
    {
        auto propertyValue = settingsCollection.Lookup(winrt::hstring(valueName));
        RETURN_HR_IF_EXPECTED(HRESULT_FROM_WIN32(ERROR_NOT_FOUND), propertyValue == nullptr);

        winrt::Windows::Foundation::IPropertyValue int64PropertyValue = propertyValue->as<winrt::Windows::Foundation::IPropertyValue>();
        RETURN_IF_NULL_ALLOC(int64PropertyValue);

        const auto propertyType = int64PropertyValue.Type();
        LOG_AND_RETURN_IF_FAILED(propertyType == winrt::Windows::Foundation::PropertyType::Int64 ? S_OK : E_INVALIDARG,
            L"get_Type unexpected");

        const auto int64Value = int64PropertyValue.GetInt64();
        value = static_cast<long>(int64Value);

        LOG_MESSAGE(L"Read %ls %d from AppData Settings.", valueName, value);
        return S_OK;
    }

    HRESULT GetStringValueByNameInCollection(
        _In_ PCWSTR valueName,
        _In_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*> settingsCollection,
        _Inout_ wil::unique_process_heap_string& value)
    {
        auto propertyValue = settingsCollection.Lookup(winrt::hstring(valueName));
        RETURN_HR_IF_EXPECTED(HRESULT_FROM_WIN32(ERROR_NOT_FOUND), propertyValue == nullptr);

        winrt::Windows::Foundation::IPropertyValue stringPropertyValue = propertyValue->as<winrt::Windows::Foundation::IPropertyValue>();
        RETURN_IF_NULL_ALLOC(stringPropertyValue);

        const auto propertyType = stringPropertyValue.Type();
        LOG_AND_RETURN_IF_FAILED(propertyType == winrt::Windows::Foundation::PropertyType::String ? S_OK : E_INVALIDARG,
            L"get_Type unexpected");

        const auto localValue = stringPropertyValue.GetString();
        value = wil::make_process_heap_string_nothrow(localValue.c_str());
        RETURN_IF_NULL_ALLOC_MSG(value, "Path: %ws", localValue.c_str());

        LOG_MESSAGE(L"Read %ls %ls from AppData Settings.", valueName, localValue.c_str());
        return S_OK;
    }

    HRESULT GetLongValueByName(_In_ PCWSTR valueName, _Inout_ long& value)
    {
        winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer = applicationData.LocalSettings();
        RETURN_IF_NULL_ALLOC(localSettingsContainer);

        const auto propertySet = localSettingsContainer.Values();
        RETURN_IF_NULL_ALLOC(propertySet);

        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*>
            settingsCollection =
            propertySet.as<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable*>>();
        RETURN_IF_NULL_ALLOC(settingsCollection);

        LOG_AND_RETURN_IF_FAILED(GetLongValueByNameInCollection(valueName, settingsCollection, value),
            L"GetLongValueByNameInCollection");
        return S_OK;
    }

    // Persist executablePath in Settings under LocalState/Extensions/SidecarMainApp, as value ExecutablePath.
    // This path should already exist, so not swallowing "not found" errors.
    HRESULT PersistExecutablePath(_In_ PCWSTR executablePath)
    {
        static const WCHAR executablePathValueName[] = L"ExecutablePath";

        // TODO: When the app container main app exists in the PIP, switch to use "SidecarMainApp";
        static const WCHAR myApplicationId[] = L"SidecarCentennialApp";
        RETURN_HR_IF(E_INVALIDARG, (executablePath == nullptr) || (*executablePath == L'\0'));

        size_t stringLen = wcslen(executablePath);
        RETURN_HR_IF(E_INVALIDARG, stringLen < 2);        

        winrt::Windows::Storage::ApplicationDataContainer localSettingsContainer = applicationData.LocalSettings();
        RETURN_IF_NULL_ALLOC(localSettingsContainer);

        auto subContainers = localSettingsContainer.Containers();
        RETURN_IF_NULL_ALLOC(subContainers);

        winrt::Windows::Storage::ApplicationDataContainer extensionsContainer = subContainers.Lookup(
            winrt::hstring(extensionsContainerName));
        RETURN_IF_NULL_ALLOC(extensionsContainer);

        auto applicationContainers = extensionsContainer.Containers();

        winrt::Windows::Storage::ApplicationDataContainer applicationContainer =
            applicationContainers.Lookup(winrt::hstring(myApplicationId));
        RETURN_IF_NULL_ALLOC(applicationContainer);

        // Filter out any double quotes in the path, if any.
        WCHAR quotelessPath[MAX_PATH];
        UINT j = 0;
        for (UINT i = 0; i < stringLen; i++)
        {
            if (executablePath[i] != doubleQuoteChar)
            {
                quotelessPath[j++] = executablePath[i];
            }
        }
        quotelessPath[j] = L'\0';

        LOG_AND_RETURN_IF_FAILED(CreateStringValue(applicationContainer, executablePathValueName, quotelessPath, true),
            L"CreateStringValue");
        return S_OK;
    }

    // Supported flags:
    // --Init:          Just perform the initialization automatically performed at first launch of the Sidecar (regardless of 
    //                  the presence of the "--init" flag, that's all.
    HRESULT ProcessInitializationFlag(std::wstring args)
    {
        auto argsArray = GetArgsVector(args);
        LOG_MESSAGE(L"ProcessInitializationFlag, argsArray size=%u.", argsArray.size());

        // GetArgsVector() uses ' ' as delimiter. For paths that contain spaces, they need to be in double quotes. A path
        // that contains spaces will appear as more than one argument. Below we look for opening and closing double quotes
        // and stitch the arguments together into a path.
        for (UINT index = 0; index < argsArray.size(); index++)
        {
            LOG_MESSAGE(L"argsArray[%u] = %s.", index, argsArray[index].c_str());
            if (IsInitializationFlag(argsArray[index].c_str()))
            {
                UINT j = index + 1;
                if (j < argsArray.size())
                {
                    LOG_MESSAGE(L"argsArray[%u] = %s.", j, argsArray[j].c_str());

                    WCHAR pathBuffer[MAX_PATH];
                    LOG_AND_RETURN_IF_FAILED(StringCchPrintf(pathBuffer, ARRAYSIZE(pathBuffer), L"%ls", argsArray[j].c_str()),
                        L"StringCchPrintf");

                    if (pathBuffer[0] == doubleQuoteChar)
                    {
                        bool foundClosingQuote = false;
                        while (!foundClosingQuote && (++j < argsArray.size()))
                        {
                            LOG_MESSAGE(L"argsArray[%u] = %s, looking for closing quote.", j, argsArray[j].c_str());
                            LOG_AND_RETURN_IF_FAILED(StringCchCat(pathBuffer, ARRAYSIZE(pathBuffer), L" "),
                                L"StringCchCat blank");
                            LOG_AND_RETURN_IF_FAILED(StringCchCat(pathBuffer, ARRAYSIZE(pathBuffer), argsArray[j].c_str()),
                                L"StringCchCat");

                            size_t stringLen = 0;
                            LOG_AND_RETURN_IF_FAILED(StringCchLength(pathBuffer, ARRAYSIZE(pathBuffer), &stringLen),
                                L"StringCchLength");

                            foundClosingQuote = (pathBuffer[stringLen - 1] == doubleQuoteChar);
                        }

                        if (!foundClosingQuote)
                        {
                            LOG_MESSAGE(L"Missing closing double quote for path string %ls??", pathBuffer);
                            RETURN_HR(E_INVALIDARG);
                        }
                    }

                    index = j;
                    LOG_AND_RETURN_IF_FAILED(PersistExecutablePath(pathBuffer), L"PersistExecutablePath");
                }
                else
                {
                    LOG_MESSAGE(L"Missing executable path @index %u??", j);
                }
            }
            else
            {
                LOG_MESSAGE(L"Unknown argument %s.", argsArray[index].c_str());
            }
        }
        return S_OK;
    }

    std::vector<std::wstring> GetArgsVector(std::wstring args)
    {
        std::vector<std::wstring> result;
        std::wistringstream stream(args);
        std::wstring arg;

        while (getline(stream, arg, L' '))
        {
            result.push_back(arg);
        }

        return result;
    }

} // namespace Microsoft::Reunion::Sidecar
