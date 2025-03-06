// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: resource_handle.proto

#include "resource_handle.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
extern PROTOBUF_INTERNAL_EXPORT_resource_5fhandle_2eproto ::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<1> scc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto;
extern PROTOBUF_INTERNAL_EXPORT_tensor_5fshape_2eproto ::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<1> scc_info_TensorShapeProto_tensor_5fshape_2eproto;
namespace tensorboard {
class ResourceHandleProto_DtypeAndShapeDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<ResourceHandleProto_DtypeAndShape> _instance;
} _ResourceHandleProto_DtypeAndShape_default_instance_;
class ResourceHandleProtoDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<ResourceHandleProto> _instance;
} _ResourceHandleProto_default_instance_;
}  // namespace tensorboard
static void InitDefaultsscc_info_ResourceHandleProto_resource_5fhandle_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::tensorboard::_ResourceHandleProto_default_instance_;
    new (ptr) ::tensorboard::ResourceHandleProto();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
  ::tensorboard::ResourceHandleProto::InitAsDefaultInstance();
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<1> scc_info_ResourceHandleProto_resource_5fhandle_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 1, 0, InitDefaultsscc_info_ResourceHandleProto_resource_5fhandle_2eproto}, {
      &scc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto.base,}};

static void InitDefaultsscc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::tensorboard::_ResourceHandleProto_DtypeAndShape_default_instance_;
    new (ptr) ::tensorboard::ResourceHandleProto_DtypeAndShape();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
  ::tensorboard::ResourceHandleProto_DtypeAndShape::InitAsDefaultInstance();
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<1> scc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 1, 0, InitDefaultsscc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto}, {
      &scc_info_TensorShapeProto_tensor_5fshape_2eproto.base,}};

static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_resource_5fhandle_2eproto[2];
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_resource_5fhandle_2eproto = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_resource_5fhandle_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_resource_5fhandle_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto_DtypeAndShape, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto_DtypeAndShape, dtype_),
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto_DtypeAndShape, shape_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto, device_),
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto, container_),
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto, name_),
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto, hash_code_),
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto, maybe_type_name_),
  PROTOBUF_FIELD_OFFSET(::tensorboard::ResourceHandleProto, dtypes_and_shapes_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::tensorboard::ResourceHandleProto_DtypeAndShape)},
  { 7, -1, sizeof(::tensorboard::ResourceHandleProto)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::tensorboard::_ResourceHandleProto_DtypeAndShape_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::tensorboard::_ResourceHandleProto_default_instance_),
};

const char descriptor_table_protodef_resource_5fhandle_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\025resource_handle.proto\022\013tensorboard\032\022te"
  "nsor_shape.proto\032\013types.proto\"\250\002\n\023Resour"
  "ceHandleProto\022\016\n\006device\030\001 \001(\t\022\021\n\tcontain"
  "er\030\002 \001(\t\022\014\n\004name\030\003 \001(\t\022\021\n\thash_code\030\004 \001("
  "\004\022\027\n\017maybe_type_name\030\005 \001(\t\022I\n\021dtypes_and"
  "_shapes\030\006 \003(\0132..tensorboard.ResourceHand"
  "leProto.DtypeAndShape\032c\n\rDtypeAndShape\022$"
  "\n\005dtype\030\001 \001(\0162\025.tensorboard.DataType\022,\n\005"
  "shape\030\002 \001(\0132\035.tensorboard.TensorShapePro"
  "toJ\004\010\007\020\010B\207\001\n\030org.tensorflow.frameworkB\016R"
  "esourceHandleP\001ZVgithub.com/tensorflow/t"
  "ensorflow/tensorflow/go/core/framework/r"
  "esource_handle_go_proto\370\001\001b\006proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_resource_5fhandle_2eproto_deps[2] = {
  &::descriptor_table_tensor_5fshape_2eproto,
  &::descriptor_table_types_2eproto,
};
static ::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase*const descriptor_table_resource_5fhandle_2eproto_sccs[2] = {
  &scc_info_ResourceHandleProto_resource_5fhandle_2eproto.base,
  &scc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto.base,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_resource_5fhandle_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_resource_5fhandle_2eproto = {
  false, false, descriptor_table_protodef_resource_5fhandle_2eproto, "resource_handle.proto", 514,
  &descriptor_table_resource_5fhandle_2eproto_once, descriptor_table_resource_5fhandle_2eproto_sccs, descriptor_table_resource_5fhandle_2eproto_deps, 2, 2,
  schemas, file_default_instances, TableStruct_resource_5fhandle_2eproto::offsets,
  file_level_metadata_resource_5fhandle_2eproto, 2, file_level_enum_descriptors_resource_5fhandle_2eproto, file_level_service_descriptors_resource_5fhandle_2eproto,
};

// Force running AddDescriptors() at dynamic initialization time.
static bool dynamic_init_dummy_resource_5fhandle_2eproto = (static_cast<void>(::PROTOBUF_NAMESPACE_ID::internal::AddDescriptors(&descriptor_table_resource_5fhandle_2eproto)), true);
namespace tensorboard {

// ===================================================================

void ResourceHandleProto_DtypeAndShape::InitAsDefaultInstance() {
  ::tensorboard::_ResourceHandleProto_DtypeAndShape_default_instance_._instance.get_mutable()->shape_ = const_cast< ::tensorboard::TensorShapeProto*>(
      ::tensorboard::TensorShapeProto::internal_default_instance());
}
class ResourceHandleProto_DtypeAndShape::_Internal {
 public:
  static const ::tensorboard::TensorShapeProto& shape(const ResourceHandleProto_DtypeAndShape* msg);
};

const ::tensorboard::TensorShapeProto&
ResourceHandleProto_DtypeAndShape::_Internal::shape(const ResourceHandleProto_DtypeAndShape* msg) {
  return *msg->shape_;
}
void ResourceHandleProto_DtypeAndShape::clear_shape() {
  if (GetArena() == nullptr && shape_ != nullptr) {
    delete shape_;
  }
  shape_ = nullptr;
}
ResourceHandleProto_DtypeAndShape::ResourceHandleProto_DtypeAndShape(::PROTOBUF_NAMESPACE_ID::Arena* arena)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena) {
  SharedCtor();
  RegisterArenaDtor(arena);
  // @@protoc_insertion_point(arena_constructor:tensorboard.ResourceHandleProto.DtypeAndShape)
}
ResourceHandleProto_DtypeAndShape::ResourceHandleProto_DtypeAndShape(const ResourceHandleProto_DtypeAndShape& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  if (from._internal_has_shape()) {
    shape_ = new ::tensorboard::TensorShapeProto(*from.shape_);
  } else {
    shape_ = nullptr;
  }
  dtype_ = from.dtype_;
  // @@protoc_insertion_point(copy_constructor:tensorboard.ResourceHandleProto.DtypeAndShape)
}

void ResourceHandleProto_DtypeAndShape::SharedCtor() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&scc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto.base);
  ::memset(&shape_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&dtype_) -
      reinterpret_cast<char*>(&shape_)) + sizeof(dtype_));
}

ResourceHandleProto_DtypeAndShape::~ResourceHandleProto_DtypeAndShape() {
  // @@protoc_insertion_point(destructor:tensorboard.ResourceHandleProto.DtypeAndShape)
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

void ResourceHandleProto_DtypeAndShape::SharedDtor() {
  GOOGLE_DCHECK(GetArena() == nullptr);
  if (this != internal_default_instance()) delete shape_;
}

void ResourceHandleProto_DtypeAndShape::ArenaDtor(void* object) {
  ResourceHandleProto_DtypeAndShape* _this = reinterpret_cast< ResourceHandleProto_DtypeAndShape* >(object);
  (void)_this;
}
void ResourceHandleProto_DtypeAndShape::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void ResourceHandleProto_DtypeAndShape::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const ResourceHandleProto_DtypeAndShape& ResourceHandleProto_DtypeAndShape::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_ResourceHandleProto_DtypeAndShape_resource_5fhandle_2eproto.base);
  return *internal_default_instance();
}


void ResourceHandleProto_DtypeAndShape::Clear() {
// @@protoc_insertion_point(message_clear_start:tensorboard.ResourceHandleProto.DtypeAndShape)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  if (GetArena() == nullptr && shape_ != nullptr) {
    delete shape_;
  }
  shape_ = nullptr;
  dtype_ = 0;
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* ResourceHandleProto_DtypeAndShape::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  ::PROTOBUF_NAMESPACE_ID::Arena* arena = GetArena(); (void)arena;
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // .tensorboard.DataType dtype = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
          _internal_set_dtype(static_cast<::tensorboard::DataType>(val));
        } else goto handle_unusual;
        continue;
      // .tensorboard.TensorShapeProto shape = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          ptr = ctx->ParseMessage(_internal_mutable_shape(), ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag,
            _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
            ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* ResourceHandleProto_DtypeAndShape::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:tensorboard.ResourceHandleProto.DtypeAndShape)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .tensorboard.DataType dtype = 1;
  if (this->dtype() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      1, this->_internal_dtype(), target);
  }

  // .tensorboard.TensorShapeProto shape = 2;
  if (this->has_shape()) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(
        2, _Internal::shape(this), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:tensorboard.ResourceHandleProto.DtypeAndShape)
  return target;
}

size_t ResourceHandleProto_DtypeAndShape::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:tensorboard.ResourceHandleProto.DtypeAndShape)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // .tensorboard.TensorShapeProto shape = 2;
  if (this->has_shape()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
        *shape_);
  }

  // .tensorboard.DataType dtype = 1;
  if (this->dtype() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->_internal_dtype());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void ResourceHandleProto_DtypeAndShape::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:tensorboard.ResourceHandleProto.DtypeAndShape)
  GOOGLE_DCHECK_NE(&from, this);
  const ResourceHandleProto_DtypeAndShape* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<ResourceHandleProto_DtypeAndShape>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:tensorboard.ResourceHandleProto.DtypeAndShape)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:tensorboard.ResourceHandleProto.DtypeAndShape)
    MergeFrom(*source);
  }
}

void ResourceHandleProto_DtypeAndShape::MergeFrom(const ResourceHandleProto_DtypeAndShape& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:tensorboard.ResourceHandleProto.DtypeAndShape)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.has_shape()) {
    _internal_mutable_shape()->::tensorboard::TensorShapeProto::MergeFrom(from._internal_shape());
  }
  if (from.dtype() != 0) {
    _internal_set_dtype(from._internal_dtype());
  }
}

void ResourceHandleProto_DtypeAndShape::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:tensorboard.ResourceHandleProto.DtypeAndShape)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void ResourceHandleProto_DtypeAndShape::CopyFrom(const ResourceHandleProto_DtypeAndShape& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:tensorboard.ResourceHandleProto.DtypeAndShape)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ResourceHandleProto_DtypeAndShape::IsInitialized() const {
  return true;
}

void ResourceHandleProto_DtypeAndShape::InternalSwap(ResourceHandleProto_DtypeAndShape* other) {
  using std::swap;
  _internal_metadata_.Swap<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(ResourceHandleProto_DtypeAndShape, dtype_)
      + sizeof(ResourceHandleProto_DtypeAndShape::dtype_)
      - PROTOBUF_FIELD_OFFSET(ResourceHandleProto_DtypeAndShape, shape_)>(
          reinterpret_cast<char*>(&shape_),
          reinterpret_cast<char*>(&other->shape_));
}

::PROTOBUF_NAMESPACE_ID::Metadata ResourceHandleProto_DtypeAndShape::GetMetadata() const {
  return GetMetadataStatic();
}


// ===================================================================

void ResourceHandleProto::InitAsDefaultInstance() {
}
class ResourceHandleProto::_Internal {
 public:
};

ResourceHandleProto::ResourceHandleProto(::PROTOBUF_NAMESPACE_ID::Arena* arena)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena),
  dtypes_and_shapes_(arena) {
  SharedCtor();
  RegisterArenaDtor(arena);
  // @@protoc_insertion_point(arena_constructor:tensorboard.ResourceHandleProto)
}
ResourceHandleProto::ResourceHandleProto(const ResourceHandleProto& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      dtypes_and_shapes_(from.dtypes_and_shapes_) {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  device_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_device().empty()) {
    device_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from._internal_device(),
      GetArena());
  }
  container_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_container().empty()) {
    container_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from._internal_container(),
      GetArena());
  }
  name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_name().empty()) {
    name_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from._internal_name(),
      GetArena());
  }
  maybe_type_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_maybe_type_name().empty()) {
    maybe_type_name_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from._internal_maybe_type_name(),
      GetArena());
  }
  hash_code_ = from.hash_code_;
  // @@protoc_insertion_point(copy_constructor:tensorboard.ResourceHandleProto)
}

void ResourceHandleProto::SharedCtor() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&scc_info_ResourceHandleProto_resource_5fhandle_2eproto.base);
  device_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  container_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  maybe_type_name_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  hash_code_ = PROTOBUF_ULONGLONG(0);
}

ResourceHandleProto::~ResourceHandleProto() {
  // @@protoc_insertion_point(destructor:tensorboard.ResourceHandleProto)
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

void ResourceHandleProto::SharedDtor() {
  GOOGLE_DCHECK(GetArena() == nullptr);
  device_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  container_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  name_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  maybe_type_name_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void ResourceHandleProto::ArenaDtor(void* object) {
  ResourceHandleProto* _this = reinterpret_cast< ResourceHandleProto* >(object);
  (void)_this;
}
void ResourceHandleProto::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void ResourceHandleProto::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const ResourceHandleProto& ResourceHandleProto::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_ResourceHandleProto_resource_5fhandle_2eproto.base);
  return *internal_default_instance();
}


void ResourceHandleProto::Clear() {
// @@protoc_insertion_point(message_clear_start:tensorboard.ResourceHandleProto)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  dtypes_and_shapes_.Clear();
  device_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  container_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  name_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  maybe_type_name_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  hash_code_ = PROTOBUF_ULONGLONG(0);
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* ResourceHandleProto::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  ::PROTOBUF_NAMESPACE_ID::Arena* arena = GetArena(); (void)arena;
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // string device = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          auto str = _internal_mutable_device();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "tensorboard.ResourceHandleProto.device"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string container = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          auto str = _internal_mutable_container();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "tensorboard.ResourceHandleProto.container"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string name = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 26)) {
          auto str = _internal_mutable_name();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "tensorboard.ResourceHandleProto.name"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // uint64 hash_code = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 32)) {
          hash_code_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // string maybe_type_name = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 42)) {
          auto str = _internal_mutable_maybe_type_name();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "tensorboard.ResourceHandleProto.maybe_type_name"));
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // repeated .tensorboard.ResourceHandleProto.DtypeAndShape dtypes_and_shapes = 6;
      case 6:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 50)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_dtypes_and_shapes(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<50>(ptr));
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag,
            _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
            ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* ResourceHandleProto::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:tensorboard.ResourceHandleProto)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string device = 1;
  if (this->device().size() > 0) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_device().data(), static_cast<int>(this->_internal_device().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "tensorboard.ResourceHandleProto.device");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_device(), target);
  }

  // string container = 2;
  if (this->container().size() > 0) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_container().data(), static_cast<int>(this->_internal_container().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "tensorboard.ResourceHandleProto.container");
    target = stream->WriteStringMaybeAliased(
        2, this->_internal_container(), target);
  }

  // string name = 3;
  if (this->name().size() > 0) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_name().data(), static_cast<int>(this->_internal_name().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "tensorboard.ResourceHandleProto.name");
    target = stream->WriteStringMaybeAliased(
        3, this->_internal_name(), target);
  }

  // uint64 hash_code = 4;
  if (this->hash_code() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteUInt64ToArray(4, this->_internal_hash_code(), target);
  }

  // string maybe_type_name = 5;
  if (this->maybe_type_name().size() > 0) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_maybe_type_name().data(), static_cast<int>(this->_internal_maybe_type_name().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "tensorboard.ResourceHandleProto.maybe_type_name");
    target = stream->WriteStringMaybeAliased(
        5, this->_internal_maybe_type_name(), target);
  }

  // repeated .tensorboard.ResourceHandleProto.DtypeAndShape dtypes_and_shapes = 6;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->_internal_dtypes_and_shapes_size()); i < n; i++) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(6, this->_internal_dtypes_and_shapes(i), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:tensorboard.ResourceHandleProto)
  return target;
}

size_t ResourceHandleProto::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:tensorboard.ResourceHandleProto)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .tensorboard.ResourceHandleProto.DtypeAndShape dtypes_and_shapes = 6;
  total_size += 1UL * this->_internal_dtypes_and_shapes_size();
  for (const auto& msg : this->dtypes_and_shapes_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  // string device = 1;
  if (this->device().size() > 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_device());
  }

  // string container = 2;
  if (this->container().size() > 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_container());
  }

  // string name = 3;
  if (this->name().size() > 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_name());
  }

  // string maybe_type_name = 5;
  if (this->maybe_type_name().size() > 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_maybe_type_name());
  }

  // uint64 hash_code = 4;
  if (this->hash_code() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::UInt64Size(
        this->_internal_hash_code());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void ResourceHandleProto::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:tensorboard.ResourceHandleProto)
  GOOGLE_DCHECK_NE(&from, this);
  const ResourceHandleProto* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<ResourceHandleProto>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:tensorboard.ResourceHandleProto)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:tensorboard.ResourceHandleProto)
    MergeFrom(*source);
  }
}

void ResourceHandleProto::MergeFrom(const ResourceHandleProto& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:tensorboard.ResourceHandleProto)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  dtypes_and_shapes_.MergeFrom(from.dtypes_and_shapes_);
  if (from.device().size() > 0) {
    _internal_set_device(from._internal_device());
  }
  if (from.container().size() > 0) {
    _internal_set_container(from._internal_container());
  }
  if (from.name().size() > 0) {
    _internal_set_name(from._internal_name());
  }
  if (from.maybe_type_name().size() > 0) {
    _internal_set_maybe_type_name(from._internal_maybe_type_name());
  }
  if (from.hash_code() != 0) {
    _internal_set_hash_code(from._internal_hash_code());
  }
}

void ResourceHandleProto::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:tensorboard.ResourceHandleProto)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void ResourceHandleProto::CopyFrom(const ResourceHandleProto& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:tensorboard.ResourceHandleProto)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ResourceHandleProto::IsInitialized() const {
  return true;
}

void ResourceHandleProto::InternalSwap(ResourceHandleProto* other) {
  using std::swap;
  _internal_metadata_.Swap<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(&other->_internal_metadata_);
  dtypes_and_shapes_.InternalSwap(&other->dtypes_and_shapes_);
  device_.Swap(&other->device_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  container_.Swap(&other->container_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  name_.Swap(&other->name_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  maybe_type_name_.Swap(&other->maybe_type_name_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
  swap(hash_code_, other->hash_code_);
}

::PROTOBUF_NAMESPACE_ID::Metadata ResourceHandleProto::GetMetadata() const {
  return GetMetadataStatic();
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace tensorboard
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::tensorboard::ResourceHandleProto_DtypeAndShape* Arena::CreateMaybeMessage< ::tensorboard::ResourceHandleProto_DtypeAndShape >(Arena* arena) {
  return Arena::CreateMessageInternal< ::tensorboard::ResourceHandleProto_DtypeAndShape >(arena);
}
template<> PROTOBUF_NOINLINE ::tensorboard::ResourceHandleProto* Arena::CreateMaybeMessage< ::tensorboard::ResourceHandleProto >(Arena* arena) {
  return Arena::CreateMessageInternal< ::tensorboard::ResourceHandleProto >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
