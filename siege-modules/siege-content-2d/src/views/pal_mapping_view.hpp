#ifndef DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>

namespace siege::views
{
    struct pal_mapping_view : win32::window_ref
    {
        constexpr static auto formats = std::array<siege::fs_string_view, 1>{{L"palettes.settings.json"}};
    
        pal_mapping_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
	    {
	    }

        auto wm_create(win32::create_message)
        {
            return 0;
        }

        auto wm_size(win32::size_message sized)
	    {
		    return std::nullopt;
	    }
    };
}

#endif//DARKSTARDTSCONVERTER_PAL_MAPPING_VIEW_HPP
