#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <siege/platform/win/desktop/win32_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/auto_handle.hpp>
#include <siege/platform/win/core/com_collection.hpp>
#include <cassert>
#include <sstream>
#include <vector>
#include <memory>
#include <istream>
#include <spanstream>
#include <oleacc.h>
#include "bmp_controller.hpp"

namespace siege::views
{
    struct gdi_deleter
    {
        void operator()(HGDIOBJ gdi_obj)
        {
            assert(::DeleteObject(gdi_obj) == TRUE);
        }
    };

    using gdi_bitmap = win32::auto_handle<HBITMAP, gdi_deleter>;
    using gdi_brush = win32::auto_handle<HBRUSH, gdi_deleter>;
    using gdi_palette = win32::auto_handle<HPALETTE, gdi_deleter>;
    using gdi_pen = win32::auto_handle<HPEN, gdi_deleter>;
    using gdi_font = win32::auto_handle<HFONT, gdi_deleter>;

	struct bmp_view : win32::window
    {
        constexpr static auto formats = std::array<std::wstring_view, 16>{{
        L".jpg", L".jpeg", L".gif", L".png", L".tag", L".bmp", L".dib" , L".pba", L".dmb", L".db0", L".db1", L".db2", L".hba", L".hb0", L".hb1", L".hb2"    
        }};

        win32::static_control static_image;
        win32::list_view palettes_list;
        bmp_controller controller;

        win32::auto_handle<HBITMAP, gdi_deleter> current_bitmap;

        bmp_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window(self)
	    {
	    }

        auto on_create(const win32::create_message& info)
        {
            auto parent_size = this->GetClientSize();
            auto this_module = win32::window_factory();

            auto width = parent_size->cx;

            std::wstring temp = L"menu.pal";
      
            palettes_list = [&] {
                auto palettes_list = *this_module.CreateWindowExW<win32::list_view>(::CREATESTRUCTW{
                            .hwndParent = *this,
						    .cy = 300,  
                            .cx = 200,
	            		    .y = 3,		
                            .x = width - 200, 
                            .style = WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            .lpszName =  L"Palettes",
                            .lpszClass = L"MFC::CMFCListCtrl"
                });

                palettes_list.SetView(win32::list_view::view_type::details_view);
            assert(palettes_list.EnableGroupView(true));
            assert(palettes_list.SetTileViewInfo(LVTILEVIEWINFO {
                .dwFlags = LVTVIF_FIXEDWIDTH,
                .sizeTile = SIZE {.cx = this->GetClientSize()->cx, .cy = 50},
                }));
        
            palettes_list.SetExtendedListViewStyle(0, 
                    LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

            palettes_list.win32::list_view::InsertColumn(-1, LVCOLUMNW {
                 .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L""),
                .cxMin = 100,
                .cxDefault = this->GetClientSize()->cx,
                });

            std::vector<win32::list_view_group> groups {
                {L"main.dpl", {{ L"palette 1"}, { L"palette 2"}, { L"palette 3"}}},
                {L"test.pal", {{ L"palette 1"}}},
                {L"other.ipl", {{ L"palette 1"}}},
            };

            palettes_list.InsertGroups(groups);

            return palettes_list;
          }();

          static_image = *this_module.CreateWindowExW<win32::static_control>(
              win32::window_point_size{ 
                .parent = *this,
                .style = win32::window_style(WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_REALSIZECONTROL),
                .size{.cx = (width / 3) * 2, .cy = parent_size->cy - 20}
                });


            return 0;
        }

        std::optional<LRESULT> on_get_object(win32::get_object_message message)
        {
            if (message.object_id == OBJID_NATIVEOM)
            {
                auto collection = std::make_unique<win32::com::OwningCollection<std::unique_ptr<IStream, void(*)(IStream*)>>>();

                auto result = LresultFromObject(__uuidof(IDispatch), message.flags, static_cast<IDispatch*>(collection.get()));

                collection.release()->Release();

                return result;
            }

            return std::nullopt;
        }

        auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);
			
			if (bmp_controller::is_bmp(stream))
			{
                auto count = controller.load_bitmap(stream);

                if (count > 0)
                {
                    auto size = static_image.GetClientSize();
                    BITMAPINFO info{
                                .bmiHeader {
                                    .biSize = sizeof(BITMAPINFOHEADER),
                                    .biWidth = LONG(size->cx),
                                    .biHeight = LONG(size->cy),
                                    .biPlanes = 1,
                                    .biBitCount = 32,
                                    .biCompression = BI_RGB
                                }
                            };
                   auto wnd_dc = ::GetDC(nullptr);

                    void* pixels = nullptr;
                    current_bitmap.reset(::CreateDIBSection(wnd_dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0));

                    controller.convert(0, std::make_pair(size->cx, size->cy), 32, std::span(reinterpret_cast<std::byte*>(pixels), size->cx * size->cy * 4));

                    static_image.SetImage(current_bitmap.get());

                    return TRUE;
                }

                return FALSE;
			}

			return FALSE;
		}

        auto on_message(win32::message message)
        {
            if (message.message == WM_MEASUREITEM || message.message == WM_DRAWITEM)
            {
                win32::ForEachDirectChildWindow(*this, [&](auto child) {

            
		        });
        
            }
		    return std::nullopt;
        }

        auto on_command(win32::command_message message)
        {
            auto sender = message.sender;

            auto menu = GetMenu(sender);

            if (menu)
            {
                auto rect = win32::window(sender).GetClientRect();

                POINT pos {
                    .x = rect->left,
                    .y = rect->top
                };

                ClientToScreen(sender, &pos);
                TrackPopupMenu(menu, TPM_LEFTALIGN, pos.x, pos.y + rect->bottom,  0, sender, nullptr);
            }

            return std::nullopt;
        }

        auto on_size(win32::size_message sized)
	    {
            auto left_size = sized.client_size;
            left_size.cx = (left_size.cx / 3) * 2;
            auto right_size = sized.client_size;
            right_size.cx -= left_size.cx;

            static_image.SetWindowPos(POINT{});
            static_image.SetWindowPos(left_size);

            palettes_list.SetWindowPos(POINT{.x = sized.client_size.cx - right_size.cx});
            palettes_list.SetWindowPos(right_size);

            return std::nullopt;
        }

        static bool is_bitmap(std::istream& raw_data)
        {
            return false;
        }
    };
}

#endif