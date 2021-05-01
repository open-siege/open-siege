#include <array>
#include <utility>
#include <vector>
#include "shared.hpp"
#include "endian_arithmetic.hpp"
#include "content/tagged_data.hpp"
#include "3space.hpp"

namespace studio::content::dts::three_space
{
  namespace endian = boost::endian;
  using file_tag = std::array<std::byte, 4>;

  namespace v1
  {
    constexpr file_tag grid_shape_tag = shared::to_tag<4>({ 0x01, 0x00, 0xbc, 0x02 });
    constexpr file_tag rectangle_region_tag = shared::to_tag<4>({ 0x0c, 0x00, 0x28, 0x00 });
    constexpr file_tag edge_table_tag = shared::to_tag<4>({ 0x0d, 0x00, 0x28, 0x00 });

    constexpr file_tag null_obj_tag = shared::to_tag<4>({ 0x0e, 0x00, 0x14, 0x00 });
    constexpr file_tag poly_tag = shared::to_tag<4>({ 0x01, 0x00, 0x14, 0x00 });
    constexpr file_tag solid_poly_tag = shared::to_tag<4>({ 0x02, 0x00, 0x14, 0x00 });
    constexpr file_tag texture_for_poly_tag = shared::to_tag<4>({ 0x0f, 0x00, 0x14, 0x00 });
    constexpr file_tag shaded_polygon_tag = shared::to_tag<4>({ 0x03, 0x00, 0x14, 0x00 });
    constexpr file_tag gourand_poly_tag = shared::to_tag<4>({ 0x09, 0x00, 0x14, 0x00 });
    constexpr file_tag alias_solid_poly_tag = shared::to_tag<4>({ 0x10, 0x00, 0x14, 0x00 });
    constexpr file_tag alias_shaded_poly_tag = shared::to_tag<4>({ 0x11, 0x00, 0x14, 0x00 });
    constexpr file_tag alias_gourand_poly_tag = shared::to_tag<4>({ 0x12, 0x00, 0x14, 0x00 });
    constexpr file_tag group_tag = shared::to_tag<4>({ 0x14, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_group_tag = shared::to_tag<4>({ 0x0a, 0x00, 0x14, 0x00 });
    constexpr file_tag base_part_tag = shared::to_tag<4>({ 0x05, 0x00, 0x14, 0x00 });
    constexpr file_tag bitmap_part_tag = shared::to_tag<4>({ 0x13, 0x00, 0x14, 0x00 });
    constexpr file_tag part_list_tag = shared::to_tag<4>({ 0x07, 0x00, 0x14, 0x00 });
    constexpr file_tag cell_anim_part_tag = shared::to_tag<4>({ 0x0b, 0x00, 0x14, 0x00 });
    constexpr file_tag detail_part_tag = shared::to_tag<4>({ 0x0c, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_part_tag = shared::to_tag<4>({ 0x15, 0x00, 0x14, 0x00 });
    constexpr file_tag shape_tag = shared::to_tag<4>({ 0x08, 0x00, 0x14, 0x00 });
    constexpr file_tag an_sequence_tag = shared::to_tag<4>({ 0x01, 0x00, 0x1e, 0x00 });
    constexpr file_tag an_cyclic_sequence_tag = shared::to_tag<4>({ 0x04, 0x00, 0x1e, 0x00 });
    constexpr file_tag an_anim_list_tag = shared::to_tag<4>({ 0x02, 0x00, 0x1e, 0x00 });
    constexpr file_tag an_shape_tag = shared::to_tag<4>({ 0x03, 0x00, 0x1e, 0x00 });
    constexpr file_tag dc_shape_tag = shared::to_tag<4>({ 0xf8, 0x01, 0xbc, 0x02 });
    constexpr file_tag dc_shape_instance_tag = shared::to_tag<4>({ 0xf5, 0x01, 0xbc, 0x02 });

    base_part read_base_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map&)
    {
      base_part base{};
      file.read(reinterpret_cast<std::byte*>(&base.transform), sizeof(base.transform));
      file.read(reinterpret_cast<std::byte*>(&base.id_number), sizeof(base.id_number));
      file.read(reinterpret_cast<std::byte*>(&base.radius), sizeof(base.radius));
      file.read(reinterpret_cast<std::byte*>(&base.center), sizeof(base.center));

      return base;
    }

    part_list read_part_list(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      part_list parts{};
      parts.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&parts.part_count), sizeof(parts.part_count));
      parts.parts = read_children<shape_item>(file, parts.part_count, readers);
      return parts;
    }

    bsp_part read_bsp_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bsp_part shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<std::byte*>(&shape.node_count), sizeof(shape.node_count));

      shape.nodes = std::vector<bsp_part::bsp_node>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.nodes.data()), sizeof(bsp_part::bsp_node) * shape.node_count);

      shape.transforms = std::vector<endian::little_uint16_t>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.transforms.data()), sizeof(endian::little_uint16_t) * shape.node_count);

      return shape;
    }

    group read_group(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      group shape{};

      shape.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.index_count), sizeof(shape.index_count));
      file.read(reinterpret_cast<std::byte*>(&shape.point_count), sizeof(shape.point_count));
      file.read(reinterpret_cast<std::byte*>(&shape.color_count), sizeof(shape.color_count));
      file.read(reinterpret_cast<std::byte*>(&shape.item_count), sizeof(shape.item_count));

      shape.indexes = std::vector<endian::little_uint16_t>(shape.index_count);
      file.read(reinterpret_cast<std::byte*>(shape.indexes.data()), sizeof(endian::little_uint16_t) * shape.index_count);

      shape.points = std::vector<vector3s>(shape.point_count);
      file.read(reinterpret_cast<std::byte*>(shape.points.data()), sizeof(vector3s) * shape.point_count);

      shape.colors = std::vector<std::array<std::byte, 4>>(shape.color_count);
      file.read(reinterpret_cast<std::byte*>(shape.colors.data()), sizeof(std::array<std::byte, 4>) * shape.color_count);

      shape.items = read_children<shape_item>(file, shape.item_count, readers);

      return shape;
    }

    bsp_group read_bsp_group(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bsp_group shape{};

      shape.base = read_group(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.node_count), sizeof(shape.node_count));

      shape.nodes = std::vector<bsp_group::bsp_group_node>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.nodes.data()), sizeof(bsp_group::bsp_group_node) * shape.node_count);

      return shape;
    }

    poly read_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      poly poly{};

      file.read(reinterpret_cast<std::byte*>(&poly.normal), sizeof(poly.normal));
      file.read(reinterpret_cast<std::byte*>(&poly.center), sizeof(poly.center));
      file.read(reinterpret_cast<std::byte*>(&poly.vertex_count), sizeof(poly.vertex_count));
      file.read(reinterpret_cast<std::byte*>(&poly.vertex_list), sizeof(poly.vertex_list));

      return poly;
    }

    solid_poly read_solid_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      solid_poly poly{};
      poly.base = read_poly(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&poly.colors), sizeof(poly.colors));

      return poly;
    }

    texture_for_poly read_texture_for_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      texture_for_poly poly{};
      poly.base = read_solid_poly(file, header, readers);

      return poly;
    }

    gouraud_poly read_gouraud_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      gouraud_poly poly{};
      poly.base = read_solid_poly(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&poly.normal_list), sizeof(poly.normal_list));

      return poly;
    }

    shaded_poly read_shaded_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      shaded_poly poly{};
      poly.base = read_solid_poly(file, header, readers);

      return poly;
    }

    cell_anim_part read_cell_anim_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      cell_anim_part shape{};
      shape.base = read_part_list(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.anim_sequence), sizeof(shape.anim_sequence));
      return shape;
    }

    detail_part read_detail_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      detail_part shape{};
      shape.base = read_part_list(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.detail_count), sizeof(shape.detail_count));

      shape.details = std::vector<endian::little_int16_t>(shape.detail_count);
      file.read(reinterpret_cast<std::byte*>(shape.details.data()), sizeof(endian::little_int16_t) * shape.detail_count);

      return shape;
    }

    bitmap_part read_bitmap_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bitmap_part shape{};
      shape.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.bmp_tag), sizeof(shape.bmp_tag));
      file.read(reinterpret_cast<std::byte*>(&shape.x_offset), sizeof(shape.x_offset));
      file.read(reinterpret_cast<std::byte*>(&shape.y_offset), sizeof(shape.y_offset));

      return shape;
    }

    an_sequence read_an_sequence(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      an_sequence shape{};

      file.read(reinterpret_cast<std::byte*>(&shape.tick), sizeof(shape.tick));
      file.read(reinterpret_cast<std::byte*>(&shape.priority), sizeof(shape.priority));
      file.read(reinterpret_cast<std::byte*>(&shape.gm), sizeof(shape.gm));

      file.read(reinterpret_cast<std::byte*>(&shape.frame_count), sizeof(shape.frame_count));
      shape.frames = std::vector<an_sequence::frame>(shape.frame_count);
      file.read(reinterpret_cast<std::byte*>(shape.frames.data()), sizeof(an_sequence::frame) * shape.frame_count);

      file.read(reinterpret_cast<std::byte*>(&shape.part_list_count), sizeof(shape.part_list_count));
      shape.part_list = shape.part_list = read_children<shape_item>(file, shape.part_list_count, readers);

      auto transform_count = shape.frame_count * shape.part_list_count;

      shape.transform_index_list = std::vector<endian::little_uint16_t>(transform_count);
      file.read(reinterpret_cast<std::byte*>(shape.transform_index_list.data()), sizeof(endian::little_uint16_t) * transform_count);

      return shape;
    }

    an_cyclic_sequence read_an_cyclic_sequence(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      an_cyclic_sequence shape{};
      shape.base = read_an_sequence(file, header, readers);

      return shape;
    }

    an_anim_list read_an_anim_list(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      an_anim_list shape{};
      file.read(reinterpret_cast<std::byte*>(&shape.sequence_count), sizeof(shape.sequence_count));

      shape.sequences.reserve(shape.sequence_count);
      for (auto i = 0; i < shape.sequence_count; ++i)
      {
        object_header child_header;
        file.read(reinterpret_cast<std::byte*>(&child_header), sizeof(child_header));
        shape.sequences.emplace_back(read_an_sequence(file, child_header, readers));
      }

      file.read(reinterpret_cast<std::byte*>(&shape.transition_count), sizeof(shape.transition_count));
      shape.transitions = std::vector<an_anim_list::transition>(shape.transition_count);
      file.read(reinterpret_cast<std::byte*>(shape.transitions.data()), sizeof(an_anim_list::transition) * shape.transition_count);

      file.read(reinterpret_cast<std::byte*>(&shape.transition_count), sizeof(shape.transition_count));
      shape.transitions = std::vector<an_anim_list::transition>(shape.transition_count);
      file.read(reinterpret_cast<std::byte*>(shape.transitions.data()), sizeof(an_anim_list::transition) * shape.transition_count);

      file.read(reinterpret_cast<std::byte*>(&shape.transform_count), sizeof(shape.transform_count));
      shape.transforms = std::vector<an_anim_list::transform>(shape.transform_count);
      file.read(reinterpret_cast<std::byte*>(shape.transforms.data()), sizeof(an_anim_list::transform) * shape.transform_count);

      file.read(reinterpret_cast<std::byte*>(&shape.default_transform_count), sizeof(shape.default_transform_count));
      shape.default_transforms = std::vector<endian::little_uint16_t>(shape.default_transform_count);
      file.read(reinterpret_cast<std::byte*>(shape.default_transforms.data()), sizeof(endian::little_uint16_t) * shape.default_transform_count);

      file.read(reinterpret_cast<std::byte*>(&shape.relations_count), sizeof(shape.relations_count));
      shape.relations = std::vector<an_anim_list::relation>(shape.relations_count);
      file.read(reinterpret_cast<std::byte*>(shape.relations.data()), sizeof(an_anim_list::relation) * shape.relations_count);

      return shape;
    }

    shape read_shape(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      shape shape{};
      shape.base = read_part_list(file, header, readers);

      return shape;
    }

    an_shape read_an_shape(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      an_shape shape{};
      shape.base = read_shape(file, header, readers);

      return shape;
    }

    bool is_3space_dts(std::basic_istream<std::byte>& stream)
    {
      file_tag header{};
      stream.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

      stream.seekg(-int(sizeof(header)), std::ios::cur);

      return header == shape_tag || header == dc_shape_tag || header == an_shape_tag;
    }

    std::vector<shape_item> read_shapes(std::basic_istream<std::byte>& file)
    {
      static shape_reader_map readers = {
        { shape_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_shape(file, header, readers); } } },
        { bsp_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_bsp_part(file, header, readers); } } },
        { part_list_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_part_list(file, header, readers); } } },
        { base_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_base_part(file, header, readers); } } },
        { detail_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_detail_part(file, header, readers); } } },
        { bitmap_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_bitmap_part(file, header, readers); } } },
        { cell_anim_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_cell_anim_part(file, header, readers); } } },
        { group_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_group(file, header, readers); } } },
        { bsp_group_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_bsp_group(file, header, readers); } } },
        { poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_poly(file, header, readers); } } },
        { texture_for_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_texture_for_poly(file, header, readers); } } },
        { gourand_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_gouraud_poly(file, header, readers); } } },
        { alias_gourand_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_gouraud_poly(file, header, readers); } } },
        { solid_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_solid_poly(file, header, readers); } } },
        { alias_solid_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_solid_poly(file, header, readers); } } },
        { shaded_polygon_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_shaded_poly(file, header, readers); } } },
        { alias_shaded_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_shaded_poly(file, header, readers); } } },
        { an_shape_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_an_shape(file, header, readers); } } },
        { an_anim_list_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_an_anim_list(file, header, readers); } } },
        { an_sequence_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_an_sequence(file, header, readers); } } },
        { an_cyclic_sequence_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_an_cyclic_sequence(file, header, readers); } } },
      };

      return read_children<shape_item>(file, 1, readers);
    }
  }// namespace v1

  namespace v2
  {
    constexpr file_tag material_list_tag = shared::to_tag<4>({ 0x1e, 0x00, 0x14, 0x00 });
    constexpr file_tag material_tag = shared::to_tag<4>({ 0x1f, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_part_tag = shared::to_tag<4>({ 0x46, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_bsp_part_tag = shared::to_tag<4>({ 0x47, 0x00, 0x14, 0x00 });
    constexpr file_tag cell_anim_part_tag = shared::to_tag<4>({ 0x51, 0x00, 0x14, 0x00 });
    constexpr file_tag detail_part_tag = shared::to_tag<4>({ 0x5a, 0x00, 0x14, 0x00 });
    constexpr file_tag mesh_tag = shared::to_tag<4>({ 0x28, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_mesh_tag = shared::to_tag<4>({ 0x2b, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_anim_mesh_tag = shared::to_tag<4>({ 0x2a, 0x00, 0x14, 0x00 });
    constexpr file_tag part_list_tag = shared::to_tag<4>({ 0x3c, 0x00, 0x14, 0x00 });
    constexpr file_tag shape_tag = shared::to_tag<4>({ 0x64, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_shape_tag = shared::to_tag<4>({ 0x65, 0x00, 0x14, 0x00 });
    constexpr file_tag null_part_tag = shared::to_tag<4>({ 0x78, 0x00, 0x14, 0x00 });
    constexpr file_tag bitmap_frame_tag = shared::to_tag<4>({ 0x6e, 0x00, 0x14, 0x00 });

    std::vector<shape_item> read_shapes(std::basic_istream<std::byte>& file)
    {
      static shape_reader_map readers = {};

      return read_children<shape_item>(file, 1, readers);
    }

    base_part read_base_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map&)
    {
      base_part base{};
      file.read(reinterpret_cast<std::byte*>(&base.transform), sizeof(base.transform));
      file.read(reinterpret_cast<std::byte*>(&base.id_number), sizeof(base.id_number));
      file.read(reinterpret_cast<std::byte*>(&base.radius), sizeof(base.radius));
      file.read(reinterpret_cast<std::byte*>(&base.center), sizeof(base.center));

      return base;
    }

    null_part read_null_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      null_part shape{};
      shape.base = read_base_part(file, header, readers);

      return shape;
    }

    part_list read_part_list(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      part_list parts{};
      parts.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&parts.part_count), sizeof(parts.part_count));
      parts.parts = read_children<shape_item>(file, parts.part_count, readers);
      return parts;
    }

    material read_material(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      material shape{};

      file.read(reinterpret_cast<std::byte*>(&shape.null), sizeof(shape.null));
      file.read(reinterpret_cast<std::byte*>(&shape.unknown0), sizeof(shape.unknown0));
      file.read(reinterpret_cast<std::byte*>(&shape.material_type), sizeof(shape.material_type));
      file.read(reinterpret_cast<std::byte*>(&shape.unknown1), sizeof(shape.unknown1));
      file.read(reinterpret_cast<std::byte*>(&shape.palette), sizeof(shape.palette));
      file.read(reinterpret_cast<std::byte*>(&shape.rgb), sizeof(shape.rgb));
      file.read(reinterpret_cast<std::byte*>(&shape.flags), sizeof(shape.flags));
      shape.texture = read_string(file);

      return shape;
    }

    material_list read_material_list(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      material_list shape{};

      file.read(reinterpret_cast<std::byte*>(&shape.material_count), sizeof(shape.material_count));
      file.read(reinterpret_cast<std::byte*>(&shape.detail_count), sizeof(shape.detail_count));

      const auto total_materials = shape.material_count * shape.detail_count;

      for (auto i = 0; i < total_materials; ++i)
      {
        object_header child_header;
        file.read(reinterpret_cast<std::byte*>(&child_header), sizeof(child_header));
        shape.materials.emplace_back(read_material(file, child_header, readers));
      }

      return shape;
    }

    mesh read_mesh(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      mesh shape{};
      shape.base = read_base_part(file, header, readers);

      file.read(reinterpret_cast<std::byte*>(&shape.vertex_count), sizeof(shape.vertex_count));
      file.read(reinterpret_cast<std::byte*>(&shape.texture_vertex_count), sizeof(shape.texture_vertex_count));
      file.read(reinterpret_cast<std::byte*>(&shape.face_count), sizeof(shape.face_count));

      shape.vertices = std::vector<mesh::vertex>(shape.vertex_count);
      file.read(reinterpret_cast<std::byte*>(shape.vertices.data()), sizeof(mesh::vertex) * shape.vertex_count);

      using tx_vertex = decltype(shape.texture_vertices)::value_type;
      shape.texture_vertices = std::vector<tx_vertex>(shape.texture_vertex_count);
      file.read(reinterpret_cast<std::byte*>(shape.texture_vertices.data()), sizeof(tx_vertex) * shape.texture_vertex_count);

      shape.faces = std::vector<mesh::face>(shape.face_count);
      file.read(reinterpret_cast<std::byte*>(shape.faces.data()), sizeof(mesh::face) * shape.face_count);

      return shape;
    }

    cell_anim_part read_cell_anim_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      cell_anim_part shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<std::byte*>(&shape.anim_sequence), sizeof(shape.anim_sequence));
      file.read(reinterpret_cast<std::byte*>(&shape.cell_count), sizeof(shape.cell_count));

      file.read(reinterpret_cast<std::byte*>(&shape.cells), sizeof(shape.cell_count));
      shape.cells = std::vector<endian::little_int32_t>(shape.cell_count);
      file.read(reinterpret_cast<std::byte*>(shape.cells.data()), sizeof(endian::little_int32_t) * shape.cell_count);

      return shape;
    }

    bitmap_frame_part read_bitmap_frame_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bitmap_frame_part shape{};
      shape.base = read_base_part(file, header, readers);

      file.read(reinterpret_cast<std::byte*>(&shape.bmp_tag), sizeof(shape.bmp_tag));
      file.read(reinterpret_cast<std::byte*>(&shape.x_offset), sizeof(shape.x_offset));
      file.read(reinterpret_cast<std::byte*>(&shape.y_offset), sizeof(shape.y_offset));
      file.read(reinterpret_cast<std::byte*>(&shape.rotation), sizeof(shape.rotation));
      file.read(reinterpret_cast<std::byte*>(&shape.squish), sizeof(shape.squish));

      return shape;
    }

    detail_part read_detail_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      detail_part shape{};
      shape.base = read_part_list(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.detail_count), sizeof(shape.detail_count));

      shape.details = std::vector<endian::little_int32_t>(shape.detail_count);
      file.read(reinterpret_cast<std::byte*>(shape.details.data()), sizeof(endian::little_int32_t) * shape.detail_count);

      return shape;
    }

    bsp_part read_bsp_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bsp_part shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<std::byte*>(&shape.node_count), sizeof(shape.node_count));

      shape.nodes = std::vector<bsp_part::bsp_node>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.nodes.data()), sizeof(bsp_part::bsp_node) * shape.node_count);

      shape.transforms = std::vector<endian::little_int32_t>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.transforms.data()), sizeof(endian::little_int32_t) * shape.node_count);

      return shape;
    }

    shape read_shape(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      shape shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<std::byte*>(&shape.sequence_count), sizeof(shape.sequence_count));
      shape.sequences = std::vector<endian::little_int32_t>(shape.sequence_count);
      file.read(reinterpret_cast<std::byte*>(shape.sequences.data()), sizeof(endian::little_int32_t) * shape.sequence_count);
      return shape;
    }
  }// namespace v2
}// namespace studio::content::dts::three_space