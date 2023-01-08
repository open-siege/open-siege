#ifndef OPEN_SIEGE_FLY2000_HPP
#define OPEN_SIEGE_FLY2000_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace fly200
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "fly2000",
    "Fly2000",
    "fly! 2000",
    "Fly! 2000",
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "FLY2K"
    };

    result.default_install_path = "<systemDrive>/Games/Fly! 2000";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"data.CAB", "."},
    {"_user1.CAB/*.bmp", "assets"},
    {"fly.ico", "."}
  };
}




#endif// OPEN_SIEGE_FLY2000_HPP
