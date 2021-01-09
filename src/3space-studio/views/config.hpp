#ifndef DARKSTARDTSCONVERTER_CONFIG_HPP
#define DARKSTARDTSCONVERTER_CONFIG_HPP

#include "view_factory.hpp"
#include "archives/file_system_archive.hpp"

view_factory create_default_view_factory();

studio::fs::file_system_archive create_default_resource_explorer(const std::filesystem::path& search_path);

#endif//DARKSTARDTSCONVERTER_CONFIG_HPP
