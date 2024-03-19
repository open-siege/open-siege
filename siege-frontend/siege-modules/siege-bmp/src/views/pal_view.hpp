#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include "win32_controls.hpp"
#include "pal_controller.hpp"

namespace siege::views
{
	struct pal_view
	{
		constexpr static std::u8string_view formats = u8".pal .ipl .ppl .dpl";

		win32::hwnd_t self;
		pal_controller controller;
		std::vector<std::vector<pal_controller::palette>> palettes;
		PAINTSTRUCT paint_data;

		std::array<HBRUSH, 16> brushes;

		pal_view(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
		{
			for (auto i = 0u; i < brushes.size(); ++i)
			{
				brushes[i] = CreateSolidBrush(RGB(i, i * 8, i * 4));
			}
		}

		~pal_view()
		{
			for (auto brush : brushes)
			{
				DeleteObject(brush);
			}
		}

		auto on_create(const win32::create_message&)
		{
			win32::CreateWindowExW(CREATESTRUCTW{
				.hwndParent = self,
				.cy = 200,
				.cx = 100,
				.y = 100,
				.style = WS_CHILD | WS_VISIBLE,
				.lpszName = L"Hello world",
				.lpszClass = win32::button::class_name,			
			});


			return 0;
		}

		auto on_paint(win32::paint_message)
		{
			HDC context = BeginPaint(self, &paint_data);

			RECT pos{};
			pos.right = 100;
			pos.bottom = 100;

			for (auto i = 0; i < brushes.size(); ++i)
			{
				FillRect(context, &pos, brushes[i]);
				OffsetRect(&pos, 100, 0);
			}

			EndPaint(self, &paint_data);
			
			return 0;
		}

		auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);
			
			if (controller.is_pal(stream))
			{
				palettes = controller.load_palettes(stream);

				return TRUE;
			}

			return FALSE;
		}

		static bool is_pal(std::istream& raw_data)
		{
			return false;
		}
	};
}

#endif