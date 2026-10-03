#ifndef _S8_H_
#define _S8_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream> //ostringstream
#include <cstring> //memcpy
#include <cstdlib> //free, malloc
#include <functional>
#include <memory>
#include <utility>

/** @file s8.h
 *  @brief Node-based text serialization with optional type and object identity metadata.
 */

namespace s8
{
  /** @brief Identifies the value represented by a Node. */
  enum class Type
  {
    NONE,     ///< An uninitialized node.
    SCALAR,   ///< A value represented by scalar text.
    SEQUENCE, ///< An ordered collection of child nodes.
    OBJECT    ///< A collection of named child nodes.
  };

  /** @brief Reports failures from low-level node mutations. */
  enum class NodeError
  {
    OK = 0,
    NODE_NOT_OBJECT,
    NODE_NOT_SEQUENCE,
    NODE_NOT_SCALAR,
    INCOMPATIBLE_SCALAR_SIZE,
    UNKNOWN
  };

  class Node; // fwd

  /** @brief Result returned when creating a child node. */
  typedef struct NodeResult
  {
    NodeError result;
    Node *node;
    std::string error_str;
  } NodeResult_t;

  /**
   * @brief A node in the intermediate serialization tree.
   *
   * Object identity metadata is separate from runtime NodeID_t values. Serialized
   * IDs preserve shared object identity, while NodeID_t values address this tree.
   * Child nodes retain a non-owning parent link. GetPath() follows those links to
   * produce paths such as `$.inventory[0].name`; a detached root returns `$`.
   */
  class Node
  {
  protected:
    Type type_;
    size_t size_;
    // Storage is selected by type_; only the matching member is used.
    void *bytes_;
    std::string scalar_str_;
    std::vector<Node *> *sequence_;
    std::unordered_map<std::string, Node *> *fields_;
    std::string object_type_name_;
    uint32_t serialized_object_id_;
    uint32_t object_reference_id_;
    Node *parent_;

    /** @brief Rebinds direct children after owned storage moves between nodes. */
    void ReparentChildren()
    {
      if (sequence_ != nullptr)
      {
        for (Node *node : *sequence_)
          node->parent_ = this;
      }
      if (fields_ != nullptr)
      {
        for (const auto &pair : *fields_)
          pair.second->parent_ = this;
      }
    }

    /** @brief Releases scalar storage and recursively owned child nodes. */
    void Clear()
    {
      std::free(bytes_);
      bytes_ = nullptr;
      if (sequence_ != nullptr)
      {
        for (Node *node : *sequence_)
          delete node;
        delete sequence_;
        sequence_ = nullptr;
      }
      if (fields_ != nullptr)
      {
        for (const auto &pair : *fields_)
          delete pair.second;
        delete fields_;
        fields_ = nullptr;
      }
    }

    /** @brief Deep-copies storage and child nodes from another node. */
    void CopyFrom(const Node &other)
    {
      if (other.bytes_ != nullptr)
      {
        bytes_ = std::malloc(other.size_);
        if (bytes_ != nullptr)
          std::memcpy(bytes_, other.bytes_, other.size_);
      }
      if (other.sequence_ != nullptr)
      {
        sequence_ = new std::vector<Node *>();
        sequence_->reserve(other.sequence_->size());
        for (const Node *node : *other.sequence_)
        {
          std::unique_ptr<Node> child(new Node(*node));
          sequence_->push_back(child.get());
          child.release();
        }
      }
      if (other.fields_ != nullptr)
      {
        fields_ = new std::unordered_map<std::string, Node *>();
        for (const auto &pair : *other.fields_)
        {
          std::unique_ptr<Node> child(new Node(*pair.second));
          fields_->emplace(pair.first, child.get());
          child.release();
        }
      }
      ReparentChildren();
    }

    /** @brief Transfers owned storage and resets the source node. */
    void MoveFrom(Node &other) noexcept
    {
      bytes_ = other.bytes_;
      sequence_ = other.sequence_;
      fields_ = other.fields_;
      ReparentChildren();
      other.bytes_ = nullptr;
      other.sequence_ = nullptr;
      other.fields_ = nullptr;
      other.size_ = 0;
      other.type_ = Type::NONE;
      other.serialized_object_id_ = 0;
      other.object_reference_id_ = 0;
    }

  public:
    /** @brief Constructs an uninitialized node. */
    Node() : Node(Type::NONE) {}
    /** @brief Constructs an empty node of the requested structural type. */
    Node(Type type) : type_(type), size_(0), bytes_(nullptr), sequence_(nullptr), fields_(nullptr), object_type_name_(), serialized_object_id_(0), object_reference_id_(0), parent_(nullptr)
    {
    }

    /** @brief Constructs a deep copy of another node and its descendants. */
    Node(const Node &other)
        : type_(other.type_), size_(other.size_), bytes_(nullptr), scalar_str_(other.scalar_str_),
          sequence_(nullptr), fields_(nullptr), object_type_name_(other.object_type_name_),
          serialized_object_id_(other.serialized_object_id_), object_reference_id_(other.object_reference_id_),
          parent_(nullptr)
    {
      CopyFrom(other);
    }

    /** @brief Move-constructs a node by transferring owned storage. */
    Node(Node &&other) noexcept
        : type_(other.type_), size_(other.size_), bytes_(nullptr), scalar_str_(std::move(other.scalar_str_)),
          sequence_(nullptr), fields_(nullptr), object_type_name_(std::move(other.object_type_name_)),
          serialized_object_id_(other.serialized_object_id_), object_reference_id_(other.object_reference_id_),
          parent_(nullptr)
    {
      MoveFrom(other);
    }

    /** @brief Replaces this node with a deep copy of another node. */
    Node &operator=(const Node &other)
    {
      if (this != &other)
      {
        Node copy(other);
        Swap(copy);
      }
      return *this;
    }

    /** @brief Replaces this node by transferring another node's storage. */
    Node &operator=(Node &&other) noexcept
    {
      if (this != &other)
      {
        Clear();
        type_ = other.type_;
        size_ = other.size_;
        scalar_str_ = std::move(other.scalar_str_);
        object_type_name_ = std::move(other.object_type_name_);
        serialized_object_id_ = other.serialized_object_id_;
        object_reference_id_ = other.object_reference_id_;
        MoveFrom(other);
      }
      return *this;
    }

    /** @brief Releases all storage owned by this node. */
    ~Node() { Clear(); }

    /** @brief Exchanges complete node state without copying. */
    void Swap(Node &other) noexcept
    {
      using std::swap;
      swap(type_, other.type_);
      swap(size_, other.size_);
      swap(bytes_, other.bytes_);
      swap(scalar_str_, other.scalar_str_);
      swap(sequence_, other.sequence_);
      swap(fields_, other.fields_);
      swap(object_type_name_, other.object_type_name_);
      swap(serialized_object_id_, other.serialized_object_id_);
      swap(object_reference_id_, other.object_reference_id_);
      ReparentChildren();
      other.ReparentChildren();
    }

    /** @brief Returns the node's structural type. */
    Type GetType() const { return type_; }
    /** @brief Returns the byte size of the in-memory scalar value. */
    size_t GetSize() const { return size_; }
    /** @brief Returns the scalar's textual representation. */
    const std::string &GetScalarStr() const { return scalar_str_; }
    /** @brief Returns mutable object fields, or nullptr when absent. */
    std::unordered_map<std::string, Node *> *GetFields() { return fields_; }
    /** @brief Returns read-only object fields, or nullptr when absent. */
    const std::unordered_map<std::string, Node *> *GetFields() const { return fields_; }
    /** @brief Returns mutable sequence elements, or nullptr when absent. */
    std::vector<Node *> *GetSequence() { return sequence_; }
    /** @brief Returns read-only sequence elements, or nullptr when absent. */
    const std::vector<Node *> *GetSequence() const { return sequence_; }
    /** @brief Returns the stable C++ type name for a typed object node. */
    const std::string &GetObjectTypeName() const { return object_type_name_; }
    /** @brief Returns the portable identity assigned to an object definition. */
    uint32_t GetSerializedObjectID() const { return serialized_object_id_; }
    /** @brief Returns the portable identity referenced by a reference-only node. */
    uint32_t GetObjectReferenceID() const { return object_reference_id_; }
    /** @brief Returns the non-owning parent pointer, or nullptr for a tree root. */
    Node *GetParent() { return parent_; }
    /** @brief Returns the non-owning read-only parent, or nullptr for a tree root. */
    const Node *GetParent() const { return parent_; }

    /**
     * @brief Returns the path from the tree root.
     * @return `$` for a root, or a field/index path such as `$.items[0].name`.
     */
    std::string GetPath() const
    {
      if (parent_ == nullptr)
      {
        return "$";
      }

      const std::string parent_path = parent_->GetPath();
      if (parent_->fields_ != nullptr)
      {
        for (const auto &pair : *parent_->fields_)
        {
          if (pair.second == this)
          {
            return parent_path + "." + pair.first;
          }
        }
      }
      if (parent_->sequence_ != nullptr)
      {
        for (size_t index = 0; index < parent_->sequence_->size(); ++index)
        {
          if ((*parent_->sequence_)[index] == this)
          {
            return parent_path + "[" + std::to_string(index) + "]";
          }
        }
      }
      return parent_path + ".?";
    }

    /** @brief Sets type metadata once on an object node. */
    bool SetObjectTypeName(const std::string &type_name)
    {
      if (type_ != Type::OBJECT || type_name.empty() || !object_type_name_.empty())
      {
        return false;
      }
      object_type_name_ = type_name;
      return true;
    }

    /** @brief Marks this object as the unique definition of a serialized identity. */
    bool SetSerializedObjectID(uint32_t id)
    {
      if (type_ != Type::OBJECT || id == 0 || serialized_object_id_ != 0 || object_reference_id_ != 0)
      {
        return false;
      }
      serialized_object_id_ = id;
      return true;
    }

    /** @brief Marks this empty object node as a reference to another definition. */
    bool SetObjectReferenceID(uint32_t id)
    {
      if (type_ != Type::OBJECT || id == 0 || object_reference_id_ != 0 ||
          serialized_object_id_ != 0 || HasFields() || !object_type_name_.empty())
      {
        return false;
      }
      object_reference_id_ = id;
      return true;
    }

    /** @brief Reports whether this object node contains fields. */
    bool HasFields() const
    {
      return type_ == Type::OBJECT && fields_ != nullptr && !fields_->empty();
    }

    /** @brief Creates or retrieves a named child, converting a NONE node to OBJECT. */
    bool CreateField(const std::string &name, Type type, NodeResult_t &result)
    {
      bool retval = false;
      if (type_ == Type::NONE || type_ == Type::OBJECT)
      {
        type_ = Type::OBJECT;
        if (fields_ == nullptr)
        {
          fields_ = new std::unordered_map<std::string, Node *>();
        }
        if (fields_->count(name) == 1)
        {
          result.node = (*fields_)[name];
        }
        else
        {
          result.node = new Node(type);
          result.node->parent_ = this;
          (*fields_)[name] = result.node;
        }
        retval = true;
        result.result = NodeError::OK;
      }
      else
      {
        result.error_str = "AddField: not an object node";
        result.result = NodeError::NODE_NOT_OBJECT;
      }
      return retval;
    }
    /** @brief Returns a named child, or nullptr if it does not exist. */
    Node *GetField(const std::string &name)
    {
      if (type_ == Type::OBJECT && fields_ != nullptr && fields_->count(name) == 1)
      {
        return (*fields_)[name];
      }
      return nullptr;
    }
    /** @brief Returns a read-only named child, or nullptr if absent. */
    const Node *GetField(const std::string &name) const
    {
      if (type_ == Type::OBJECT && fields_ != nullptr && fields_->count(name) == 1)
      {
        return fields_->at(name);
      }
      return nullptr;
    }
    /** @brief Appends a child to a sequence node. */
    bool CreateElement(Type type, NodeResult_t &result)
    {
      bool retval = false;
      if (type_ == Type::NONE || type_ == Type::SEQUENCE)
      {
        type_ = Type::SEQUENCE;
        if (sequence_ == nullptr)
        {
          sequence_ = new std::vector<Node *>();
        }
        result.node = new Node(type);
        result.node->parent_ = this;
        result.result = NodeError::OK;
        sequence_->push_back(result.node);
        retval = true;
      }
      else
      {
        result.node = nullptr;
        result.error_str = "AddElement: not a sequence node";
        result.result = NodeError::NODE_NOT_SEQUENCE;
      }
      return retval;
    }

    /** @brief Returns a sequence child by index, or nullptr when out of range. */
    Node *GetElement(size_t index)
    {
      if (type_ == Type::SEQUENCE && sequence_ != nullptr && sequence_->size() > index)
      {
        return (*sequence_)[index];
      }
      return nullptr;
    }
    /** @brief Returns a read-only sequence child, or nullptr if out of range. */
    const Node *GetElement(size_t index) const
    {
      if (type_ == Type::SEQUENCE && sequence_ != nullptr && sequence_->size() > index)
      {
        return (*sequence_)[index];
      }
      return nullptr;
    }
    /** @brief Stores a scalar and its textual representation. */
    template <typename T>
    NodeError SetValue(const T &value)
    {
      if (type_ != Type::NONE && type_ != Type::SCALAR)
      {
        return NodeError::NODE_NOT_SCALAR;
      }

      type_ = Type::SCALAR;
      if (bytes_ == nullptr)
      {
        size_ = sizeof(value);
        bytes_ = malloc(size_);
      }
      if (sizeof(value) != size_)
      {
        return NodeError::INCOMPATIBLE_SCALAR_SIZE;
      }

      memcpy(bytes_, &value, size_);
      std::ostringstream oss;
      oss << value;
      scalar_str_ = oss.str();
      return NodeError::OK;
    }
    /** @brief Returns the in-memory scalar value when available. */
    template <typename T>
    T *GetValue()
    {
      if (bytes_ != nullptr)
      {
        return static_cast<T *>(bytes_);
      }
      return nullptr;
    }
    /** @brief Returns read-only in-memory scalar storage when available. */
    template <typename T>
    const T *GetValue() const
    {
      return bytes_ == nullptr ? nullptr : static_cast<const T *>(bytes_);
    }
  };

  /** @brief Reports text codec success or failure. */
  enum class CodecError
  {
    OK = 0,
    UNEXPECTED_NULL,
    MALFORMED_INPUT,
    UNIMPLEMENTED
  };

  /**
   * @brief Result of a codec operation.
   *
   * A successful result has status CodecError::OK and an empty diagnostic.
   * On failure, error_message contains caller-loggable context.
   */
  struct CodecResult
  {
    CodecError status;
    std::string error_message;

    /** @brief Constructs a codec result with an optional diagnostic. */
    CodecResult(CodecError status = CodecError::OK, const std::string &error_message = std::string())
        : status(status), error_message(error_message) {}

    /** @brief Reports whether the codec operation succeeded. */
    bool Ok() const { return status == CodecError::OK; }
  };

  /** @brief Identifies the last failed Serialiser API operation. */
  enum class SerialiserError
  {
    OK = 0,
    INVALID_NODE_ID,
    INVALID_NODE_TYPE,
    EMPTY_FIELD_NAME,
    NODE_OPERATION_FAILED,
    SCALAR_PARSE_FAILED,
    OBJECT_TYPE_MISMATCH,
    NULL_SHARED_OBJECT,
    INVALID_OBJECT_REFERENCE,
    SERIALISE_CALLBACK_FAILED,
    DESERIALISE_CALLBACK_FAILED,
    CODEC_FAILED
  };

  /**
   * @brief Persistent diagnostic for the most recent top-level Serialiser call.
   *
   * node_path snapshots the failing node's path when the result is created, and
   * error_message includes that path when a valid node is available.
   */
  struct SerialiserResult
  {
    SerialiserError status;
    NodeError node_error;
    CodecError codec_error;
    uint32_t node_id;
    std::string node_path;
    std::string error_message;

    SerialiserResult(SerialiserError status = SerialiserError::OK,
                     const std::string &error_message = std::string(),
                     uint32_t node_id = 0,
                     NodeError node_error = NodeError::OK,
                     CodecError codec_error = CodecError::OK,
                     const std::string &node_path = std::string())
        : status(status), node_error(node_error), codec_error(codec_error),
          node_id(node_id), node_path(node_path), error_message(error_message) {}

    /** @brief Reports whether the most recent operation succeeded. */
    bool Ok() const { return status == SerialiserError::OK; }
  };

  class Serialiser; // fwd

  /** @brief Interface for converting between a node tree and an external format. */
  class Codec
  {
  public:
    virtual ~Codec() {};
    /**
     * @brief Encodes the tree rooted at @p s into @p str.
     * @return Success, or an error status with a caller-loggable diagnostic.
     */
    virtual CodecResult ToString(const Node *s, std::string &str) const = 0;
    /**
     * @brief Decodes @p str into @p s using freshly allocated runtime node IDs.
     * @return Success, or an error status with a caller-loggable diagnostic.
     */
    virtual CodecResult FromString(const std::string &str, Serialiser &s) const = 0;
  };

  /**
   * @brief Builds and reads a Node tree addressed by runtime node IDs.
   *
   * Serialiser is the central intermediate representation for the s8 system.
   * Application classes describe themselves through static functions, Serialiser
   * builds or reads a tree of Node objects, and a Codec converts that tree to or
   * from an external representation. PlaintextCodec is the supplied JSON-like
   * Codec implementation. ObjectRegistry is optional and is used only when the
   * concrete C++ type must be selected from serialized `$type` metadata.
   *
   * @par Serializable type contract
   * A typed class `T` provides the following static functions:
   * @code{.cpp}
   * static std::string TypeName();
   * static bool Serialise(const T&, s8::Serialiser&, s8::Serialiser::NodeID_t);
   * static bool Deserialise(T&, s8::Serialiser&, s8::Serialiser::NodeID_t);
   * @endcode
   * WriteObject() and ReadObject() invoke this contract. The class controls field
   * names and scalar conversions, so scalar C++ type information is not written
   * into the text.
   *
   * @par Basic object example
   * A value object writes its fields below the node passed to Serialise():
   * @code{.cpp}
   * struct Point
   * {
   *   int x = 0;
   *   int y = 0;
   *
   *   static std::string TypeName() { return "Point"; }
   *
   *   static bool Serialise(const Point &point, s8::Serialiser &s,
   *                         s8::Serialiser::NodeID_t id)
   *   {
   *     return s.Value(s.ScalarField(id, "x"), point.x) &&
   *            s.Value(s.ScalarField(id, "y"), point.y);
   *   }
   *
   *   static bool Deserialise(Point &point, s8::Serialiser &s,
   *                           s8::Serialiser::NodeID_t id)
   *   {
   *     return s.ReadScalar(s.GetFieldID(id, "x"), point.x) &&
   *            s.ReadScalar(s.GetFieldID(id, "y"), point.y);
   *   }
   * };
   *
   * Point point{3, 7};
   * s8::PlaintextCodec codec;
   * s8::Serialiser writer;
   * writer.WriteObject(writer.GetRootID(), point);
   * std::string text;
   * writer.Serialise(codec, text);
   *
   * s8::Serialiser reader;
   * reader.Deserialise(text, codec);
   * Point copy;
   * reader.ReadObject(reader.GetRootID(), copy);
   * @endcode
   *
   * @par Shared-reference example
   * Use WriteSharedObject() and ReadSharedObject() for owning references. The
   * first occurrence writes an object definition; aliases and cycle edges write
   * references to that definition.
   * @code{.cpp}
   * struct Actor
   * {
   *   std::string name;
   *   std::shared_ptr<Actor> target;
   *
   *   static std::string TypeName() { return "Actor"; }
   *
   *   static bool Serialise(const Actor &actor, s8::Serialiser &s,
   *                         s8::Serialiser::NodeID_t id)
   *   {
   *     if (!s.Value(s.ScalarField(id, "name"), actor.name))
   *       return false;
   *     return !actor.target ||
   *            s.WriteSharedObject(s.ObjectField(id, "target"), actor.target);
   *   }
   *
   *   static bool Deserialise(Actor &actor, s8::Serialiser &s,
   *                           s8::Serialiser::NodeID_t id)
   *   {
   *     if (!s.ReadScalar(s.GetFieldID(id, "name"), actor.name))
   *       return false;
   *     const auto target_id = s.GetFieldID(id, "target");
   *     if (target_id == s8::Serialiser::INVALID_ID)
   *     {
   *       actor.target.reset();
   *       return true;
   *     }
   *     return s.ReadSharedObject(target_id, actor.target);
   *   }
   * };
   *
   * struct Scene
   * {
   *   std::shared_ptr<Actor> player;
   *   std::shared_ptr<Actor> selected;
   *
   *   static std::string TypeName() { return "Scene"; }
   *
   *   static bool Serialise(const Scene &scene, s8::Serialiser &s,
   *                         s8::Serialiser::NodeID_t id)
   *   {
   *     return s.WriteSharedObject(s.ObjectField(id, "player"), scene.player) &&
   *            s.WriteSharedObject(s.ObjectField(id, "selected"), scene.selected);
   *   }
   *
   *   static bool Deserialise(Scene &scene, s8::Serialiser &s,
   *                           s8::Serialiser::NodeID_t id)
   *   {
   *     return s.ReadSharedObject(s.GetFieldID(id, "player"), scene.player) &&
   *            s.ReadSharedObject(s.GetFieldID(id, "selected"), scene.selected);
   *   }
   * };
   *
   * Scene scene;
   * scene.player = std::make_shared<Actor>();
   * scene.player->name = "Ada";
   * scene.player->target = scene.player; // A cycle.
   * scene.selected = scene.player;       // An alias.
   * s8::Serialiser writer;
   * writer.WriteObject(writer.GetRootID(), scene);
   * @endcode
   *
   * @par Node parents and paths
   * Every child node has a non-owning link to its parent. Paths start at `$`,
   * use `.field` for object fields, and `[index]` for sequence elements.
   * @code{.cpp}
   * s8::Serialiser serialiser;
   * const auto root = serialiser.GetRootID();
   * const auto items = serialiser.SequenceField(root, "items");
   * const auto first_item = serialiser.ObjectElement(items);
   * const auto name = serialiser.ScalarField(first_item, "name");
   * serialiser.Value(name, std::string("potion"));
   *
   * const s8::Node *name_node = serialiser.GetNode(name);
   * const s8::Node *item_node = serialiser.GetNode(first_item);
   * assert(name_node->GetParent() == item_node);
   * assert(name_node->GetPath() == "$.items[0].name");
   * @endcode
   * Parent links are repaired when a Node tree is copied or moved. A copied or
   * moved root is detached and therefore starts a new path at `$`.
   *
   * @par System flow
   * @dot
   * digraph s8_serialisation_system {
   *   rankdir=LR;
   *   graph [fontname="sans-serif", nodesep=0.35, ranksep=0.55];
   *   node [shape=box, fontname="sans-serif"];
   *   edge [fontname="sans-serif", fontsize=10];
   *
   *   subgraph cluster_write {
   *     label="Serialization: C++ object to text";
   *     color="#4f81bd";
   *
   *     write_object [label="Application object T"];
   *     write_entry [label="Serialiser::WriteObject<T>()"];
   *     write_contract [label="T::TypeName()\nT::Serialise()"];
   *     write_tree [label="Serialiser\nNode tree + NodeID_t map"];
   *     write_fields [label="ScalarField / ObjectField\nSequenceField / Value"];
   *     encode [label="Serialiser::Serialise(codec)\nCodec::ToString()"];
   *     text [label="Plaintext\n$type, fields, $id/$ref", shape=note];
   *
   *     write_object -> write_entry;
   *     write_entry -> write_contract;
   *     write_contract -> write_fields;
   *     write_fields -> write_tree;
   *     write_tree -> encode;
   *     encode -> text;
   *   }
   *
   *   subgraph cluster_read {
   *     label="Deserialization: text to C++ object";
   *     color="#70ad47";
   *
   *     input [label="Plaintext", shape=note];
   *     decode [label="Serialiser::Deserialise(text, codec)\nCodec::FromString()"];
   *     parser [label="PlaintextCodec::Parser\nAddField / AddElement / Value"];
   *     read_tree [label="Serialiser\nrebuilt Node tree + fresh NodeID_t values"];
   *     direct [label="Known T\nSerialiser::ReadObject<T>()"];
   *     registry [label="Unknown concrete type\nObjectRegistry::Create()"];
   *     factory [label="Lookup $type\nregistered factory for T"];
   *     read_contract [label="T::Deserialise()\nGetFieldID / ReadScalar"];
   *     result [label="Reconstructed object T"];
   *
   *     input -> decode;
   *     decode -> parser;
   *     parser -> read_tree;
   *     read_tree -> direct [label="caller knows T"];
   *     read_tree -> registry [label="type chosen from text"];
   *     registry -> factory;
   *     factory -> read_contract;
   *     direct -> read_contract;
   *     read_contract -> result;
   *   }
   * }
   * @enddot
   *
   * @par Runtime node IDs and serialized object IDs
   * NodeID_t values are local handles used to navigate one Serialiser instance.
   * They are regenerated by Codec::FromString() and never appear in plaintext.
   * Shared object identity uses a separate portable namespace: WriteSharedObject()
   * emits `$id` for the first occurrence and `$ref` for aliases or cycle edges.
   * ReadSharedObject() caches a placeholder before calling T::Deserialise(), which
   * permits forward references and cycles to resolve to the same shared_ptr.
   * ValidateObjectReferences() rejects duplicate definitions and dangling refs.
   *
   * @par Error reporting
   * Each top-level API call updates GetLastResult(). Nested calls made by a
   * type's static Serialise() or Deserialise() function preserve the first
   * concrete failure, such as a missing field or scalar conversion error. If a
   * callback simply returns false, the result reports SERIALISE_CALLBACK_FAILED
   * or DESERIALISE_CALLBACK_FAILED. Codec failures also retain their CodecError.
   * SerialiserResult::node_path contains the failing node's full path, which is
   * also appended to SerialiserResult::error_message.
   * A later successful top-level call clears the state; ClearResult() can clear
   * it explicitly.
   * @code{.cpp}
   * if (!writer.WriteObject(writer.GetRootID(), object))
   * {
   *   const s8::SerialiserResult &result = writer.GetLastResult();
   *   std::cerr << result.error_message << '\n';
   * }
   *
   * s8::Serialiser reader;
   * const bool loaded = reader.Deserialise(text, codec).Ok() &&
   *                     reader.ReadObject(reader.GetRootID(), object);
   * if (!loaded)
   * {
   *   const s8::SerialiserResult &result = reader.GetLastResult();
   *   std::cerr << "Deserialisation failed at " << result.node_path << ": "
   *             << result.error_message << '\n';
   * }
   * @endcode
   *
   * @par Registry and RTTI
   * ObjectRegistry maps stable TypeName() strings to executable-local factories.
   * The registry and function callbacks are not serialized; the loading program
   * must register every dynamically creatable type. Known-target ReadObject<T>()
   * does not require registry lookup. The system does not use C++ RTTI.
   *
   * @see Node
   * @see Codec
   * @see PlaintextCodec
   * @see ObjectRegistry
   * @see RegisteredObject
   */
  class Serialiser
  {
  public:
    /** @brief Runtime handle for a node owned by this Serialiser. */
    typedef uint32_t NodeID_t;
    /** @brief Sentinel returned when a node operation fails. */
    static const NodeID_t INVALID_ID = 0;

  protected:
    struct ObjectCacheEntry
    {
      std::string type_name;
      std::shared_ptr<void> object;
      bool populating;
      bool complete;

      ObjectCacheEntry() : type_name(), object(), populating(false), complete(false) {}
    };

    std::unordered_map<uint32_t, Node *> node_map_;
    NodeID_t next_id_;
    Node root_;
    uint32_t next_serialized_object_id_;
    std::unordered_map<const void *, uint32_t> written_object_ids_;
    std::unordered_map<uint32_t, ObjectCacheEntry> read_object_cache_;
    mutable SerialiserResult last_result_;
    mutable size_t operation_depth_;

    SerialiserResult FailureResult(SerialiserError status, const std::string &message,
                                   NodeID_t node_id, NodeError node_error = NodeError::OK,
                                   CodecError codec_error = CodecError::OK) const
    {
      const Node *node = GetNode(node_id);
      const std::string path = node == nullptr ? std::string() : node->GetPath();
      const std::string diagnostic = path.empty() ? message : message + " at " + path;
      return SerialiserResult(status, diagnostic, node_id, node_error, codec_error, path);
    }

    void BeginOperation() const
    {
      if (operation_depth_ == 0)
      {
        last_result_ = SerialiserResult();
      }
      ++operation_depth_;
    }

    bool EndOperation(bool success, SerialiserError fallback_status,
                      const std::string &fallback_message, NodeID_t node_id = INVALID_ID) const
    {
      if (!success && last_result_.Ok())
      {
        last_result_ = FailureResult(fallback_status, fallback_message, node_id);
      }
      --operation_depth_;
      if (success && operation_depth_ == 0)
      {
        last_result_ = SerialiserResult();
      }
      return success;
    }

    NodeID_t EndNodeOperation(NodeID_t result, SerialiserError fallback_status,
                              const std::string &fallback_message, NodeID_t node_id) const
    {
      EndOperation(result != INVALID_ID, fallback_status, fallback_message, node_id);
      return result;
    }

    void SetFailure(SerialiserError status, const std::string &message,
                    NodeID_t node_id = INVALID_ID, NodeError node_error = NodeError::OK,
                    CodecError codec_error = CodecError::OK) const
    {
      if (last_result_.Ok())
      {
        last_result_ = FailureResult(status, message, node_id, node_error, codec_error);
      }
    }

  public:
    /** @brief Constructs an empty serializer rooted at an object node. */
    Serialiser()
        : next_id_(INVALID_ID + 1), root_(Type::OBJECT), next_serialized_object_id_(1),
          last_result_(), operation_depth_(0)
    {
      node_map_[next_id_] = &root_;
      next_id_++;
    };

    /** @brief Disabled because runtime IDs contain pointers into this instance. */
    Serialiser(const Serialiser &) = delete;
    /** @brief Disabled because runtime IDs contain pointers into this instance. */
    Serialiser &operator=(const Serialiser &) = delete;
    /** @brief Disabled because moving would invalidate runtime node pointers. */
    Serialiser(Serialiser &&) = delete;
    /** @brief Disabled because moving would invalidate runtime node pointers. */
    Serialiser &operator=(Serialiser &&) = delete;
    /** @brief Destroys the serializer and its owned node tree. */
    virtual ~Serialiser() = default;

    /**
     * @brief Encodes the current root node with @p codec.
     * @return CodecResult whose diagnostic may be logged when status is not OK.
     */
    CodecResult Serialise(const Codec &codec, std::string &output) const
    {
      BeginOperation();
      CodecResult result = codec.ToString(&root_, output);
      if (!result.Ok())
      {
        SetFailure(SerialiserError::CODEC_FAILED, result.error_message, GetRootID(),
                   NodeError::OK, result.status);
      }
      EndOperation(result.Ok(), SerialiserError::CODEC_FAILED,
                   "Codec failed to serialise the node tree", GetRootID());
      return result;
    }

    /**
     * @brief Rebuilds this Serialiser's node tree from text.
     * @return CodecResult whose diagnostic may be logged when status is not OK.
     */
    CodecResult Deserialise(const std::string &str, const Codec &codec)
    {
      BeginOperation();
      CodecResult result = codec.FromString(str, *this);
      if (!result.Ok())
      {
        SetFailure(SerialiserError::CODEC_FAILED, result.error_message, GetRootID(),
                   NodeError::OK, result.status);
      }
      EndOperation(result.Ok(), SerialiserError::CODEC_FAILED,
                   "Codec failed to deserialise the input", GetRootID());
      return result;
    }

    /** @brief Returns the diagnostic from the most recent top-level API call. */
    const SerialiserResult &GetLastResult() const { return last_result_; }

    /** @brief Explicitly clears the stored diagnostic. */
    void ClearResult() const { last_result_ = SerialiserResult(); }

    /** @brief Returns the root object node. */
    Node &GetRoot()
    {
      return root_;
    }
    /** @brief Returns the read-only root object node. */
    const Node &GetRoot() const
    {
      return root_;
    }
    /** @brief Returns the runtime ID assigned to the root node. */
    NodeID_t GetRootID() const
    {
      return GetIDFor(&root_);
    }

    /** @brief Resolves a runtime node ID, or nullptr when it is invalid. */
    Node *GetNode(NodeID_t id)
    {
      if (id == INVALID_ID)
        return nullptr;

      auto it = node_map_.find(id);
      if (it != node_map_.end())
      {
        return it->second;
      }
      return nullptr;
    }
    /** @brief Resolves a runtime node ID to a read-only node. */
    const Node *GetNode(NodeID_t id) const
    {
      if (id == INVALID_ID)
        return nullptr;

      auto it = node_map_.find(id);
      return it == node_map_.end() ? nullptr : it->second;
    }

    /** @brief Finds the runtime ID associated with a node pointer. */
    NodeID_t GetIDFor(const Node *node) const
    {
      for (const auto &pair : node_map_)
      {
        if (pair.second == node)
        {
          return pair.first;
        }
      }
      return INVALID_ID;
    }

    /** @brief Resolves a named object field to its runtime node ID. */
    NodeID_t GetFieldID(NodeID_t id, const std::string &name) const
    {
      BeginOperation();
      const Node *node = GetNode(id);
      NodeID_t result = node == nullptr ? INVALID_ID : GetIDFor(node->GetField(name));
      return EndNodeOperation(result, SerialiserError::INVALID_NODE_ID,
                              "Object field was not found", id);
    }

    /** @brief Resolves a sequence element to its runtime node ID. */
    NodeID_t GetElementID(NodeID_t id, size_t index) const
    {
      BeginOperation();
      const Node *node = GetNode(id);
      NodeID_t result = node == nullptr ? INVALID_ID : GetIDFor(node->GetElement(index));
      return EndNodeOperation(result, SerialiserError::INVALID_NODE_ID,
                              "Sequence element was not found", id);
    }

    /** @brief Returns the number of elements in a sequence node. */
    size_t GetSequenceSize(NodeID_t id) const
    {
      const Node *node = GetNode(id);
      const std::vector<Node *> *sequence = node == nullptr ? nullptr : node->GetSequence();
      return sequence == nullptr ? 0 : sequence->size();
    }

    /** @brief Returns an object's stable type name, or an empty string. */
    const std::string &GetObjectType(NodeID_t id) const
    {
      static const std::string empty;
      const Node *node = GetNode(id);
      return node == nullptr ? empty : node->GetObjectTypeName();
    }

    /** @brief Assigns stable type metadata to an object node. */
    bool SetObjectType(NodeID_t id, const std::string &type_name)
    {
      BeginOperation();
      Node *node = GetNode(id);
      bool success = node != nullptr && node->SetObjectTypeName(type_name);
      return EndOperation(success, SerialiserError::INVALID_NODE_TYPE,
                          "Could not set object type metadata", id);
    }

    /** @brief Marks an object node as a serialized identity definition. */
    bool SetSerializedObjectID(NodeID_t id, uint32_t serialized_id)
    {
      BeginOperation();
      Node *node = GetNode(id);
      bool success = node != nullptr && node->SetSerializedObjectID(serialized_id);
      return EndOperation(success, SerialiserError::INVALID_OBJECT_REFERENCE,
                          "Could not set serialized object identity", id);
    }

    /** @brief Marks an object node as a reference to a serialized identity. */
    bool SetObjectReferenceID(NodeID_t id, uint32_t serialized_id)
    {
      BeginOperation();
      Node *node = GetNode(id);
      bool success = node != nullptr && node->SetObjectReferenceID(serialized_id);
      return EndOperation(success, SerialiserError::INVALID_OBJECT_REFERENCE,
                          "Could not set serialized object reference", id);
    }

    Node *operator[](NodeID_t id)
    {
      return GetNode(id);
    }
    /** @brief Resolves a runtime node ID to a read-only node. */
    const Node *operator[](NodeID_t id) const
    {
      return GetNode(id);
    }

    /** @brief Creates a scalar field under an object node. */
    NodeID_t ScalarField(NodeID_t id, const std::string &name)
    {
      return AddField(id, name, s8::Type::SCALAR);
    }
    /** @brief Creates a sequence field under an object node. */
    NodeID_t SequenceField(NodeID_t id, const std::string &name)
    {
      return AddField(id, name, s8::Type::SEQUENCE);
    }
    /** @brief Creates an object field under an object node. */
    NodeID_t ObjectField(NodeID_t id, const std::string &name)
    {
      return AddField(id, name, s8::Type::OBJECT);
    }

    /** @brief Appends a scalar node to a sequence. */
    NodeID_t ScalarElement(NodeID_t id)
    {
      return AddElement(id, s8::Type::SCALAR);
    }
    /** @brief Appends a sequence node to a sequence. */
    NodeID_t SequenceElement(NodeID_t id)
    {
      return AddElement(id, s8::Type::SEQUENCE);
    }
    /** @brief Appends an object node to a sequence. */
    NodeID_t ObjectElement(NodeID_t id)
    {
      return AddElement(id, s8::Type::OBJECT);
    }

    /** @brief Creates a named child and registers its runtime ID. */
    NodeID_t AddField(NodeID_t node_id, const std::string &name, Type type)
    {
      BeginOperation();
      NodeID_t retval = INVALID_ID;
      Node *node = nullptr;
      if (!name.empty() && node_id != INVALID_ID)
      {
        node = GetNode(node_id);
        if (node != nullptr)
        {
          NodeResult_t result;
          if (node->CreateField(name, type, result))
          {
            retval = next_id_++;
            node_map_[retval] = result.node;
          }
          else
          {
            SetFailure(SerialiserError::NODE_OPERATION_FAILED, result.error_str,
                       node_id, result.result);
          }
        }
      }
      SerialiserError status = name.empty() ? SerialiserError::EMPTY_FIELD_NAME
                                            : SerialiserError::INVALID_NODE_ID;
      const std::string message = name.empty() ? "Field name cannot be empty"
                                               : "Could not add field to node";
      return EndNodeOperation(retval, status, message, node_id);
    }

    /** @brief Appends a child and registers its runtime ID. */
    NodeID_t AddElement(NodeID_t node_id, Type type)
    {
      BeginOperation();
      NodeID_t retval = INVALID_ID;
      Node *node = (*this)[node_id];
      if (nullptr != node && node->GetType() == Type::SEQUENCE)
      {
        NodeResult_t result;
        if (node->CreateElement(type, result))
        {
          retval = next_id_++;
          node_map_[retval] = result.node;
        }
        else
        {
          SetFailure(SerialiserError::NODE_OPERATION_FAILED, result.error_str,
                     node_id, result.result);
        }
      }
      return EndNodeOperation(retval, SerialiserError::INVALID_NODE_TYPE,
                              "Could not append an element to node", node_id);
    }

    /** @brief Writes a scalar value into an existing scalar node. */
    template <typename T>
    bool Value(NodeID_t id, const T &value)
    {
      BeginOperation();
      bool retval = false;
      Node *node = GetNode(id);
      if (node != nullptr && node->GetType() == Type::SCALAR)
      {
        NodeError node_error = node->SetValue<T>(value);
        retval = node_error == NodeError::OK;
        if (!retval)
        {
          SetFailure(SerialiserError::NODE_OPERATION_FAILED,
                     "Could not write scalar value", id, node_error);
        }
      }
      return EndOperation(retval, node == nullptr ? SerialiserError::INVALID_NODE_ID : SerialiserError::INVALID_NODE_TYPE,
                          "Target node is not a scalar", id);
    }

    /** @brief Reads the exact text stored by a scalar node. */
    bool ReadScalar(NodeID_t id, std::string &value) const
    {
      BeginOperation();
      const Node *node = GetNode(id);
      if (node == nullptr || node->GetType() != Type::SCALAR)
      {
        return EndOperation(false, node == nullptr ? SerialiserError::INVALID_NODE_ID : SerialiserError::INVALID_NODE_TYPE,
                            "Source node is not a scalar", id);
      }
      value = node->GetScalarStr();
      return EndOperation(true, SerialiserError::SCALAR_PARSE_FAILED, std::string(), id);
    }

    /** @brief Parses an entire scalar string into the caller-selected type. */
    template <typename T>
    bool ReadScalar(NodeID_t id, T &value) const
    {
      BeginOperation();
      const Node *node = GetNode(id);
      if (node == nullptr || node->GetType() != Type::SCALAR)
      {
        return EndOperation(false, node == nullptr ? SerialiserError::INVALID_NODE_ID : SerialiserError::INVALID_NODE_TYPE,
                            "Source node is not a scalar", id);
      }

      std::istringstream input(node->GetScalarStr());
      if (!(input >> value))
      {
        return EndOperation(false, SerialiserError::SCALAR_PARSE_FAILED,
                            "Could not parse scalar value", id);
      }
      input >> std::ws;
      return EndOperation(input.eof(), SerialiserError::SCALAR_PARSE_FAILED,
                          "Scalar value contains trailing characters", id);
    }

    /**
     * @brief Writes a typed object using its static serialization contract.
     * @tparam T Type providing TypeName() and Serialise(const T&, Serialiser&, NodeID_t).
     */
    template <typename T>
    bool WriteObject(NodeID_t id, const T &object)
    {
      BeginOperation();
      bool success = SetObjectType(id, T::TypeName()) && T::Serialise(object, *this, id);
      return EndOperation(success, SerialiserError::SERIALISE_CALLBACK_FAILED,
                          "The object's static Serialise function failed", id);
    }

    /**
     * @brief Reads a typed object using its static deserialization contract.
     * @tparam T Type providing TypeName() and Deserialise(T&, Serialiser&, NodeID_t).
     */
    template <typename T>
    bool ReadObject(NodeID_t id, T &object)
    {
      BeginOperation();
      Node *node = GetNode(id);
      if (node == nullptr || node->GetType() != Type::OBJECT)
      {
        return EndOperation(false, node == nullptr ? SerialiserError::INVALID_NODE_ID : SerialiserError::INVALID_NODE_TYPE,
                            "Source node is not an object", id);
      }

      const std::string &type_name = node->GetObjectTypeName();
      if (!type_name.empty() && type_name != T::TypeName())
      {
        return EndOperation(false, SerialiserError::OBJECT_TYPE_MISMATCH,
                            "Serialized object type does not match the requested type", id);
      }
      bool success = T::Deserialise(object, *this, id);
      return EndOperation(success, SerialiserError::DESERIALISE_CALLBACK_FAILED,
                          "The object's static Deserialise function failed", id);
    }

    /**
     * @brief Writes a shared object definition or a reference to a prior object.
     *
     * The first pointer occurrence receives a portable serialized ID. Later
     * occurrences become reference-only nodes, preventing recursion on cycles.
     */
    template <typename T>
    bool WriteSharedObject(NodeID_t id, const std::shared_ptr<T> &object)
    {
      BeginOperation();
      Node *node = GetNode(id);
      if (node == nullptr || node->GetType() != Type::OBJECT)
      {
        return EndOperation(false, node == nullptr ? SerialiserError::INVALID_NODE_ID : SerialiserError::INVALID_NODE_TYPE,
                            "Target node is not an object", id);
      }
      if (object == nullptr)
      {
        return EndOperation(false, SerialiserError::NULL_SHARED_OBJECT,
                            "Cannot write a null shared object", id);
      }

      auto existing = written_object_ids_.find(object.get());
      if (existing != written_object_ids_.end())
      {
        bool success = SetObjectReferenceID(id, existing->second);
        return EndOperation(success, SerialiserError::INVALID_OBJECT_REFERENCE,
                            "Could not write shared object reference", id);
      }

      uint32_t serialized_id = next_serialized_object_id_++;
      // Register identity before descending so back-edges become references.
      written_object_ids_[object.get()] = serialized_id;
      bool success = SetSerializedObjectID(id, serialized_id) && WriteObject(id, *object);
      return EndOperation(success, SerialiserError::SERIALISE_CALLBACK_FAILED,
                          "Could not write shared object definition", id);
    }

    /**
     * @brief Reconstructs a shared object while preserving aliases and cycles.
     *
     * A placeholder is cached before Deserialise runs, allowing recursive
     * references to observe the same object while it is being populated.
     */
    template <typename T>
    bool ReadSharedObject(NodeID_t id, std::shared_ptr<T> &object)
    {
      BeginOperation();
      Node *node = GetNode(id);
      if (node == nullptr || node->GetType() != Type::OBJECT)
      {
        return EndOperation(false, node == nullptr ? SerialiserError::INVALID_NODE_ID : SerialiserError::INVALID_NODE_TYPE,
                            "Source node is not an object", id);
      }

      uint32_t serialized_id = node->GetObjectReferenceID();
      bool is_reference = serialized_id != 0;
      if (!is_reference)
      {
        serialized_id = node->GetSerializedObjectID();
      }
      if (serialized_id == 0)
      {
        return EndOperation(false, SerialiserError::INVALID_OBJECT_REFERENCE,
                            "Shared object has no serialized identity", id);
      }

      auto existing = read_object_cache_.find(serialized_id);
      if (existing == read_object_cache_.end())
      {
        ObjectCacheEntry entry;
        entry.type_name = T::TypeName();
        entry.object = std::make_shared<T>();
        // Cache before population to resolve forward references and cycles.
        read_object_cache_[serialized_id] = entry;
        existing = read_object_cache_.find(serialized_id);
      }
      if (existing->second.type_name != T::TypeName())
      {
        return EndOperation(false, SerialiserError::OBJECT_TYPE_MISMATCH,
                            "Shared object type does not match the requested type", id);
      }

      object = std::static_pointer_cast<T>(existing->second.object);
      if (is_reference || existing->second.complete || existing->second.populating)
      {
        return EndOperation(true, SerialiserError::DESERIALISE_CALLBACK_FAILED,
                            std::string(), id);
      }

      existing->second.populating = true;
      if (!ReadObject(id, *object))
      {
        existing->second.populating = false;
        return EndOperation(false, SerialiserError::DESERIALISE_CALLBACK_FAILED,
                            "Could not read shared object definition", id);
      }
      existing->second.populating = false;
      existing->second.complete = true;
      return EndOperation(true, SerialiserError::DESERIALISE_CALLBACK_FAILED,
                          std::string(), id);
    }

    /** @brief Reports whether every requested shared object was fully populated. */
    bool SharedObjectsResolved() const
    {
      BeginOperation();
      for (const auto &pair : read_object_cache_)
      {
        if (!pair.second.complete)
        {
          return EndOperation(false, SerialiserError::INVALID_OBJECT_REFERENCE,
                              "A shared object was referenced but not populated");
        }
      }
      return EndOperation(true, SerialiserError::INVALID_OBJECT_REFERENCE, std::string());
    }

    /** @brief Validates that serialized IDs are unique and every reference resolves. */
    bool ValidateObjectReferences() const
    {
      BeginOperation();
      std::unordered_map<uint32_t, bool> definitions;
      for (const auto &pair : node_map_)
      {
        uint32_t serialized_id = pair.second->GetSerializedObjectID();
        if (serialized_id != 0 && definitions.count(serialized_id) != 0)
        {
          return EndOperation(false, SerialiserError::INVALID_OBJECT_REFERENCE,
                              "Serialized object identity is defined more than once",
                              pair.first);
        }
        if (serialized_id != 0)
        {
          definitions[serialized_id] = true;
        }
      }

      for (const auto &pair : node_map_)
      {
        uint32_t reference_id = pair.second->GetObjectReferenceID();
        if (reference_id != 0 && definitions.count(reference_id) == 0)
        {
          return EndOperation(false, SerialiserError::INVALID_OBJECT_REFERENCE,
                              "Serialized object reference has no definition",
                              pair.first);
        }
      }
      return EndOperation(true, SerialiserError::INVALID_OBJECT_REFERENCE, std::string());
    }
  };

  /** @brief Owns a registry-created object without requiring RTTI. */
  class RegisteredObject
  {
  private:
    std::string type_name_;
    std::shared_ptr<void> object_;

  public:
    RegisteredObject() : type_name_(), object_() {}
    RegisteredObject(const std::string &type_name, const std::shared_ptr<void> &object)
        : type_name_(type_name), object_(object)
    {
    }

    /** @brief Reports whether factory construction and deserialization succeeded. */
    bool IsValid() const { return object_ != nullptr; }
    /** @brief Returns the stable registered type name. */
    const std::string &GetTypeName() const { return type_name_; }

    /** @brief Returns the object as T when its explicit type name matches. */
    template <typename T>
    T *As()
    {
      return type_name_ == T::TypeName() ? static_cast<T *>(object_.get()) : nullptr;
    }

    /** @brief Returns a read-only T pointer when the registered type matches. */
    template <typename T>
    const T *As() const
    {
      return type_name_ == T::TypeName() ? static_cast<const T *>(object_.get()) : nullptr;
    }
  };

  /** @brief Maps stable type names to executable-local object factories. */
  class ObjectRegistry
  {
  private:
    typedef std::function<std::shared_ptr<void>(Serialiser &, Serialiser::NodeID_t)> Factory;
    std::unordered_map<std::string, Factory> factories_;

  public:
    /** @brief Registers the factory and static deserializer for T. */
    template <typename T>
    bool Register()
    {
      const std::string type_name = T::TypeName();
      if (type_name.empty() || factories_.count(type_name) != 0)
      {
        return false;
      }

      factories_[type_name] = [](Serialiser &serialiser, Serialiser::NodeID_t id) -> std::shared_ptr<void>
      {
        std::shared_ptr<T> object = std::make_shared<T>();
        if (!serialiser.ReadObject<T>(id, *object))
        {
          return std::shared_ptr<void>();
        }
        return object;
      };
      return true;
    }

    /** @brief Creates the type named by an object node's metadata. */
    RegisteredObject Create(Serialiser &serialiser, Serialiser::NodeID_t id) const
    {
      const std::string &type_name = serialiser.GetObjectType(id);
      auto factory = factories_.find(type_name);
      if (type_name.empty() || factory == factories_.end())
      {
        return RegisteredObject();
      }

      std::shared_ptr<void> object = factory->second(serialiser, id);
      return object == nullptr ? RegisteredObject() : RegisteredObject(type_name, object);
    }
  };

  /**
   * @brief Encodes and decodes the JSON-like s8 plaintext format.
   *
   * `$type` selects registered C++ types, while `$id` and `$ref` preserve
   * shared object identity independently of runtime NodeID_t values.
   * Encoded objects and sequences use separators only between members. The
   * decoder continues to accept legacy object text containing a trailing comma.
   */
  class PlaintextCodec : public Codec
  {
  public:
    /**
     * @brief Formatting options controlling how ToString() renders plaintext.
     *
     * Options are specific to PlaintextCodec; other Codec specialisations would
     * define their own option type rather than sharing this one.
     */
    struct Options
    {
      bool pretty_print = false; ///< Emit newlines and indentation like formatted JSON.
      int indent_width = 2;      ///< Spaces added per nesting level when pretty_print is set.
    };

  protected:
    const char VALUE_START = '"';
    const char VALUE_END = '"';
    const char OBJECT_START = '{';
    const char OBJECT_END = '}';
    const char OBJECT_FIELD_SEPARATOR = ',';
    const char OBJECT_FIELD_KEY_VALUE_SEPARATOR = ':';
    const char SEQUENCE_START = '[';
    const char SEQUENCE_END = ']';
    const char SEQUENCE_SEPARATOR = ',';
    const std::string OBJECT_TYPE_FIELD = "$type";
    const std::string OBJECT_ID_FIELD = "$id";
    const std::string OBJECT_REFERENCE_FIELD = "$ref";

  private:
    Options options_;

    /** @brief Returns indentation whitespace for a given nesting depth. */
    std::string Indent(int depth) const
    {
      return std::string(static_cast<size_t>(depth) * static_cast<size_t>(options_.indent_width > 0 ? options_.indent_width : 0), ' ');
    }

    /** @brief Stateful recursive-descent parser used by FromString. */
    class Parser
    {
    private:
      const PlaintextCodec &codec_;
      const std::string &input_;
      Serialiser &serialiser_;
      size_t position_;

      void SkipWhitespace()
      {
        while (position_ < input_.size() &&
               (input_[position_] == ' ' || input_[position_] == '\t' ||
                input_[position_] == '\n' || input_[position_] == '\r'))
        {
          ++position_;
        }
      }

      bool Consume(char expected)
      {
        SkipWhitespace();
        if (position_ >= input_.size() || input_[position_] != expected)
        {
          return false;
        }
        ++position_;
        return true;
      }

      bool ParseQuotedString(std::string &value)
      {
        if (!Consume(codec_.VALUE_START))
        {
          return false;
        }

        value.clear();
        while (position_ < input_.size())
        {
          char current = input_[position_++];
          if (current == codec_.VALUE_END)
          {
            return true;
          }
          if (current == '\\')
          {
            if (position_ >= input_.size())
            {
              return false;
            }
            current = input_[position_++];
            if (current == 'n')
              current = '\n';
            else if (current == 'r')
              current = '\r';
            else if (current == 't')
              current = '\t';
            else if (current != codec_.VALUE_END && current != '\\')
              return false;
          }
          value += current;
        }
        return false;
      }

      /** @brief Parses a positive serialized object identity from quoted text. */
      bool ParseObjectID(uint32_t &id)
      {
        std::string value;
        if (!ParseQuotedString(value))
        {
          return false;
        }

        std::istringstream input(value);
        if (!(input >> id) || id == 0)
        {
          return false;
        }
        input >> std::ws;
        return input.eof();
      }

      /** @brief Ensures reference-only nodes contain no definition data. */
      bool ObjectIsValid(Serialiser::NodeID_t object_id)
      {
        Node *node = serialiser_.GetNode(object_id);
        return node != nullptr &&
               (node->GetObjectReferenceID() == 0 ||
                (!node->HasFields() && node->GetObjectTypeName().empty() &&
                 node->GetSerializedObjectID() == 0));
      }

      Type NextType()
      {
        SkipWhitespace();
        if (position_ >= input_.size())
          return Type::NONE;
        if (input_[position_] == codec_.OBJECT_START)
          return Type::OBJECT;
        if (input_[position_] == codec_.SEQUENCE_START)
          return Type::SEQUENCE;
        if (input_[position_] == codec_.VALUE_START)
          return Type::SCALAR;
        return Type::NONE;
      }

      bool ParseValue(Serialiser::NodeID_t node_id, Type type)
      {
        if (node_id == Serialiser::INVALID_ID)
          return false;

        if (type == Type::OBJECT)
          return ParseObject(node_id);
        if (type == Type::SEQUENCE)
          return ParseSequence(node_id);
        if (type == Type::SCALAR)
        {
          std::string value;
          return ParseQuotedString(value) && serialiser_.Value<std::string>(node_id, value);
        }
        return false;
      }

      bool ParseObject(Serialiser::NodeID_t object_id)
      {
        if (!Consume(codec_.OBJECT_START))
          return false;

        SkipWhitespace();
        if (Consume(codec_.OBJECT_END))
          return ObjectIsValid(object_id);

        while (position_ < input_.size())
        {
          std::string name;
          if (!ParseQuotedString(name) || !Consume(codec_.OBJECT_FIELD_KEY_VALUE_SEPARATOR))
            return false;

          if (name == codec_.OBJECT_TYPE_FIELD)
          {
            std::string type_name;
            if (!ParseQuotedString(type_name) || !serialiser_.SetObjectType(object_id, type_name))
              return false;
          }
          else if (name == codec_.OBJECT_ID_FIELD)
          {
            uint32_t serialized_id = 0;
            if (!ParseObjectID(serialized_id) ||
                !serialiser_.SetSerializedObjectID(object_id, serialized_id))
              return false;
          }
          else if (name == codec_.OBJECT_REFERENCE_FIELD)
          {
            uint32_t serialized_id = 0;
            if (!ParseObjectID(serialized_id) ||
                !serialiser_.SetObjectReferenceID(object_id, serialized_id))
              return false;
          }
          else
          {
            Type type = NextType();
            Serialiser::NodeID_t child_id = serialiser_.AddField(object_id, name, type);
            if (!ParseValue(child_id, type))
              return false;
          }

          SkipWhitespace();
          if (Consume(codec_.OBJECT_END))
            return ObjectIsValid(object_id);
          if (!Consume(codec_.OBJECT_FIELD_SEPARATOR))
            return false;

          // Continue accepting plaintext produced by older trailing-comma encoders.
          SkipWhitespace();
          if (Consume(codec_.OBJECT_END))
            return ObjectIsValid(object_id);
        }
        return false;
      }

      bool ParseSequence(Serialiser::NodeID_t sequence_id)
      {
        if (!Consume(codec_.SEQUENCE_START))
          return false;

        SkipWhitespace();
        if (Consume(codec_.SEQUENCE_END))
          return true;

        while (position_ < input_.size())
        {
          Type type = NextType();
          Serialiser::NodeID_t child_id = serialiser_.AddElement(sequence_id, type);
          if (!ParseValue(child_id, type))
            return false;

          SkipWhitespace();
          if (Consume(codec_.SEQUENCE_END))
            return true;
          if (!Consume(codec_.SEQUENCE_SEPARATOR))
            return false;
        }
        return false;
      }

    public:
      Parser(const PlaintextCodec &codec, const std::string &input, Serialiser &serialiser)
          : codec_(codec), input_(input), serialiser_(serialiser), position_(0)
      {
      }

      /** @brief Parses the complete document and validates graph references. */
      bool Parse()
      {
        if (NextType() != Type::OBJECT ||
            !ParseValue(serialiser_.GetRootID(), Type::OBJECT))
        {
          return false;
        }
        SkipWhitespace();
        return position_ == input_.size() && serialiser_.ValidateObjectReferences();
      }
    };

  public:
    PlaintextCodec() : Codec(), options_() {}
    explicit PlaintextCodec(const Options &options) : Codec(), options_(options) {}
    virtual ~PlaintextCodec() {}

    /** @brief Returns the formatting options currently in effect. */
    const Options &GetOptions() const { return options_; }
    /** @brief Replaces the formatting options used by subsequent ToString() calls. */
    void SetOptions(const Options &options) { options_ = options; }

    /**
     * @brief Converts a node tree to plaintext without modifying the tree.
     * @return Success, or a CodecResult identifying the failing child node.
     */
    CodecResult ToString(const Node *s, std::string &str) const override
    {
      return ToString(s, str, 0);
    }

  private:
    /**
     * @brief Converts a node tree to plaintext, indenting by @p depth when pretty-printing.
     * @return Success, or a CodecResult identifying the failing child node.
     */
    CodecResult ToString(const Node *s, std::string &str, int depth) const
    {
      std::ostringstream oss;

      if (s == nullptr)
      {
        return CodecResult(CodecError::UNEXPECTED_NULL, "Cannot serialise a null node");
      }
      switch (s->GetType())
      {
      case Type::SCALAR:
      {
        oss << VALUE_START << s->GetScalarStr() << VALUE_END;
        break;
      }
      case Type::SEQUENCE:
      {
        oss << SEQUENCE_START;
        const std::vector<Node *> *sequence = s->GetSequence();
        if (sequence == nullptr || sequence->empty())
        {
          oss << SEQUENCE_END;
          break;
        }
        if (options_.pretty_print)
          oss << "\n";
        for (size_t i = 0; i < sequence->size(); ++i)
        {
          const Node *n = (*sequence)[i];
          std::string child_string;
          CodecResult result = this->ToString(n, child_string, depth + 1);
          if (!result.Ok())
          {
            result.error_message = "Sequence element " + std::to_string(i) + ": " + result.error_message;
            return result;
          }
          if (options_.pretty_print)
            oss << Indent(depth + 1);
          oss << child_string;
          if (i < sequence->size() - 1)
            oss << SEQUENCE_SEPARATOR;
          if (options_.pretty_print)
            oss << "\n";
        }
        if (options_.pretty_print)
          oss << Indent(depth);
        oss << SEQUENCE_END;
        break;
      }
      case Type::OBJECT:
      {
        auto make_member = [this](const std::string &key, const std::string &value_text)
        {
          std::ostringstream member;
          member << VALUE_START << key << VALUE_END << OBJECT_FIELD_KEY_VALUE_SEPARATOR;
          if (options_.pretty_print)
            member << ' ';
          member << value_text;
          return member.str();
        };

        if (s->GetObjectReferenceID() != 0)
        {
          std::ostringstream ref_value;
          ref_value << VALUE_START << s->GetObjectReferenceID() << VALUE_END;
          oss << OBJECT_START;
          if (options_.pretty_print)
            oss << "\n"
                << Indent(depth + 1);
          oss << make_member(OBJECT_REFERENCE_FIELD, ref_value.str());
          if (options_.pretty_print)
            oss << "\n"
                << Indent(depth);
          oss << OBJECT_END;
          break;
        }

        std::vector<std::string> members;
        if (s->GetSerializedObjectID() != 0)
        {
          std::ostringstream id_value;
          id_value << VALUE_START << s->GetSerializedObjectID() << VALUE_END;
          members.push_back(make_member(OBJECT_ID_FIELD, id_value.str()));
        }
        if (!s->GetObjectTypeName().empty())
        {
          members.push_back(make_member(OBJECT_TYPE_FIELD, VALUE_START + s->GetObjectTypeName() + VALUE_END));
        }
        if (s->HasFields())
        {
          for (const auto &pair : *(s->GetFields()))
          {
            std::string child_string;
            CodecResult result = this->ToString(pair.second, child_string, depth + 1);
            if (!result.Ok())
            {
              result.error_message = "Object field '" + pair.first + "': " + result.error_message;
              return result;
            }
            members.push_back(make_member(pair.first, child_string));
          }
        }

        oss << OBJECT_START;
        if (!members.empty())
        {
          if (options_.pretty_print)
            oss << "\n";
          for (size_t i = 0; i < members.size(); ++i)
          {
            if (options_.pretty_print)
              oss << Indent(depth + 1);
            oss << members[i];
            if (i < members.size() - 1)
              oss << OBJECT_FIELD_SEPARATOR;
            if (options_.pretty_print)
              oss << "\n";
          }
          if (options_.pretty_print)
            oss << Indent(depth);
        }
        oss << OBJECT_END;

        break;
      } // end case
      case Type::NONE:
        return CodecResult(CodecError::UNIMPLEMENTED, "Cannot serialise an uninitialized node");
      } // end switch
      str = oss.str();
      return CodecResult();
    }

  public:
    /**
     * @brief Parses plaintext into a Serialiser with fresh runtime node IDs.
     * @return Success, or CodecError::MALFORMED_INPUT with a diagnostic.
     */
    CodecResult
    FromString(const std::string &str, Serialiser &s) const override
    {
      Parser parser(*this, str, s);
      if (!parser.Parse())
      {
        return CodecResult(CodecError::MALFORMED_INPUT, "Malformed plaintext input");
      }
      return CodecResult();
    }
  };
}

#endif
