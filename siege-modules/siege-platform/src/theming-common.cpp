#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/stream.hpp>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  void apply_theme(const win32::window_ref& colors, win32::header& control)
  {
    struct sub_class final : win32::header::notifications
    {
      using win32::header::notifications::wm_notify;

      std::map<std::wstring_view, COLORREF> colors;

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      virtual std::optional<win32::lresult_t> wm_notify(win32::header header, NMCUSTOMDRAW& custom_draw) override
      {

        if (custom_draw.dwDrawStage == CDDS_PREPAINT)
        {
          auto font = win32::load_font(LOGFONTW{
            .lfPitchAndFamily = VARIABLE_PITCH,
            .lfFaceName = L"Segoe UI" });

          auto text_bk_color = colors[properties::header::text_bk_color];
          FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(text_bk_color));

          return CDRF_NOTIFYITEMDRAW | CDRF_NEWFONT;
        }


        if (custom_draw.dwDrawStage == CDDS_ITEMPREPAINT)
        {
          auto focused_item = Header_GetFocusedItem(header);

          auto text_highlight_color = colors[properties::header::text_highlight_color];
          auto text_bk_color = colors[properties::header::text_bk_color];

          if (custom_draw.dwItemSpec == focused_item)
          {
            ::SetBkColor(custom_draw.hdc, text_highlight_color);
            ::SelectObject(custom_draw.hdc, get_solid_brush(text_highlight_color));
          }
          else
          {
            ::SetBkColor(custom_draw.hdc, text_bk_color);
            ::SelectObject(custom_draw.hdc, get_solid_brush(text_bk_color));
          }

          auto text_color = colors[properties::header::text_color];
          ::SetTextColor(custom_draw.hdc, text_color);

          return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
        }

        if (custom_draw.dwDrawStage == CDDS_ITEMPOSTPAINT)
        {
          auto rect = custom_draw.rc;

          if (header.GetWindowStyle() & HDS_FILTERBAR)
          {
            thread_local std::wstring filter_value;
            filter_value.clear();
            filter_value.resize(255, L'\0');
            HD_TEXTFILTERW string_filter{
              .pszText = filter_value.data(),
              .cchTextMax = (int)filter_value.size(),
            };

            win32::gdi::drawing_context_ref context(custom_draw.hdc);
            auto header_item = header.GetItem(custom_draw.dwItemSpec, { .mask = HDI_FILTER, .type = HDFT_ISSTRING, .pvFilter = &string_filter });

            filter_value.erase(std::wcslen(filter_value.data()));


            auto bottom = custom_draw.rc;

            rect.bottom = rect.bottom / 2;
            bottom.top = rect.bottom;
            FillRect(custom_draw.hdc, &bottom, get_solid_brush(colors[properties::header::text_bk_color]));

            bottom.left += 10;
            bottom.right -= 10;
            if (filter_value.empty())
            {
              ::DrawTextW(context, L"Enter text here", -1, &bottom, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
            }
            else
            {
              ::DrawTextW(context, filter_value.c_str(), -1, &bottom, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
            }
          }

          return CDRF_DODEFAULT;
        }

        return CDRF_DODEFAULT;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto result = win32::header::notifications::dispatch_message((sub_class*)dwRefData, message, wParam, lParam);

        if (result)
        {
          return *result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::header::bk_color, *colors.FindPropertyExW<COLORREF>(win32::properties::header::bk_color) },
      { win32::properties::header::text_color, *colors.FindPropertyExW<COLORREF>(win32::properties::header::text_color) },
      { win32::properties::header::text_bk_color, *colors.FindPropertyExW<COLORREF>(win32::properties::header::text_bk_color) },
      { win32::properties::header::text_highlight_color, *colors.FindPropertyExW<COLORREF>(win32::properties::header::text_highlight_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::header::class_name, &existing_object) && existing_object == 0)
    {
      win32::theme_module().SetWindowTheme(control, L"", L"");
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::header::class_name, (DWORD_PTR) new sub_class(std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }

  void apply_theme(const win32::window_ref& colors, win32::tab_control& control)
  {
    struct sub_class final : win32::tab_control::notifications
    {
      std::map<std::wstring_view, COLORREF> colors;

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::tab_control tabs, DRAWITEMSTRUCT& item) override
      {
        thread_local std::wstring buffer(256, '\0');

        if (item.itemAction == ODA_DRAWENTIRE || item.itemAction == ODA_SELECT)
        {
          auto context = win32::gdi::drawing_context_ref(item.hDC);

          SetBkColor(context, colors[win32::properties::tab_control::bk_color]);

          win32::tab_control control(item.hwndItem);

          auto item_rect = item.rcItem;

          auto text_highlight_color = colors[win32::properties::tab_control::text_highlight_color];
          auto text_bk_color = colors[win32::properties::tab_control::text_bk_color];

          if (item.itemState & ODS_HOTLIGHT)
          {
            context.FillRect(item_rect, get_solid_brush(text_highlight_color));
          }
          else if (item.itemState & ODS_SELECTED)
          {
            context.FillRect(item_rect, get_solid_brush(text_highlight_color));
          }
          else
          {
            context.FillRect(item_rect, get_solid_brush(text_bk_color));
          }

          auto text_color = colors[win32::properties::tab_control::text_color];
          ::SetTextColor(context, text_color);
          ::SetBkMode(context, TRANSPARENT);
          ::SelectObject(context, get_solid_brush(text_bk_color));

          auto item_info = control.GetItem(item.itemID, TCITEMW{ .mask = TCIF_TEXT, .pszText = buffer.data(), .cchTextMax = int(buffer.size()) });

          if (item_info)
          {
            ::DrawTextW(context, (LPCWSTR)item_info->pszText, -1, &item.rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
          }
          return TRUE;
        }

        return std::nullopt;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto result = win32::tab_control::notifications::dispatch_message((sub_class*)dwRefData, message, wParam, lParam);

        if (result)
        {
          return *result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }

      static LRESULT __stdcall HandleChildMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_PAINT)
        {
          PAINTSTRUCT ps;
          HDC hdc = BeginPaint(hWnd, &ps);

          
          auto* self = (sub_class*)dwRefData;
          auto tabs = win32::tab_control(hWnd);
          auto parent_context = win32::gdi::drawing_context_ref(hdc);
          auto rect = tabs.GetClientRect();
          auto bk_color = self->colors[win32::properties::tab_control::bk_color];

          parent_context.FillRect(*rect, get_solid_brush(bk_color));

          auto result = DefSubclassProc(hWnd, message, (WPARAM)hdc, lParam);

          auto text_bk_color = self->colors[win32::properties::tab_control::text_bk_color];
          auto pen = CreatePen(PS_SOLID, 3, text_bk_color);

          auto old_pen = SelectObject(hdc, pen);
          SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

          SetDCPenColor(hdc, RGB(0, 0, 0));


          auto x_border = GetSystemMetrics(SM_CXEDGE);
          auto y_border = GetSystemMetrics(SM_CYEDGE);

          
          auto client_area = tabs.GetClientRect();

          client_area = tabs.AdjustRect(false, *client_area);

          client_area->left = std::clamp<LONG>(client_area->left - x_border, 0, client_area->left);
          client_area->right += x_border;
          client_area->top = std::clamp<LONG>(client_area->top - y_border, 0, client_area->top);
          client_area->bottom += y_border;

          auto count = tabs.GetItemCount();

          if (count > 0)
          {
            auto tab_rect = tabs.GetItemRect(count - 1);
            rect->left = tab_rect->right;
            rect->bottom = tab_rect->bottom;
          }

          parent_context.FillRect(*rect, get_solid_brush(bk_color));
          parent_context.FillRect(*client_area, get_solid_brush(bk_color));

          RECT item_rect;
          for (auto i = 0; i < TabCtrl_GetItemCount(hWnd); ++i)
          {
            TabCtrl_GetItemRect(hWnd, i, &item_rect);
            Rectangle(hdc, item_rect.left, item_rect.top, item_rect.right, item_rect.bottom);
          }

          SelectObject(hdc, old_pen);
          DeleteObject(pen);

          EndPaint(hWnd, &ps);

          return result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    auto style = control.GetWindowStyle();
    control.SetWindowStyle(style | TCS_OWNERDRAWFIXED);


    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::tab_control::bk_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tab_control::bk_color) },
      { win32::properties::tab_control::text_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tab_control::text_color) },
      { win32::properties::tab_control::text_bk_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tab_control::text_bk_color) },
      { win32::properties::tab_control::text_highlight_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tab_control::text_highlight_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::tab_control::class_name, &existing_object) && existing_object == 0)
    {
      auto data = (DWORD_PTR) new sub_class(std::move(color_map));
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::tab_control::class_name, data);
      ::SetWindowSubclass(control, sub_class::HandleChildMessage, (UINT_PTR)win32::tab_control::class_name, data);
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }

  void
    apply_theme(const win32::window_ref& colors, win32::list_view& control)
  {
    static auto default_bk_color = ListView_GetBkColor(control);
    auto color = colors.FindPropertyExW<COLORREF>(properties::list_view::bk_color).value_or(default_bk_color);
    ListView_SetBkColor(control, color);

    static auto default_text_color = ListView_GetTextColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::text_color).value_or(default_text_color);
    ListView_SetTextColor(control, color);

    static auto default_text_bk_color = ListView_GetTextBkColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::text_bk_color).value_or(default_text_bk_color);
    ListView_SetTextBkColor(control, color);

    static auto default_outline_color = ListView_GetOutlineColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::outline_color).value_or(default_outline_color);
    ListView_SetOutlineColor(control, color);

    auto header = control.GetHeader();

    if (header)
    {
      win32::apply_theme(colors, header);
    }

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }

    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }

  void apply_theme(const win32::window_ref& colors, win32::tree_view& control)
  {
    auto useDarkMode = colors.FindPropertyExW<bool>(L"AppsUseDarkTheme").value_or(false);
    auto color = colors.FindPropertyExW<COLORREF>(properties::tree_view::bk_color).value_or(CLR_NONE);
    TreeView_SetBkColor(control, color);

    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::text_color).value_or(CLR_NONE);
    TreeView_SetTextColor(control, color);

    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::line_color).value_or(CLR_NONE);
    TreeView_SetLineColor(control, color);

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (useDarkMode)
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }
    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }

  void apply_theme(const win32::window_ref& colors, win32::tool_bar& control)
  {
    auto highlight_color = colors.FindPropertyExW<COLORREF>(properties::tool_bar::btn_highlight_color);
    auto shadow_color = colors.FindPropertyExW<COLORREF>(properties::tool_bar::btn_shadow_color);

    bool change_theme = false;

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    win32::theme_module().SetWindowTheme(control, L"", L"");
    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (highlight_color || shadow_color)
    {
      change_theme = true;
      COLORSCHEME scheme{ .dwSize = sizeof(COLORSCHEME), .clrBtnHighlight = CLR_DEFAULT, .clrBtnShadow = CLR_DEFAULT };

      if (highlight_color)
      {
        scheme.clrBtnHighlight = *highlight_color;
      }

      if (shadow_color)
      {
        scheme.clrBtnShadow = *shadow_color;
      }
      ::SendMessageW(control, TB_SETCOLORSCHEME, 0, (LPARAM)&scheme);
    }

    struct sub_class final : win32::tool_bar::notifications
    {
      using win32::tool_bar::notifications::wm_notify;

      std::map<std::wstring_view, COLORREF> colors;

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      std::optional<win32::lresult_t> wm_notify(win32::tool_bar buttons, NMTBCUSTOMDRAW& custom_draw) override
      {
        if (custom_draw.nmcd.dwDrawStage == CDDS_PREPAINT)
        {
          auto font = win32::load_font(LOGFONTW{
            .lfPitchAndFamily = VARIABLE_PITCH,
            .lfFaceName = L"Segoe UI" });

          SelectFont(custom_draw.nmcd.hdc, font);
          return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
        }

        if (custom_draw.nmcd.dwDrawStage == CDDS_ITEMPREPAINT || custom_draw.nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
        {
          custom_draw.clrBtnFace = colors[properties::tool_bar::btn_face_color];
          custom_draw.clrBtnHighlight = colors[properties::tool_bar::btn_highlight_color];
          custom_draw.clrText = colors[properties::tool_bar::text_color];

          return CDRF_NEWFONT | TBCDRF_USECDCOLORS;
        }

        if (custom_draw.nmcd.dwDrawStage == CDDS_POSTPAINT)
        {
          win32::gdi::drawing_context_ref context(custom_draw.nmcd.hdc);

          auto count = buttons.ButtonCount();
          auto rect = buttons.GetClientRect();
          if (count > 0)
          {
            auto button_rect = buttons.GetItemRect(count - 1);

            rect->left = button_rect->right;
            rect->bottom = button_rect->bottom;

            context.FillRect(*rect, get_solid_brush(colors[properties::tool_bar::bk_color]));
          }
        }

        return CDRF_DODEFAULT;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto result = win32::tool_bar::notifications::dispatch_message((sub_class*)dwRefData, message, wParam, lParam);

        if (result)
        {
          return *result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::tool_bar::bk_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tool_bar::bk_color) },
      { win32::properties::tool_bar::text_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tool_bar::text_color) },
      { win32::properties::tool_bar::btn_face_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tool_bar::btn_face_color) },
      { win32::properties::tool_bar::btn_highlight_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tool_bar::btn_highlight_color) },
      { win32::properties::tool_bar::btn_shadow_color, *colors.FindPropertyExW<COLORREF>(win32::properties::tool_bar::btn_shadow_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::tool_bar::class_name, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::tool_bar::class_name, (DWORD_PTR) new sub_class(std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }

}// namespace win32