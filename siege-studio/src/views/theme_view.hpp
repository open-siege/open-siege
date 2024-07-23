#ifndef THEME_VIEW_HPP
#define THEME_VIEW_HPP

#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>


namespace siege::views
{
  struct theme_view : win32::window_ref
  {
    win32::window_ref theme_properties;
    win32::list_box options;

    win32::list_view control_settings;

    std::map<std::wstring_view, std::wstring_view> control_labels = {
      { win32::button::class_name, L"Button" },
      { win32::edit::class_name, L"Edit" },
      { win32::static_control::class_name, L"Static Control" },
      { win32::list_box::class_name, L"List Box" },
      { win32::scroll_bar::class_name, L"Scroll Bar" },
      { win32::combo_box::class_name, L"Combo Box" },
      { win32::list_view::class_name, L"List View" },
      { win32::tab_control::class_name, L"Tab Control" },
      { win32::tree_view::class_name, L"Tree View" },
      { win32::combo_box_ex::class_name, L"Combo Box Ex" },
      { win32::header::class_name, L"Header" },
      { win32::tool_bar::class_name, L"Toolbar" },
      { L"Menu", L"Menu" },
      { L"Window", L"Window" }
    };

    std::map<std::wstring_view, std::wstring_view> property_labels = {
      { L"BkColor", L"Background Color" },
      { L"TextColor", L"Text Color" },
      { L"LineColor", L"Line Color" },
      { L"TextBkColor", L"Text Background Color" },
      { L"OutlineColor", L"Outline Color" },
      { L"BtnHighlightColor", L"Button Highlight Color" },
      { L"BtnShadowColor", L"Button Shadow Color" },
      { L"BtnFaceColor", L"Button Face Color" },
      { L"TextHighlightColor", L"Text Highlight Color" },
      { L"MarkColor", L"Mark Color" },
    };

    // simple settings has preferred theme option (from system or user-defined)
    // simple settings has preferred accent color (from system or user-defined)
    // simple settings has theme selection (light, dark)
    // advanced allows theme settings to be changed
    // allows theme settings to be saved
    // theme settings changed per control type

    theme_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
      if (IsWindow((win32::hwnd_t)params.lpCreateParams))
      {
        theme_properties.reset((win32::hwnd_t)params.lpCreateParams);
      }
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      options.InsertString(-1, L"Simple");
      options.InsertString(-1, L"Advanced");
      options.SetCurrentSelection(0);
      ListBox_SetItemHeight(options, 0, options.GetItemHeight(0) * 2);

      control_settings = *control_factory.CreateWindowExW<win32::list_view>(::CREATESTRUCTW{
        .style = WS_CHILD | LVS_REPORT | LVS_EDITLABELS });

      control_settings.EnableGroupView(true);

      control_settings.InsertColumn(-1, LVCOLUMNW{
                                          .pszText = const_cast<wchar_t*>(L"Property"),
                                        });

      control_settings.InsertColumn(-1, LVCOLUMNW{
                                          .pszText = const_cast<wchar_t*>(L"Value"),
                                        });


      std::vector<std::wstring_view> property_names = [] {
        std::vector<std::wstring_view> results;
        results.reserve(32);
        std::copy(win32::properties::tree_view::props.begin(), win32::properties::tree_view::props.end(), std::back_inserter(results));
        std::copy(win32::properties::list_view::props.begin(), win32::properties::list_view::props.end(), std::back_inserter(results));
        std::copy(win32::properties::tool_bar::props.begin(), win32::properties::tool_bar::props.end(), std::back_inserter(results));
        std::copy(win32::properties::list_box::props.begin(), win32::properties::list_box::props.end(), std::back_inserter(results));
        std::copy(win32::properties::window::props.begin(), win32::properties::window::props.end(), std::back_inserter(results));
        std::copy(win32::properties::menu::props.begin(), win32::properties::menu::props.end(), std::back_inserter(results));
        std::copy(win32::properties::header::props.begin(), win32::properties::header::props.end(), std::back_inserter(results));
        std::copy(win32::properties::static_control::props.begin(), win32::properties::static_control::props.end(), std::back_inserter(results));
        std::copy(win32::properties::tab_control::props.begin(), win32::properties::tab_control::props.end(), std::back_inserter(results));
        return results;
      }();

      std::vector<win32::list_view_group> groups;
      groups.reserve(16);

      for (auto& name : property_names)
      {
        std::vector<win32::list_view_item> items;

        auto temp = theme_properties.GetPropW<COLORREF>(name);

        std::wstring property_value = temp ? std::to_wstring(temp) : std::wstring(L"System Default");

        auto separator = name.find(L'.');
        auto control_name = control_labels.at(name.substr(0, separator));
        auto property_name = property_labels.at(name.substr(separator + 1));

        auto existing_group = std::find_if(groups.begin(), groups.end(), [&](auto& item) {
          return item.text == control_name;
        });

        if (existing_group == groups.end())
        {
          win32::list_view_item item{ std::wstring(property_name) };
          item.sub_items.emplace_back(property_value);
          groups.emplace_back(std::wstring(control_name), std::vector<win32::list_view_item>{ std::move(item) });
        }
        else
        {
          auto& item = existing_group->items.emplace_back(std::wstring(property_name));
          item.sub_items.emplace_back(property_value);
        }
      }

      for (auto& group : groups)
      {
        group.state = LVGS_COLLAPSIBLE;
      }

      control_settings.InsertGroups(groups);

      return 0;
    }

    std::optional<win32::lresult_t> wm_notify(win32::notify_message message)
    {
      if (message.code == LBN_SELCHANGE && message.hwndFrom == options)
      {
        ShowWindow(control_settings, options.GetCurrentSelection() == 1 ? SW_SHOW : SW_HIDE);
      }

      return std::nullopt;
    }

    std::optional<win32::lresult_t> wm_notify(win32::list_view_display_info_notfication message)
    {
      if (message.hdr.code == LVN_BEGINLABELEDITW && message.hdr.hwndFrom == control_settings)
      {
        if (message.item.iSubItem == 0)
        {
          return FALSE;
        }
        return TRUE;
      }

      if (message.hdr.code == LVN_ENDLABELEDITW && message.hdr.hwndFrom == control_settings)
      {
        return TRUE;
      }

      return std::nullopt;
    }

    auto wm_size(std::size_t, SIZE client_size)
    {
      auto left_size = SIZE{ .cx = client_size.cx / 3, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy };

      options.SetWindowPos(POINT{});
      options.SetWindowPos(left_size);

      control_settings.SetWindowPos(POINT{ .x = left_size.cx });
      control_settings.SetWindowPos(right_size);

      auto column_count = control_settings.GetColumnCount();

      if (!column_count)
      {
        return 0;
      }

      auto column_width = right_size.cx / column_count;

      for (auto i = 0u; i < column_count; ++i)
      {
        control_settings.SetColumnWidth(i, column_width);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      return std::nullopt;
    }
  };
}// namespace siege::views

#endif