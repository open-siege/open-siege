#include <utility>
#include <execution>
#include <future>
#include <unordered_map>

#include <wx/dataview.h>

#include "pal_mapping_view.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include "bmp_shared.hpp"

namespace studio::views
{
  pal_mapping_view::pal_mapping_view(view_context context)
    : context(std::move(context))
  {
  }

  void pal_mapping_view::setup_view(wxWindow& parent)
  {
    auto palettes = context.explorer.find_files(context.actions.get_extensions_by_category("all_palettes"));

    palette_data.load_palettes(context.explorer, palettes);

    wxArrayString available_palettes;
    available_palettes.reserve(palettes.size() + 2);

    available_palettes.Add(std::string(auto_generated_name));
    available_palettes.Add("Internal");

    for (auto& palette : palettes)
    {
      available_palettes.Add(get_palette_key(context.explorer, palette));
    }

    auto panel = std::make_unique<wxPanel>(&parent);
    auto palettes_have_same_values = std::make_unique<wxCheckBox>(panel.get(), wxID_ANY, "Sync Palette Values");

    auto table = std::unique_ptr<wxDataViewListCtrl>(new wxDataViewListCtrl(&parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_MULTIPLE));

    table->AppendTextColumn("Image Folder", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    table->AppendTextColumn("Image", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

    auto selectionColumn = std::make_unique<wxDataViewColumn>("Default Palette",
      std::make_unique<wxDataViewChoiceRenderer>(available_palettes).release(),
      table->GetColumnCount(),
      wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    table->AppendColumn(selectionColumn.release());


    selectionColumn = std::make_unique<wxDataViewColumn>("Default Palette Index",
      std::make_unique<wxDataViewSpinRenderer>(0, 255).release(),
      table->GetColumnCount(),
      wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

    table->AppendColumn(selectionColumn.release());

    selectionColumn = std::make_unique<wxDataViewColumn>("Selected Palette",
      std::make_unique<wxDataViewChoiceRenderer>(available_palettes).release(),
      table->GetColumnCount(),
      wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    table->AppendColumn(selectionColumn.release());

    selectionColumn = std::make_unique<wxDataViewColumn>("Selected Palette Index",
      std::make_unique<wxDataViewSpinRenderer>(0, 255).release(),
      table->GetColumnCount(),
      wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

    table->AppendColumn(selectionColumn.release());

    table->AppendTextColumn("Actions", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

    table->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, [this, table = table.get()](wxDataViewEvent& event) {
        table->EditItem(event.GetItem(), event.GetDataViewColumn());
    });

    table->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, [this, palettes_have_same_values = palettes_have_same_values.get()](wxDataViewEvent& event) {
      constexpr auto default_palette_key_col = 2;
      constexpr auto default_palette_index_col = 3;
      constexpr auto selected_palette_key_col = 4;
      constexpr auto selected_palette_index_col = 5;

      auto* model = event.GetModel();

      wxVariant folder_path;
      model->GetValue(folder_path, event.GetItem(), 0);

      wxVariant filename;
      model->GetValue(filename, event.GetItem(), 1);

      wxVariant default_palette_key;
      model->GetValue(default_palette_key, event.GetItem(), default_palette_key_col);

      wxVariant default_palette_index;
      model->GetValue(default_palette_index, event.GetItem(), default_palette_index_col);

      wxVariant selected_palette_key;
      model->GetValue(selected_palette_key, event.GetItem(), selected_palette_key_col);

      wxVariant selected_palette_index;
      model->GetValue(selected_palette_index, event.GetItem(), selected_palette_index_col);

      auto palette_iter = palette_data.loaded_palettes.find(default_palette_key.GetString().utf8_string());

      studio::resources::file_info info;
      info.filename = filename.GetString().utf8_string();
      info.folder_path = folder_path.GetString().utf8_string();

      if (palette_iter != palette_data.loaded_palettes.end())
      {
        if (default_palette_index.GetInteger() >= palette_iter->second.second.size())
        {
          default_palette_index = palette_iter->second.second.empty() ? long{} : long(palette_iter->second.second.size() - 1);
          model->SetValue(default_palette_index, event.GetItem(), default_palette_index_col);
        }
      }

      palette_iter = palette_data.loaded_palettes.find(default_palette_key.GetString().utf8_string());

      if (palette_iter != palette_data.loaded_palettes.end())
      {
        if (selected_palette_index.GetInteger() >= palette_iter->second.second.size())
        {
          selected_palette_index = palette_iter->second.second.empty() ? long{} : long(palette_iter->second.second.size() - 1);
          model->SetValue(selected_palette_index, event.GetItem(), selected_palette_index_col);
        }
      }

      if (palettes_have_same_values->GetValue())
      {
        auto* column = event.GetDataViewColumn();

        if (column->GetModelColumn() == default_palette_key_col || column->GetModelColumn() == default_palette_index_col)
        {
          selected_palette_key = default_palette_key;
          selected_palette_index = default_palette_index;
          model->SetValue(selected_palette_key, event.GetItem(), selected_palette_key_col);
          model->SetValue(selected_palette_index, event.GetItem(), selected_palette_index_col);
        }
        else if (column->GetModelColumn() == selected_palette_key_col || column->GetModelColumn() == selected_palette_index_col)
        {
          default_palette_key = selected_palette_key;
          default_palette_index = selected_palette_index;
          model->SetValue(default_palette_key, event.GetItem(), default_palette_key_col);
          model->SetValue(default_palette_index, event.GetItem(), default_palette_index_col);
        }
      }

      set_default_palette(context.explorer, info,
        default_palette_key.GetString().utf8_string(),
        default_palette_index.GetInteger());

      set_selected_palette(context.explorer, info,
        selected_palette_key.GetString().utf8_string(),
        selected_palette_index.GetInteger());
    });

    panel->SetSizer(std::make_unique<wxBoxSizer>(wxHORIZONTAL).release());

    panel->GetSizer()->Add(palettes_have_same_values.release(), 2, wxEXPAND, 0);
    panel->GetSizer()->AddStretchSpacer(8);

    auto images = context.explorer.find_files(context.actions.get_extensions_by_category("all_images"));

    std::unordered_map<std::string, wxDataViewItem> table_rows;
    for (auto& image : images)
    {
      wxVector<wxVariant> results;
      results.reserve(7);

      results.push_back(image.folder_path.string());
      results.push_back(image.filename.string());
      results.push_back("?");
      results.push_back(0);
      results.push_back("?");
      results.push_back(0);

      results.push_back("");

      table->AppendItem(results);
      table_rows.emplace((image.folder_path / image.filename).string(), table->RowToItem(table->GetItemCount() - 1));
    }

    pending_load = std::async(std::launch::async, [this, table = table.get(), images = std::move(images), table_rows = std::move(table_rows)]() mutable {

      struct palette_row
      {
        wxDataViewItem item;
        std::pair<std::string_view, std::size_t> default_palette;
        std::pair<std::string_view, std::size_t> selected_palette;
      };

      auto distance = 16u;


      while (!images.empty())
      {
        auto begin = images.begin();
        auto end = begin;

        if (std::distance(end, images.end()) < distance)
        {
          end = images.end();
        }
        else
        {
          std::advance(end, distance);
        }

        std::vector<palette_row> temp(std::distance(begin, end));
        std::transform(std::execution::par_unseq, begin, end, temp.begin(), [&](const auto& image) -> palette_row {
          auto image_stream = context.explorer.load_file(image);

          auto bmp_data = load_image_data_for_pal_detection(image, *image_stream.second);

          const auto default_palette = detect_default_palette(bmp_data.second, image, context.explorer, palette_data.loaded_palettes);

          const auto selected_palette = selected_palette_from_settings(image, context.explorer, palette_data.loaded_palettes).value_or(default_palette);
          auto key = (image.folder_path / image.filename).string();
          return { table_rows.at(key), default_palette, selected_palette };
        });

        table->CallAfter([table = table, temp = std::move(temp)]() {
          auto* model = table->GetModel();

          // Duplicated because of ice errors when values are static
          constexpr auto default_palette_key_col = 2;
          constexpr auto default_palette_index_col = 3;
          constexpr auto selected_palette_key_col = 4;
          constexpr auto selected_palette_index_col = 5;

          for (auto& row : temp)
          {
            auto& [item, default_palette, selected_palette] = row;
            model->SetValue(std::string(default_palette.first), item, default_palette_key_col);
            model->SetValue(long(default_palette.second), item, default_palette_index_col);
            model->SetValue(std::string(selected_palette.first), item, selected_palette_key_col);
            model->SetValue(long(selected_palette.second), item, selected_palette_index_col);
          }
        });

        images.erase(begin, end);
      }
    });

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(panel.release(), 1, wxEXPAND, 0);
    sizer->Add(table.release(), 15, wxEXPAND, 0);

    parent.SetSizer(sizer.release());
  }
}// namespace studio::views