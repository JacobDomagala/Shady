#include "gltf_loader.hpp"
#include "utils/assert.hpp"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <nlohmann/json.hpp>
#include <numeric>
#include <optional>
#include <string_view>
#include <system_error>

namespace shady::scene {
namespace {

void Require(bool valid, std::string_view message) {
  if (!valid)
    utils::Assert(false, "glTF: " + std::string(message));
}

template <class T>
const T &At(const std::vector<T> &values, int index, std::string_view label) {
  Require(index >= 0 && static_cast<size_t>(index) < values.size(),
          std::string(label) + " index out of range: " + std::to_string(index));
  return values[static_cast<size_t>(index)];
}

size_t Product(size_t a, size_t b) {
  Require(b == 0 || a <= (std::numeric_limits<size_t>::max)() / b,
          "size overflow");
  return a * b;
}

template <class T> T Read(const unsigned char *bytes) {
  static_assert(std::endian::native == std::endian::little,
                "glTF requires little-endian decoding");
  T value;
  std::memcpy(&value, bytes, sizeof(T));
  return value;
}

uint32_t ReadIndex(const unsigned char *bytes, int type) {
  switch (type) {
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    return Read<uint8_t>(bytes);
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    return Read<uint16_t>(bytes);
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    return Read<uint32_t>(bytes);
  default:
    Require(false, "indices must be unsigned byte, short or int");
    return 0;
  }
}

// tinygltf 2.9.0 forms embedded-image pointers before validating their
// bufferView ranges. Check the container and JSON ranges before giving it any
// bytes to decode.
std::vector<unsigned char> CheckedFile(const std::string &path, bool binary) {
  std::ifstream file(std::filesystem::path(path),
                     std::ios::binary | std::ios::ate);
  Require(file.is_open(), "cannot open file");
  const auto length = file.tellg();
  Require(length > 0 && static_cast<uint64_t>(length) <= UINT32_MAX,
          "empty file or file exceeds 4 GiB limit");
  std::vector<unsigned char> bytes(static_cast<size_t>(length));
  file.seekg(0);
  file.read(reinterpret_cast<char *>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
  Require(static_cast<bool>(file), "file read failed");
  size_t jsonOffset = 0, jsonLength = bytes.size();
  if (binary) {
    Require(bytes.size() >= 20 && Read<uint32_t>(bytes.data()) == 0x46546c67,
            "invalid GLB header/magic");
    Require(Read<uint32_t>(bytes.data() + 4) == 2, "unsupported GLB version");
    Require(Read<uint32_t>(bytes.data() + 8) == bytes.size(),
            "GLB length does not match file size");
    bool hasBinary = false;
    size_t chunkIndex = 0;
    for (size_t offset = 12; offset < bytes.size(); ++chunkIndex) {
      Require(bytes.size() - offset >= 8, "truncated GLB chunk header");
      const auto size = Read<uint32_t>(bytes.data() + offset);
      const auto type = Read<uint32_t>(bytes.data() + offset + 4);
      Require(size % 4 == 0 && size <= bytes.size() - offset - 8,
              "invalid GLB chunk length/alignment");
      if (chunkIndex == 0) {
        Require(type == 0x4e4f534a && size > 0, "GLB first chunk must be JSON");
        jsonOffset = offset + 8;
        jsonLength = size;
      } else
        Require(type != 0x4e4f534a, "duplicate GLB JSON chunk");
      if (type == 0x004e4942) {
        Require(chunkIndex == 1 && !hasBinary,
                "GLB BIN chunk must be second and unique");
        hasBinary = true;
      }
      offset += 8 + size;
    }
    // Unknown chunks must be ignored. Tinygltf expects chunk 2 to be BIN;
    // omit unknown-only trailing chunks when there is no BIN chunk.
    if (!hasBinary && bytes.size() > jsonOffset + jsonLength) {
      bytes.resize(jsonOffset + jsonLength);
      const auto size = static_cast<uint32_t>(bytes.size());
      std::memcpy(bytes.data() + 8, &size, sizeof(size));
    }
  }
  const auto json = nlohmann::json::parse(
      bytes.begin() + static_cast<std::ptrdiff_t>(jsonOffset),
      bytes.begin() + static_cast<std::ptrdiff_t>(jsonOffset + jsonLength),
      nullptr, false);
  Require(!json.is_discarded(), "invalid JSON in " + path);
  Require(json.is_object(), "document must be an object");
  auto member = [](const nlohmann::json &object,
                   const char *key) -> const nlohmann::json & {
    Require(object.is_object(), "expected a JSON object");
    const auto it = object.find(key);
    Require(it != object.end(), std::string("missing JSON member: ") + key);
    return *it;
  };
  auto unsignedValue = [](const nlohmann::json &value) -> size_t {
    Require(value.is_number_unsigned(),
            "byte sizes and indices must be non-negative integers");
    const auto number = value.get<uint64_t>();
    Require(number <= (std::numeric_limits<size_t>::max)(),
            "byte size overflow");
    return static_cast<size_t>(number);
  };
  std::vector<size_t> bufferSizes;
  if (json.contains("buffers")) {
    Require(json["buffers"].is_array(), "buffers must be an array");
    for (const auto &buffer : json["buffers"]) {
      const auto size = unsignedValue(member(buffer, "byteLength"));
      Require(size > 0, "buffer byteLength must be positive");
      bufferSizes.push_back(size);
    }
  }
  if (json.contains("bufferViews")) {
    Require(json["bufferViews"].is_array(), "bufferViews must be an array");
    for (const auto &view : json["bufferViews"]) {
      const auto index = unsignedValue(member(view, "buffer"));
      const auto offset =
          view.contains("byteOffset") ? unsignedValue(view["byteOffset"]) : 0;
      const auto size = unsignedValue(member(view, "byteLength"));
      Require(index < bufferSizes.size() && size > 0 &&
                  offset <= bufferSizes[index] &&
                  size <= bufferSizes[index] - offset,
              "bufferView exceeds declared buffer");
    }
  }
  if (json.contains("images")) {
    Require(json["images"].is_array(), "images must be an array");
    for (const auto &image : json["images"]) {
      Require(image.is_object(), "image must be a JSON object");
      if (!image.contains("bufferView"))
        continue;
      const auto index = unsignedValue(image["bufferView"]);
      Require(json.contains("bufferViews") &&
                  index < json["bufferViews"].size(),
              "image bufferView index out of range");
      Require(unsignedValue(json["bufferViews"][index]["byteLength"]) <=
                  static_cast<size_t>(INT32_MAX),
              "encoded image exceeds decoder size limit");
    }
  }
  return bytes;
}

// A checked, tightly packed accessor. Sparse values overlay the base or
// implicit zeros.
class Accessor {
public:
  Accessor(const tinygltf::Model &model, int index)
      : info(At(model.accessors, index, "accessor")) {
    Require(
        info.type == TINYGLTF_TYPE_SCALAR || info.type == TINYGLTF_TYPE_VEC2 ||
            info.type == TINYGLTF_TYPE_VEC3 || info.type == TINYGLTF_TYPE_VEC4,
        "unsupported accessor shape");
    const int size = tinygltf::GetComponentSizeInBytes(
        static_cast<uint32_t>(info.componentType));
    Require(size > 0 && info.componentType != TINYGLTF_COMPONENT_TYPE_DOUBLE,
            "invalid component type");
    componentSize = static_cast<size_t>(size);
    components = static_cast<size_t>(
        tinygltf::GetNumComponentsInType(static_cast<uint32_t>(info.type)));
    elementSize = Product(componentSize, components);
    Require(info.count > 0 &&
                info.count <=
                    static_cast<size_t>((std::numeric_limits<int32_t>::max)()),
            "accessor count exceeds engine limits or is zero");
    Require(!info.normalized ||
                info.componentType == TINYGLTF_COMPONENT_TYPE_BYTE ||
                info.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ||
                info.componentType == TINYGLTF_COMPONENT_TYPE_SHORT ||
                info.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,
            "invalid normalized component type");
    if (info.bufferView >= 0) {
      const auto &view = At(model.bufferViews, info.bufferView, "bufferView");
      const auto stride = view.byteStride ? view.byteStride : elementSize;
      Require(!view.byteStride ||
                  (stride >= 4 && stride <= 252 && stride % 4 == 0),
              "invalid byteStride");
      const auto *base = Range(model, info.bufferView, info.byteOffset,
                               info.count, stride, elementSize, componentSize);
      bytes.resize(Product(info.count, elementSize));
      for (size_t i = 0; i < info.count; ++i)
        std::memcpy(bytes.data() + i * elementSize, base + i * stride,
                    elementSize);
    } else {
      Require(info.bufferView == -1 && info.byteOffset == 0,
              "invalid accessor without bufferView");
      // Bound zero-initialized accessors before allocating, including
      // sparse-only data.
      Require(Product(info.count, elementSize) <= size_t{512} * 1024 * 1024,
              "zero-filled accessor exceeds 512 MiB limit");
      bytes.resize(Product(info.count, elementSize), 0);
    }
    if (info.sparse.isSparse) {
      const auto &sparse = info.sparse;
      Require(sparse.count > 0 &&
                  static_cast<size_t>(sparse.count) <= info.count,
              "invalid sparse count");
      Require(sparse.indices.componentType ==
                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ||
                  sparse.indices.componentType ==
                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ||
                  sparse.indices.componentType ==
                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,
              "invalid sparse index type");
      const auto indexSize =
          static_cast<size_t>(tinygltf::GetComponentSizeInBytes(
              static_cast<uint32_t>(sparse.indices.componentType)));
      Require(
          At(model.bufferViews, sparse.indices.bufferView, "sparse indices")
                      .byteStride == 0 &&
              At(model.bufferViews, sparse.values.bufferView, "sparse values")
                      .byteStride == 0,
          "sparse bufferViews must be tightly packed");
      const auto count = static_cast<size_t>(sparse.count);
      const auto *indices =
          Range(model, sparse.indices.bufferView, sparse.indices.byteOffset,
                count, indexSize, indexSize, indexSize);
      const auto *values =
          Range(model, sparse.values.bufferView, sparse.values.byteOffset,
                count, elementSize, elementSize, componentSize);
      uint32_t previous = 0;
      for (size_t i = 0; i < count; ++i) {
        const auto destination =
            ReadIndex(indices + i * indexSize, sparse.indices.componentType);
        Require(destination < info.count && (i == 0 || destination > previous),
                "sparse indices must be ordered, unique and in range");
        std::memcpy(bytes.data() + destination * elementSize,
                    values + i * elementSize, elementSize);
        previous = destination;
      }
    }
  }

  glm::vec4 Vector(size_t index) const {
    Require(index < info.count, "vertex attribute count mismatch");
    glm::vec4 result(0.0F);
    for (size_t channel = 0; channel < components; ++channel) {
      const auto *ptr =
          bytes.data() + index * elementSize + channel * componentSize;
      float value = 0;
      switch (info.componentType) {
      case TINYGLTF_COMPONENT_TYPE_FLOAT:
        value = Read<float>(ptr);
        break;
      case TINYGLTF_COMPONENT_TYPE_BYTE:
        value = static_cast<float>(Read<int8_t>(ptr));
        if (info.normalized)
          value = std::max(value / 127.0F, -1.0F);
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        value = static_cast<float>(Read<uint8_t>(ptr));
        if (info.normalized)
          value /= 255.0F;
        break;
      case TINYGLTF_COMPONENT_TYPE_SHORT:
        value = static_cast<float>(Read<int16_t>(ptr));
        if (info.normalized)
          value = std::max(value / 32767.0F, -1.0F);
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        value = static_cast<float>(Read<uint16_t>(ptr));
        if (info.normalized)
          value /= 65535.0F;
        break;
      default:
        Require(false, "unsupported vertex component type");
        break;
      }
      Require(std::isfinite(value), "non-finite vertex attribute");
      result[static_cast<glm::length_t>(channel)] = value;
    }
    return result;
  }

  uint32_t Index(size_t index) const {
    Require(info.type == TINYGLTF_TYPE_SCALAR && !info.normalized,
            "invalid indices accessor");
    const auto value =
        ReadIndex(bytes.data() + index * elementSize, info.componentType);
    const uint32_t maximum = componentSize == 1   ? 255U
                             : componentSize == 2 ? 65535U
                                                  : UINT32_MAX;
    Require(value != maximum, "primitive restart index is forbidden");
    return value;
  }

  tinygltf::Accessor info;

private:
  static const unsigned char *Range(const tinygltf::Model &model, int viewIndex,
                                    size_t offset, size_t count, size_t stride,
                                    size_t size, size_t alignment) {
    const auto &view = At(model.bufferViews, viewIndex, "bufferView");
    const auto &buffer = At(model.buffers, view.buffer, "buffer").data;
    Require(view.byteOffset <= buffer.size() &&
                view.byteLength <= buffer.size() - view.byteOffset,
            "bufferView exceeds buffer");
    Require(stride >= size && stride % alignment == 0 &&
                offset % alignment == 0 && view.byteOffset % alignment == 0,
            "invalid accessor alignment or stride");
    Require(offset <= view.byteLength && size <= view.byteLength - offset,
            "accessor exceeds bufferView");
    Require(count > 0 &&
                count - 1 <= (view.byteLength - offset - size) / stride,
            "accessor range exceeds bufferView");
    return buffer.data() + view.byteOffset + offset;
  }

  size_t componentSize = 0;
  size_t components = 0;
  size_t elementSize = 0;
  std::vector<unsigned char> bytes;
};

glm::vec3 Unit(glm::vec3 vector, glm::vec3 fallback = glm::vec3(0, 1, 0)) {
  const auto length = glm::length(vector);
  return std::isfinite(length) && length > 1e-20F ? vector / length : fallback;
}

glm::mat4 LocalMatrix(const tinygltf::Node &node) {
  auto valid = [](const std::vector<double> &values, size_t size) {
    return values.empty() ||
           (values.size() == size && std::ranges::all_of(values, [](double v) {
              return std::isfinite(v) &&
                     std::abs(v) <= static_cast<double>(
                                        (std::numeric_limits<float>::max)());
            }));
  };
  Require(valid(node.matrix, 16) && valid(node.translation, 3) &&
              valid(node.rotation, 4) && valid(node.scale, 3),
          "invalid node transform");
  glm::mat4 matrix(1);
  if (!node.matrix.empty()) {
    Require(node.translation.empty() && node.rotation.empty() &&
                node.scale.empty(),
            "node mixes matrix and TRS");
    for (size_t i = 0; i < 16; ++i)
      matrix[static_cast<int>(i / 4)][static_cast<int>(i % 4)] =
          static_cast<float>(node.matrix[i]);
    Require(matrix[0][3] == 0 && matrix[1][3] == 0 && matrix[2][3] == 0 &&
                matrix[3][3] == 1,
            "node matrix is not affine");
    return matrix;
  }
  if (!node.translation.empty())
    matrix = glm::translate(matrix,
                            glm::vec3(node.translation[0], node.translation[1],
                                      node.translation[2]));
  if (!node.rotation.empty()) {
    glm::quat q(static_cast<float>(node.rotation[3]),
                static_cast<float>(node.rotation[0]),
                static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2]));
    Require(std::abs(glm::length(q) - 1.0F) < 0.001F,
            "node quaternion is not normalized");
    matrix *= glm::mat4_cast(glm::normalize(q));
  }
  if (!node.scale.empty())
    matrix = glm::scale(matrix,
                        glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
  return matrix;
}

struct TextureBinding {
  int image = -1;
  int texCoord = 0;
  glm::vec2 offset{0};
  glm::vec2 scale{1};
  float rotation = 0;
};

template <class Info>
TextureBinding Binding(const tinygltf::Model &model, const Info &info) {
  TextureBinding result;
  if (info.index == -1)
    return result;
  const auto &texture = At(model.textures, info.index, "texture");
  At(model.images, texture.source, "image");
  result.image = texture.source;
  result.texCoord = info.texCoord;
  if (texture.sampler != -1)
    At(model.samplers, texture.sampler, "sampler");
  if (auto it = info.extensions.find("KHR_texture_transform");
      it != info.extensions.end()) {
    const auto &transform = it->second;
    Require(transform.IsObject(), "invalid texture transform");
    auto number = [](const tinygltf::Value &value) {
      Require(value.IsNumber(), "invalid texture transform number");
      const auto resultValue = static_cast<float>(value.GetNumberAsDouble());
      Require(std::isfinite(resultValue), "non-finite texture transform");
      return resultValue;
    };
    auto vec = [&](const char *key, glm::vec2 fallback) {
      if (!transform.Has(key))
        return fallback;
      const auto &value = transform.Get(key);
      Require(value.IsArray() && value.ArrayLen() == 2,
              "invalid texture transform vector");
      return glm::vec2(number(value.Get(0)), number(value.Get(1)));
    };
    result.offset = vec("offset", result.offset);
    result.scale = vec("scale", result.scale);
    if (transform.Has("rotation"))
      result.rotation = number(transform.Get("rotation"));
    if (transform.Has("texCoord")) {
      Require(transform.Get("texCoord").IsInt(),
              "invalid texture coordinate set");
      result.texCoord = transform.Get("texCoord").GetNumberAsInt();
    }
  }
  Require(result.texCoord >= 0, "negative texture coordinate set");
  return result;
}

void Primitive(GltfAsset &asset, const tinygltf::Primitive &primitive,
               const glm::mat4 &world, const std::string &name) {
  const auto &model = asset.source;
  Require(primitive.mode >= -1 && primitive.mode <= TINYGLTF_MODE_TRIANGLE_FAN,
          "invalid primitive mode");
  const auto mode =
      primitive.mode == -1 ? TINYGLTF_MODE_TRIANGLES : primitive.mode;
  if (mode != TINYGLTF_MODE_TRIANGLES && mode != TINYGLTF_MODE_TRIANGLE_STRIP &&
      mode != TINYGLTF_MODE_TRIANGLE_FAN) {
    asset.warnings.push_back("Skipping non-triangle primitive in " + name);
    return;
  }
  if (!primitive.attributes.contains("POSITION")) {
    asset.warnings.push_back("Skipping primitive without POSITION in " + name);
    return;
  }
  Require(primitive.targets.empty(),
          "morph targets are not supported: " + name);
  GltfMesh mesh;
  mesh.name = name;
  std::array<TextureBinding, 3> bindings;
  bool doubleSided = false;
  if (primitive.material != -1) {
    const auto &material = At(model.materials, primitive.material, "material");
    if (material.alphaMode == "BLEND") {
      asset.warnings.push_back(
          "Skipping BLEND material (deferred transparency unsupported): " +
          material.name);
      return;
    }
    Require(material.alphaMode == "OPAQUE" || material.alphaMode == "MASK",
            "invalid alpha mode");
    const auto &pbr = material.pbrMetallicRoughness;
    Require(pbr.baseColorFactor.size() == 4,
            "baseColorFactor must have four components");
    for (int i = 0; i < 4; ++i) {
      const auto value = pbr.baseColorFactor[static_cast<size_t>(i)];
      Require(std::isfinite(value) && value >= 0 && value <= 1,
              "invalid baseColorFactor");
      mesh.material.baseColorFactor[i] = static_cast<float>(value);
    }
    Require(std::isfinite(pbr.metallicFactor) && pbr.metallicFactor >= 0 &&
                pbr.metallicFactor <= 1 && std::isfinite(pbr.roughnessFactor) &&
                pbr.roughnessFactor >= 0 && pbr.roughnessFactor <= 1 &&
                std::isfinite(material.normalTexture.scale) &&
                std::isfinite(material.alphaCutoff) &&
                material.alphaCutoff >= 0,
            "invalid material factors");
    mesh.material.metallicFactor = static_cast<float>(pbr.metallicFactor);
    mesh.material.roughnessFactor = static_cast<float>(pbr.roughnessFactor);
    mesh.material.normalScale =
        static_cast<float>(material.normalTexture.scale);
    mesh.material.alphaCutoff = material.alphaMode == "MASK"
                                    ? static_cast<float>(material.alphaCutoff)
                                    : -1.0F;
    doubleSided = material.doubleSided;
    bindings = {Binding(model, pbr.baseColorTexture),
                Binding(model, pbr.metallicRoughnessTexture),
                Binding(model, material.normalTexture)};
  }
  Accessor positions(model, primitive.attributes.find("POSITION")->second);
  Require(positions.info.type == TINYGLTF_TYPE_VEC3 &&
              positions.info.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT &&
              !positions.info.normalized,
          "POSITION must be float VEC3");
  auto attribute = [&](const std::string &semantic, int type,
                       bool normalizedAllowed) -> std::optional<Accessor> {
    auto it = primitive.attributes.find(semantic);
    if (it == primitive.attributes.end())
      return std::nullopt;
    Accessor accessor(model, it->second);
    Require(accessor.info.count == positions.info.count &&
                accessor.info.type == type,
            "invalid " + semantic + " shape/count");
    Require((accessor.info.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT &&
             !accessor.info.normalized) ||
                (normalizedAllowed && accessor.info.normalized &&
                 (accessor.info.componentType ==
                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ||
                  accessor.info.componentType ==
                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)),
            "unsupported " + semantic + " component format");
    return accessor;
  };
  const auto normals = attribute("NORMAL", TINYGLTF_TYPE_VEC3, false);
  const auto tangents = attribute("TANGENT", TINYGLTF_TYPE_VEC4, false);
  std::optional<Accessor> colors;
  if (auto it = primitive.attributes.find("COLOR_0");
      it != primitive.attributes.end()) {
    const auto type = At(model.accessors, it->second, "color").type;
    Require(type == TINYGLTF_TYPE_VEC3 || type == TINYGLTF_TYPE_VEC4,
            "invalid COLOR_0 shape");
    colors = attribute("COLOR_0", type, true);
  }
  std::array<std::optional<Accessor>, 3> uvs;
  for (size_t slot = 0; slot < bindings.size(); ++slot) {
    mesh.images[slot] = bindings[slot].image;
    if (bindings[slot].image >= 0) {
      uvs[slot] =
          attribute("TEXCOORD_" + std::to_string(bindings[slot].texCoord),
                    TINYGLTF_TYPE_VEC2, true);
      Require(uvs[slot].has_value(),
              "texture references missing TEXCOORD attribute");
    }
  }
  // Retain UV0 even on untextured primitives for inspection and later material
  // edits.
  if (!uvs[0])
    uvs[0] = attribute("TEXCOORD_0", TINYGLTF_TYPE_VEC2, true);
  const glm::mat3 tangentMatrix(world);
  const auto determinant = glm::determinant(tangentMatrix);
  Require(std::isfinite(determinant), "non-finite world transform");
  if (determinant == 0) {
    asset.warnings.push_back("Skipping zero-scale mesh: " + name);
    return;
  }
  const auto normalMatrix = glm::transpose(glm::inverse(tangentMatrix));
  mesh.vertices.resize(positions.info.count);
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto &vertex = mesh.vertices[i];
    vertex.m_position =
        glm::vec3(world * glm::vec4(glm::vec3(positions.Vector(i)), 1));
    Require(std::isfinite(vertex.m_position.x) &&
                std::isfinite(vertex.m_position.y) &&
                std::isfinite(vertex.m_position.z),
            "non-finite transformed position");
    vertex.m_normal = normals
                          ? Unit(normalMatrix * glm::vec3(normals->Vector(i)))
                          : glm::vec3(0);
    vertex.m_tangent = glm::vec4(0);
    if (normals && tangents) {
      const auto tangent = tangents->Vector(i);
      Require(tangent.w == 1 || tangent.w == -1, "invalid tangent handedness");
      const auto transformed = tangentMatrix * glm::vec3(tangent);
      vertex.m_tangent = glm::vec4(
          Unit(transformed -
                   vertex.m_normal * glm::dot(vertex.m_normal, transformed),
               glm::vec3(0)),
          tangent.w * (determinant < 0 ? -1.0F : 1.0F));
    }
    // Transformed or alternate normal UVs need a tangent frame derived in the
    // shader.
    if (bindings[2].texCoord != 0 || bindings[2].rotation != 0 ||
        bindings[2].scale != glm::vec2(1))
      vertex.m_tangent = glm::vec4(0);
    std::array<glm::vec2 *, 3> destinations = {&vertex.m_texCoords,
                                               &vertex.m_materialTexCoords,
                                               &vertex.m_normalTexCoords};
    for (size_t slot = 0; slot < bindings.size(); ++slot) {
      const auto uv =
          (uvs[slot] ? glm::vec2(uvs[slot]->Vector(i)) : glm::vec2(0)) *
          bindings[slot].scale;
      const auto c = std::cos(bindings[slot].rotation);
      const auto s = std::sin(bindings[slot].rotation);
      *destinations[slot] =
          glm::vec2(c * uv.x - s * uv.y, s * uv.x + c * uv.y) +
          bindings[slot].offset;
    }
    if (colors) {
      vertex.m_color = colors->Vector(i);
      if (colors->info.type == TINYGLTF_TYPE_VEC3)
        vertex.m_color.a = 1;
      for (int c = 0; c < 4; ++c)
        Require(vertex.m_color[c] >= 0 && vertex.m_color[c] <= 1,
                "COLOR_0 outside [0, 1]");
    }
  }
  std::vector<uint32_t> sourceIndices;
  if (primitive.indices != -1) {
    Accessor indices(model, primitive.indices);
    if (indices.info.bufferView >= 0)
      Require(At(model.bufferViews, indices.info.bufferView, "indices")
                      .byteStride == 0,
              "indices must be tightly packed");
    sourceIndices.resize(indices.info.count);
    for (size_t i = 0; i < sourceIndices.size(); ++i) {
      sourceIndices[i] = indices.Index(i);
      Require(sourceIndices[i] < mesh.vertices.size(),
              "index exceeds vertex count");
    }
  } else {
    sourceIndices.resize(mesh.vertices.size());
    std::iota(sourceIndices.begin(), sourceIndices.end(), uint32_t{0});
  }
  Require(sourceIndices.size() >= 3 && (mode != TINYGLTF_MODE_TRIANGLES ||
                                        sourceIndices.size() % 3 == 0),
          "invalid triangle index count");
  if (mode == TINYGLTF_MODE_TRIANGLES)
    mesh.indices = std::move(sourceIndices);
  else {
    for (size_t i = 2; i < sourceIndices.size(); ++i) {
      if (mode == TINYGLTF_MODE_TRIANGLE_FAN)
        mesh.indices.insert(
            mesh.indices.end(),
            {sourceIndices[0], sourceIndices[i - 1], sourceIndices[i]});
      else if (i % 2 == 0)
        mesh.indices.insert(
            mesh.indices.end(),
            {sourceIndices[i - 2], sourceIndices[i - 1], sourceIndices[i]});
      else
        mesh.indices.insert(
            mesh.indices.end(),
            {sourceIndices[i - 1], sourceIndices[i - 2], sourceIndices[i]});
    }
  }
  if (determinant < 0)
    for (size_t i = 0; i < mesh.indices.size(); i += 3)
      std::swap(mesh.indices[i + 1], mesh.indices[i + 2]);
  if (!normals) {
    std::vector<render::Vertex> flat;
    flat.reserve(mesh.indices.size());
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
      const auto &a = mesh.vertices[mesh.indices[i]];
      const auto &b = mesh.vertices[mesh.indices[i + 1]];
      const auto &c = mesh.vertices[mesh.indices[i + 2]];
      const auto normal = Unit(
          glm::cross(b.m_position - a.m_position, c.m_position - a.m_position));
      for (size_t j = 0; j < 3; ++j) {
        auto vertex = mesh.vertices[mesh.indices[i + j]];
        vertex.m_normal = normal;
        vertex.m_tangent = glm::vec4(0);
        flat.push_back(vertex);
      }
    }
    mesh.vertices = std::move(flat);
    std::iota(mesh.indices.begin(), mesh.indices.end(), uint32_t{0});
  }
  if (doubleSided) {
    // The renderer batches draws with fixed back-face culling. Supply reversed
    // faces with reversed normals so both sides have the glTF lighting
    // convention.
    const auto count = mesh.vertices.size();
    Require(count <= static_cast<size_t>(INT32_MAX) / 2,
            "double-sided mesh exceeds engine limits");
    mesh.vertices.reserve(count * 2);
    for (size_t i = 0; i < count; ++i) {
      auto back = mesh.vertices[i];
      back.m_normal = -back.m_normal;
      back.m_tangent.w = -back.m_tangent.w;
      mesh.vertices.push_back(back);
    }
    const auto indexCount = mesh.indices.size();
    mesh.indices.reserve(Product(indexCount, 2));
    for (size_t i = 0; i < indexCount; i += 3)
      mesh.indices.insert(mesh.indices.end(),
                          {mesh.indices[i] + static_cast<uint32_t>(count),
                           mesh.indices[i + 2] + static_cast<uint32_t>(count),
                           mesh.indices[i + 1] + static_cast<uint32_t>(count)});
  }
  Require(mesh.vertices.size() <= static_cast<size_t>(INT32_MAX) &&
              mesh.indices.size() <= UINT32_MAX,
          "mesh exceeds engine limits");
  asset.meshes.push_back(std::move(mesh));
}

} // namespace

GltfAsset ImportGltf(tinygltf::Model model) {
  GltfAsset asset;
  asset.source = std::move(model);
  auto &source = asset.source;
  Require(source.asset.version == "2.0" && (source.asset.minVersion.empty() ||
                                            source.asset.minVersion == "2.0"),
          "unsupported glTF version");
  for (const auto &extension : source.extensionsRequired)
    Require(extension == "KHR_texture_transform",
            "unsupported required extension: " + extension);
  for (const auto &extension : source.extensionsUsed)
    if (extension != "KHR_texture_transform")
      asset.warnings.push_back("Ignoring optional extension: " + extension);
  if (!source.animations.empty())
    asset.warnings.push_back(
        "Animations are not evaluated; loading the static scene");
  for (const auto &material : source.materials) {
    if (material.occlusionTexture.index >= 0 ||
        material.emissiveTexture.index >= 0 ||
        std::ranges::any_of(material.emissiveFactor,
                            [](double v) { return v != 0; })) {
      asset.warnings.push_back("Occlusion/emissive channels are not supported "
                               "by the deferred renderer");
      break;
    }
  }
  for (const auto &sampler : source.samplers)
    if ((sampler.magFilter != -1 &&
         sampler.magFilter != TINYGLTF_TEXTURE_FILTER_LINEAR) ||
        (sampler.minFilter != -1 &&
         sampler.minFilter != TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) ||
        sampler.wrapS != TINYGLTF_TEXTURE_WRAP_REPEAT ||
        sampler.wrapT != TINYGLTF_TEXTURE_WRAP_REPEAT) {
      asset.warnings.push_back("Custom texture samplers are approximated by "
                               "linear mipmapped repeat sampling");
      break;
    }
  for (auto &image : source.images) {
    Require(image.width > 0 && image.height > 0 && image.component == 4 &&
                !image.as_is,
            "image must decode to RGBA (missing or invalid image: " +
                image.uri + ")");
    const auto components = Product(Product(static_cast<size_t>(image.width),
                                            static_cast<size_t>(image.height)),
                                    4);
    if (image.bits == 16 &&
        image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
      Require(image.image.size() == Product(components, 2),
              "invalid 16-bit image size");
      std::vector<unsigned char> converted(components);
      for (size_t i = 0; i < components; ++i)
        converted[i] = static_cast<unsigned char>(
            Read<uint16_t>(image.image.data() + i * 2) >> 8);
      image.image = std::move(converted);
      image.bits = 8;
      image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    }
    Require(image.bits == 8 &&
                image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE &&
                image.image.size() == components,
            "invalid RGBA image size/type");
  }
  // Validate the entire graph, including disconnected cycles, without
  // recursion.
  std::vector<int> parents(source.nodes.size(), -1);
  for (size_t i = 0; i < source.nodes.size(); ++i) {
    for (int child : source.nodes[i].children) {
      At(source.nodes, child, "child node");
      Require(parents[static_cast<size_t>(child)] == -1,
              "node has multiple parents");
      Require(i <= static_cast<size_t>(INT32_MAX), "too many nodes");
      parents[static_cast<size_t>(child)] = static_cast<int>(i);
    }
  }
  std::vector<int> roots;
  for (size_t i = 0; i < parents.size(); ++i)
    if (parents[i] == -1)
      roots.push_back(static_cast<int>(i));
  auto queue = roots;
  for (size_t i = 0; i < queue.size(); ++i) {
    const auto &children = source.nodes[static_cast<size_t>(queue[i])].children;
    queue.insert(queue.end(), children.begin(), children.end());
  }
  Require(queue.size() == source.nodes.size(), "node graph contains a cycle");
  Require(source.defaultScene >= -1, "invalid default scene");
  if (!source.scenes.empty())
    roots = At(source.scenes,
               source.defaultScene == -1 ? 0 : source.defaultScene, "scene")
                .nodes;
  else
    Require(source.defaultScene == -1, "default scene does not exist");
  std::vector<std::pair<int, glm::mat4>> pending;
  for (auto it = roots.rbegin(); it != roots.rend(); ++it) {
    At(source.nodes, *it, "scene root");
    Require(parents[static_cast<size_t>(*it)] == -1, "scene root has a parent");
    pending.emplace_back(*it, glm::mat4(1));
  }
  std::vector<bool> visited(source.nodes.size(), false);
  size_t vertexCount = 0;
  size_t indexCount = 0;
  while (!pending.empty()) {
    const auto [index, parent] = pending.back();
    pending.pop_back();
    Require(!visited[static_cast<size_t>(index)], "duplicate node in scene");
    visited[static_cast<size_t>(index)] = true;
    const auto &node = source.nodes[static_cast<size_t>(index)];
    Require(node.skin == -1, "skinned meshes are not supported: " + node.name);
    const auto world = parent * LocalMatrix(node);
    if (node.mesh != -1) {
      const auto &mesh = At(source.meshes, node.mesh, "mesh");
      for (const auto &primitive : mesh.primitives)
        Primitive(asset, primitive, world, mesh.name);
    }
    for (auto it = node.children.rbegin(); it != node.children.rend(); ++it)
      pending.emplace_back(*it, world);
  }
  for (const auto &mesh : asset.meshes) {
    Require(mesh.vertices.size() <=
                    static_cast<size_t>(INT32_MAX) - vertexCount &&
                mesh.indices.size() <= UINT32_MAX - indexCount,
            "scene exceeds renderer buffer limits");
    vertexCount += mesh.vertices.size();
    indexCount += mesh.indices.size();
  }
  return asset;
}

GltfAsset LoadGltf(const std::string &path) {
  auto extension = std::filesystem::path(path).extension().string();
  std::ranges::transform(extension, extension.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  Require(extension == ".glb" || extension == ".gltf",
          "unsupported model format: " + extension);
  tinygltf::TinyGLTF loader;
  loader.SetPreserveImageChannels(false);
  tinygltf::Model model;
  std::string error, warning;
  const auto bytes = CheckedFile(path, extension == ".glb");
  std::error_code pathError;
  const auto absolutePath = std::filesystem::absolute(path, pathError);
  Require(!pathError, "cannot resolve model path: " + path);
  const auto directory = absolutePath.parent_path().string();
  const auto size = static_cast<unsigned int>(bytes.size());
  const bool loaded =
      extension == ".glb"
          ? loader.LoadBinaryFromMemory(&model, &error, &warning, bytes.data(),
                                        size, directory)
          : loader.LoadASCIIFromString(
                &model, &error, &warning,
                reinterpret_cast<const char *>(bytes.data()), size, directory);
  Require(loaded, "tinygltf load failed: " + error);
  auto asset = ImportGltf(std::move(model));
  if (!warning.empty())
    asset.warnings.insert(asset.warnings.begin(), warning);
  return asset;
}

} // namespace shady::scene
