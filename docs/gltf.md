# glTF / GLB import

Shady imports static glTF 2.0 scenes through the CPU-only `LoadGltf` function.
`Model::LoadModel` uploads the resulting images and meshes only after import has
completed successfully. Invalid assets are reported through `utils::Assert`,
which logs a fatal error and terminates, matching the rest of the engine.

## tinygltf integration

Conan pins `tinygltf/2.9.0`, whose package is header-only. CMake requires its
`TinyGLTF::TinyGLTF` target. `src/scene/tinygltf.cpp` is the sole translation unit
defining `TINYGLTF_IMPLEMENTATION` and the external STB implementations.
The engine and importer targets compile with exceptions disabled.
`TINYGLTF_NOEXCEPTION` and `JSON_NOEXCEPTION` are defined consistently for the
loader and its consumers. JSON parsing uses its non-throwing mode; parse failures
and missing members are checked before access and reported through the engine
assertion path.

`FileManager` deliberately retains its separate **static** STB implementation:
its vertical-flip setting must not change embedded glTF image orientation.
tinygltf explicitly decodes four image channels; 16-bit decoded channels are
converted to the renderer's 8-bit RGBA format. Base-color images use sRGB Vulkan
formats; metallic/roughness and normal images use linear UNORM formats. Cache
keys distinguish an image's color and data uses.

Before calling tinygltf, the importer checks GLB magic, version, exact file
length, chunk order/alignment/ranges, and JSON bufferView ranges. This is also
necessary because tinygltf 2.9.0 forms embedded-image pointers before checking
those ranges. The checked bytes are passed directly to its memory-loading API,
with the model's directory used for relative external resources.

## Supported rendering paths

- Binary GLB and JSON glTF, including external buffers/images and data URIs.
- Embedded PNG/JPEG images, image bufferViews, and untextured meshes.
- Indexed/non-indexed triangles, triangle strips and triangle fans; all three
  unsigned index widths.
- Interleaved/offset accessors, sparse overlays and sparse-only accessors;
  normalized unsigned byte/short UVs and `COLOR_0`.
- Node matrix/TRS transforms, nesting, repeated mesh instances, default-scene
  selection and a root-node fallback when scenes are absent.
- Mirrored winding and tangent handedness, inverse-transpose normals, flat
  normal generation when normals are absent.
- Base color, metallic/roughness, normal textures and material factors;
  per-texture coordinate sets and `KHR_texture_transform`.
- `OPAQUE`, `MASK` (including the shadow pass), vertex color/alpha, and
  double-sided geometry. Double-sided primitives are expanded into front and
  reversed back faces to work with the existing batched culling pipeline.

Missing or transformed normal-map tangents use a derivative-based shader frame.
This is an approximation, not a MikkTSpace implementation. Degenerate UVs leave
the geometric normal unchanged. Zero-scale meshes are skipped with a warning.

## Explicit limitations

This is not a complete glTF renderer or a substitute for the Khronos validator.

- `BLEND` primitives and point/line primitives are skipped with warnings.
- Skins, morph targets, and unsupported **required** extensions are rejected.
  Animations are not evaluated; the static node transforms are imported.
- Optional extensions other than `KHR_texture_transform` are reported and
  ignored. This includes transmission, clearcoat, iridescence, material
  variants and punctual lights. A valid asset can therefore look different
  from a viewer implementing those extensions.
- Occlusion/emissive channels and custom sampler settings are reported as
  unsupported. The current renderer uses linear mipmapped repeat sampling.
- Geometry must fit the renderer's signed vertex offsets and unsigned index
  counts. Files must be smaller than 4 GiB; individual zero-filled accessors
  are limited to 512 MiB. The decoder currently targets little-endian platforms.
