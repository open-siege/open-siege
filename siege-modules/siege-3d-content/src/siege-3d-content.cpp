#include <bit>
#include <filesystem>
#include <memory>
#include <siege/platform/win/desktop/win32_common_controls.hpp>
#include <siege/platform/win/desktop/win32_class.hpp>
#include <siege/platform/win/core/com_collection.hpp>
#include <siege/platform/win/core/com_stream_buf.hpp>
#include <siege/platform/win/desktop/window_module.hpp>

extern "C"
{
    extern const std::uint32_t DefaultFileIcon = SIID_IMAGEFILES;

    HRESULT __stdcall GetSupportedExtensions(_Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static std::vector<std::wstring_view> supported_extensions = []{
                std::vector<std::wstring_view> extensions;
                extensions.reserve(32);
                return extensions;
            }();

;
        *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view>>(supported_extensions).release();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedFormatCategories(_In_ LCID, _Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!formats)
        {
            return E_POINTER;
        }

        static auto categories = std::array<std::wstring_view, 1> {{
        }};
        
        *formats = std::make_unique<win32::com::ReadOnlyCollectionRef<std::wstring_view, decltype(categories)>>(categories).release();

        return S_OK;
    }

    HRESULT __stdcall GetSupportedExtensionsForCategory(_In_ const wchar_t* category, _Outptr_ win32::com::IReadOnlyCollection** formats) noexcept
    {
        if (!category)
        {
            return E_INVALIDARG;
        }

        if (!formats)
        {
            return E_POINTER;
        }

        std::wstring_view category_str = category;

        *formats = std::make_unique<win32::com::OwningCollection<std::wstring_view>>().release();
        
        return S_OK;
    }

    _Success_(return == S_OK || return == S_FALSE)
    HRESULT __stdcall IsStreamSupported(_In_ IStream* data) noexcept
    {
        if (!data)
        {
            return E_INVALIDARG;
        }

        return S_FALSE;
    }

    _Success_(return == S_OK || return == S_FALSE)
    HRESULT __stdcall GetWindowClassForStream(_In_ IStream* data, _Outptr_ wchar_t** class_name) noexcept
    {
        if (!data)
        {
            return E_INVALIDARG;
        }

        if (!class_name)
        {
            return E_POINTER;
        }

        static std::wstring empty;
        *class_name = empty.data();
        
        try
        {
            static auto this_module =  win32::window_module_ref::current_module();
            return S_FALSE;
        }
        catch(...)
        {
            return S_FALSE;
        }
    }

    BOOL WINAPI DllMain(
        HINSTANCE hinstDLL,  
        DWORD fdwReason, 
        LPVOID lpvReserved ) noexcept
    {

        if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
        {
            if (lpvReserved != nullptr)
            {
                return TRUE; // do not do cleanup if process termination scenario
            }

            win32::window_module_ref this_module(hinstDLL);

           if (fdwReason == DLL_PROCESS_ATTACH)
           {

            }
            else if (fdwReason == DLL_PROCESS_DETACH)
            {
            }
        }

        return TRUE;
    }
}