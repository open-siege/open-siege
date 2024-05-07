#ifndef WINDOW_MODULE_HPP
#define WINDOW_MODULE_HPP

#include <expected>
#include <siege/platform/win/core/module.hpp>
#include <siege/platform/win/desktop/win32_window.hpp>
#include <WinUser.h>

namespace win32
{
    inline auto widen(std::string_view data)
     {
            return std::wstring(data.begin(), data.end());
     }

     template <typename TType>
     auto type_name()
     {
            static auto name = widen(typeid(TType).name());
            return name;
     }

     template <typename TType>
     std::wstring window_class_name()
     {
        if constexpr (requires(TType t) { TType::class_name; })
        {
            return TType::class_name;
        }
     
        return L"";
     }

	struct window_module_ref : win32::module_ref
	{
		using module_ref::module_ref;
	
		inline static window_module_ref current_module()
		{
			return window_module_ref(module_ref::current_module().get()); 
		}

        auto RegisterClassW(::WNDCLASSEXW meta)
        {
            meta.cbSize = sizeof(::WNDCLASSEXW);
            return ::RegisterClassExW(&meta);
        }

        template <typename TClass>
        auto UnregisterClassW()
        {
            return ::UnregisterClassW(type_name<TClass>().c_str(), get());
        }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		std::optional<WNDCLASSEXW> GetClassInfoExW(std::wstring_view name)
		{
			WNDCLASSEXW temp{.cbSize = sizeof(WNDCLASSEXW)};

			if (::GetClassInfoExW(*this, name.data(), &temp));
			{
                if (!temp.lpfnWndProc)
                {
                    return std::nullopt;
                }

                try
                {
                    temp.hInstance = win32::module_ref(temp.lpfnWndProc);
                }
                catch(...)
                {
                }

				return temp;
			}

			return std::nullopt;
		}
#endif

		template <typename TControl = window>
        std::expected<TControl, DWORD> CreateWindowExW(CREATESTRUCTW params)
        {
            std::wstring class_name;

            if (!params.lpszClass || params.lpszClass[0] == L'\0')
            {
                class_name = window_class_name<TControl>();
                params.lpszClass = class_name.c_str();
            }

            auto result = ::CreateWindowExW(
                    params.dwExStyle,
                    params.lpszClass,
                    params.lpszName,
                    params.style,
                    params.x,
                    params.y,
                    params.cx,
                    params.cy,
                    params.hwndParent,
                    params.hMenu,
                    *this,
                    params.lpCreateParams
                );

            if (!result)
            {
                return std::unexpected(GetLastError());
            }

            return TControl(result);
        }
	};

}


#endif