#ifndef Magnum_Trade_MeshData_h
#define Magnum_Trade_MeshData_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class @ref Magnum::Trade::MeshData, @ref Magnum::Trade::MeshIndexData, @ref Magnum::Trade::MeshAttributeData, enum @ref Magnum::Trade::MeshAttributeName, @ref Magnum::Trade::MeshAttributeType
 */

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/StridedArrayView.h>

#include "Magnum/Mesh.h"
#include "Magnum/Trade/Trade.h"
#include "Magnum/Trade/visibility.h"

namespace Magnum { namespace Trade {

/**
@brief Mesh attribute type

@see @ref MeshData, @ref MeshAttributeData, @ref MeshAttributeName
*/
enum class MeshAttributeType: UnsignedByte {
    /**
     * @ref Magnum::Vector2 "Vector2". Usually used for
     * @ref MeshAttributeName::Positions2D and
     * @ref MeshAttributeName::TextureCoordinates2D.
     */
    Vector2,

    /**
     * @ref Magnum::Vector3 "Vector3" or @ref Magnum::Color3 "Color3". Usually
     * used for @ref MeshAttributeName::Positions3D,
     * @ref MeshAttributeName::Normals and @ref MeshAttributeName::Colors.
     */
    Vector3,

    /**
     * @ref Magnum::Vector4 "Vector4" or @ref Magnum::Color4 "Color4". Usually
     * used for @ref MeshAttributeName::Colors.
     */
    Vector4
};

/** @debugoperatorenum{MeshAttributeType} */
MAGNUM_TRADE_EXPORT Debug& operator<<(Debug& debug, MeshAttributeType value);

/**
@brief Mesh attribute name

@see @ref MeshData, @ref MeshAttributeData, @ref MeshAttributeType
*/
enum class MeshAttributeName: UnsignedByte {
    /**
     * 2D positions. Type is usually @ref Magnum::Vector2 "Vector2".
     * @see @ref MeshAttributeType::Vector2, @ref MeshData::positions2D()
     */
    Positions2D,

    /**
     * 3D positions. Type is usually @ref Magnum::Vector3 "Vector3".
     * @see @ref MeshAttributeType::Vector3, @ref MeshData::positions3D()
     */
    Positions3D,

    /**
     * Normals. Type is usually @ref Magnum::Vector3 "Vector3".
     * @see @ref MeshAttributeType::Vector3, @ref MeshData::normals()
     */
    Normals,

    /**
     * 2D texture coordinates. Type is usually @ref Magnum::Vector2 "Vector2".
     * @see @ref MeshAttributeType::Vector2,
     *      @ref MeshData::textureCoordinates2D()
     */
    TextureCoordinates2D,

    /**
     * Vertex colors. Type is usually @ref Magnum::Vector3 "Vector3" or
     * @ref Magnum::Vector4 "Vector4" (or @ref Color3 / @ref Color4).
     * @see @ref MeshAttributeType::Vector3,
     *      @ref MeshAttributeType::Vector4,
     *      @ref MeshData::colors()
     */
    Colors,

    /**
     * This and all higher values are for importer-specific attributes. Can be
     * of any type. See documentation of a particular importer for details.
     */
    Custom = 128,
};

/** @debugoperatorenum{MeshAttributeName} */
MAGNUM_TRADE_EXPORT Debug& operator<<(Debug& debug, MeshAttributeName value);

/**
@brief Mesh index data

Convenience type for populating @ref MeshData. Has no accessors, as the data
are then accessible through @ref MeshData APIs.
@see @ref MeshAttributeData
*/
class MeshIndexData {
    public:
        /**
         * @brief Construct with a runtime-specified index type
         * @param type      Mesh index type
         * @param data      Index data
         *
         * The @p data size is expected to correspond to given @p type (e.g.,
         * for @ref MeshIndexType::UnsignedInt the @p data array size should
         * be divisible by 4). If you know the @p type at compile time, you can
         * use one of the @ref MeshIndexData(Containers::ArrayView<UnsignedByte>),
         * @ref MeshIndexData(Containers::ArrayView<UnsignedShort>) or
         * @ref MeshIndexData(Containers::ArrayView<UnsignedInt>) constructors,
         * which infer the index type automatically.
         */
        /*implicit*/ MeshIndexData(MeshIndexType type, Containers::ArrayView<char> data) noexcept: type{type}, data{reinterpret_cast<const Containers::ArrayView<char>&>(data)} {}

        /** @brief Construct with unsigned byte indices */
        /*implicit*/ MeshIndexData(Containers::ArrayView<UnsignedByte> data) noexcept: type{MeshIndexType::UnsignedByte}, data{reinterpret_cast<const Containers::ArrayView<char>&>(data)} {}

        /** @brief Construct with unsigned short indices */
        /*implicit*/ MeshIndexData(Containers::ArrayView<UnsignedShort> data) noexcept: type{MeshIndexType::UnsignedShort}, data{reinterpret_cast<const Containers::ArrayView<char>&>(data)} {}

        /** @brief Construct with unsigned int indices */
        /*implicit*/ MeshIndexData(Containers::ArrayView<UnsignedInt> data) noexcept: type{MeshIndexType::UnsignedInt}, data{reinterpret_cast<const Containers::ArrayView<char>&>(data)} {}

        /** @todo here will go meshlet things as well */

    private:
        /* Not prefixed with _ because use them like public in MeshData */
        friend MeshData;
        MeshIndexType type;
        Containers::ArrayView<char> data;
};

/*
decisions:

- make all data non-const because the 90% use case is to load them from a file
  and later operate on them (pretransforming, vertex reordering, ...), so we
  want to preserve that
- the use case of zero-copy import from a RO (or RO mmaped) memory is the
  remaining 10%. Even in that case we might want to mmap RW and perform
  optimization directly on the data.

TODO: apply the same decision on animationdata
*/

/**
@brief Mesh attribute data

Convenience type for populating @ref MeshData. Has no accessors, as the data
are then accessible through @ref MeshData APIs.
*/
class MeshAttributeData {
    public:
        /**
         * @brief Default constructor
         *
         * Leaves contents at unspecified values. Provided as a convenience for
         * initialization of the attribute array for @ref MeshData, expected to
         * be replaced with concrete values later.
         */
        /*implicit*/ MeshAttributeData() noexcept: name{}, type{}, data{} {}

        /**
         * @brief Constructor
         * @param name      Attribute name
         * @param type      Attribute type
         * @param data      Attribute data
         */
        template<class T> /*implicit*/ MeshAttributeData(MeshAttributeName name, MeshAttributeType type, Containers::StridedArrayView<T> data) noexcept: name{name}, type{type}, data{reinterpret_cast<const Containers::StridedArrayView<char>&>(data)} {}

        // TODO: provide a type-erased version

    private:
        /* Not prefixed with _ because use them like public in MeshData */
        friend MeshData;
        MeshAttributeName name;
        MeshAttributeType type;
        Containers::StridedArrayView<char> data;
};

/**
@brief Mesh data

*/
class MAGNUM_TRADE_EXPORT MeshData {
    public:
        /**
         * @brief Constructor
         * @param indexData     Index data. Use @cpp nullptr @ce if the index
         *      data are not owned by the instance (for example a memory-mapped
         *      file) or if they are lumped together with @p vertexData.
         * @param vertexData    Vertex data. Use @cpp nullptr @ce if the vertex
         *      data are not owned by the instance (for example a memory-mapped
         *      file).
         * @param indices       Index data description
         * @param attributes    Description of all vertex attribute data
         * @param importerState Importer-specific state
         *
         * If @p indexData is not @cpp nullptr @ce, @p indices are expected
         * to point to it. Similarly, if @p vertexData is not @cpp nullptr @ce,
         * @p attributes are expected to reference sub-views of the array. The
         * constructor also checks all expected conditions described in
         * @ref MeshIndexData and @ref MeshAttributeData documentation.
         */
        // TODO: assert on everything here
        explicit MeshData(Containers::Array<char>&& indexData, Containers::Array<char>&& vertexData, MeshIndexData indices, Containers::Array<MeshAttributeData>&& attributes, const void* importerState = nullptr) noexcept;

        // TODO: meshlets: begin & count of unique vertices + begin & count of primitive indices ... needs a new array?

        // TODO: meshlets, one possibility
        // - offsets into the vertex buffer for pulling vertex data, >8bit usually
        // - offsets into the vertex buffer offsets for combining them into primitives, 8bit is enough
        // - a counting offset array saying how many vertices are there for each meshlet (size of the array - 1 is meshlet count)
        // - a counting offset array saying how many tris (size of the array - 1 is meshlet count)
        // - an API that spits out
        //  Meshlet {
        //      arrayview vertices
        //      arrayview prims
        //  }
        // for each meshlet
        // - ability to convert all this back into a linear index buffer (should this go to MeshTools instead?)

        ~MeshData();

        /** @brief Copying is not allowed */
        MeshData(const MeshData&) = delete;

        /** @brief Move constructor */
        MeshData(MeshData&&) noexcept;

        /** @brief Copying is not allowed */
        MeshData& operator=(const MeshData&) = delete;

        /** @brief Move assignment */
        MeshData& operator=(MeshData&&) noexcept;

        /**
         * @brief Raw index data
         *
         * Owned index data. Returns @cpp nullptr @ce if the mesh is
         * non-indexed or the data are not owned (for example when the
         * instance describes a memory-mapped mesh data).
         * @see @ref isIndexed(), @ref indexCount(), @ref indexType(),
         *      @ref indices(), @ref releaseIndices()
         */
        Containers::ArrayView<char> indexData() & { return _indexData; }
        Containers::ArrayView<char> indexData() && = delete; /**< @overload */

        /** @overload */
        Containers::ArrayView<const char> indexData() const & { return _indexData; }
        Containers::ArrayView<const char> indexData() const && = delete; /**< @overload */

        /**
         * @brief Raw vertex data
         *
         * Contains data for all vertex attributes. Returns @cpp nullptr @ce if
         * the vertex data are not owned by the mesh data (for example when the
         * instance describes a memory-mapped mesh data).
         * @see @ref attributeCount(), @ref attributeName(),
         *      @ref attributeType(), @ref attribute(), @ref releaseVertices()
         */
        Containers::ArrayView<char> vertexData() & { return _vertexData; }
        Containers::ArrayView<char> vertexData() && = delete; /**< @overload */

        /** @overload */
        Containers::ArrayView<const char> vertexData() const & { return _vertexData; }
        Containers::ArrayView<const char> vertexData() const && = delete; /**< @overload */

        /** @brief Whether the mesh is indexed */
        bool isIndexed() const { return _indices.data; }

        /**
         * @brief Index count
         *
         * @see @ref indexType(), @ref indices()
         */
        UnsignedInt indexCount() const;

        /**
         * @brief Index type
         *
         * If the mesh is not indexed, returns a default-constructed value.
         * @see @ref isIndexed()
         */
        MeshIndexType indexType() const { return _indices.type; }

        /**
         * @brief Mesh indices
         *
         * Returns @cpp nullptr @ce if the mesh is not indexed. Expects that
         * @p T corresponds to @ref indexType(). You can also use the
         * non-templated @ref indices() accessor to get indices converted to
         * 32-bit, but note that such operation involves extra data conversion
         * and an allocation.
         * @see @ref attribute()
         */
        template<class T> Containers::ArrayView<T> indices() const;

        /**
         * @brief Mesh vertex count
         *
         * Count of elements in every attribute array returned by
         * @ref attribute(). Note that this is different from
         * @ref attributeCount(), which returns count of attribute arrays. If
         * a mesh has no attributes, returns @cpp 0 @ce.
         */
        UnsignedInt vertexCount() const{
            return _attributes ? _attributes[0].data.size() : 0;
        }

        /**
         * @brief Attribute array count
         *
         * Note that this is different from @ref vertexCount(), which returns
         * count of vertices in the mesh.
         * @see @ref indexCount(), @ref attributeCount(MeshAttributeName) const
         */
        UnsignedInt attributeCount() const { return _attributes.size(); }

        /**
         * @brief Whether the mesh has given attribute
         *
         * @see @ref attributeCount()
         */
        bool hasAttribute(MeshAttributeName name) const {
            return attributeCount(name);
        }

        /**
         * @brief Count of given named attribute
         *
         * Unlike @ref attributeCount() const this returns count for given
         * attribute name --- for example a mesh can have more sets of UV
         * texture coordinates.
         * @see @ref hasAttribute(MeshAttributeName) const
         */
        UnsignedInt attributeCount(MeshAttributeName name) const;

        /**
         * @brief Name of an attribute
         *
         * The @p id is expected to be smaller than @ref attributeCount() const.
         * @see @ref attributeType()
         */
        MeshAttributeName attributeName(UnsignedInt id) const;

        /**
         * @brief Type of an attribute
         *
         * The @p is expected to be smaller than @ref attributeName(). You can
         * also use @ref attributeType(MeshAttributeName, UnsignedInt) const to
         * directly get a type of given named attribute.
         * @see @ref attributeName()
         */
        MeshAttributeType attributeType(UnsignedInt id) const;

        /**
         * @brief Type of a named attribute
         *
         * The @p id is expected to be smaller than
         * @ref attributeCount(MeshAttributeName) const.
         * @see @ref attributeType(UnsignedInt) const
         */
        MeshAttributeType attributeType(MeshAttributeName name, UnsignedInt id) const {
            return attributeType(attributeFor(name, id));
        }

        /**
         * @brief Data for given attribute array
         *
         * The @p id is expected to be smaller than @ref attributeCount() const
         * and @p T is expected to correspond to
         * @ref attributeType(UnsignedInt) const.
         * @see @ref attribute(MeshAttributeName, UnsignedInt) const
         */
        template<class T> Containers::StridedArrayView<T> attribute(UnsignedInt id) const;

        /**
         * @brief Data for given named attribute array
         *
         * The @p id is expected to be smaller than
         * @ref attributeCount(MeshAttributeName) const and @p T is expected to
         * correspond to @ref attributeType(MeshAttributeName, UnsignedInt) const.
         * You can also use the non-templated @ref positions2D(),
         * @ref positions3D(), @ref textureCoordinates2D() and @ref colors()
         * accessors to get the often used attributes in a commonly used type,
         * but note that such operation involves extra data conversion and an
         * allocation.
         * @see @ref attribute(UnsignedInt) const
         */
        template<class T> Containers::StridedArrayView<T> attribute(MeshAttributeName name, UnsignedInt id) const {
            return attribute<T>(attributeFor(name, id));
        }

        /**
         * @brief Indices as 32-bit integers
         *
         * Convenience alternative to the templated @ref indices(). Converts
         * the index array from an arbitrary storage type and returns it in a
         * newly allocated instance.
         */
        Containers::Array<UnsignedInt> indices() const;

        /**
         * @brief Positions as 2D float vectors
         *
         * Convenience alternative to @ref attribute(MeshAttributeName, UnsignedInt) const
         * with @ref MeshAttributeName::Positions2D as the first argument.
         * Converts the position array from an arbitrary underlying type and
         * returns it in a newly allocated instance.
         */
        Containers::Array<Vector2> positions2D(UnsignedInt id) const;

        /**
         * @brief Positions as 3D float vectors
         *
         * Convenience alternative to @ref attribute(MeshAttributeName, UnsignedInt) const
         * with @ref MeshAttributeName::Positions3D as the first argument.
         * Converts the position array from an arbitrary underlying type and
         * returns it in a newly allocated instance.
         */
        Containers::Array<Vector3> positions3D(UnsignedInt id) const;

        /**
         * @brief Normals as 3D float vectors
         *
         * Convenience alternative to @ref attribute(MeshAttributeName, UnsignedInt) const
         * with @ref MeshAttributeName::Normals as the first argument. Converts
         * the position array from an arbitrary underlying type and returns it
         * in a newly allocated instance.
         */
        Containers::Array<Vector3> normals(UnsignedInt id) const;

        /**
         * @brief 2D texture coordinates as float vectors
         *
         * Convenience alternative to @ref attribute(MeshAttributeName, UnsignedInt) const
         * with @ref MeshAttributeName::TextureCoordinates2D as the first
         * argument. Converts the position array from an arbitrary underlying
         * type and returns it in a newly allocated instance.
         */
        Containers::Array<Vector2> textureCoordinates2D(UnsignedInt id) const;

        /**
         * @brief Colors as RGBA floats
         *
         * Convenience alternative to @ref attribute(MeshAttributeName, UnsignedInt) const
         * with @ref MeshAttributeName::Colors as the first argument. Converts
         * the position array from an arbitrary underlying type and returns it
         * in a newly allocated instance.
         */
        Containers::Array<Color4> colors(UnsignedInt id) const;

        /**
         * @brief Release index data storage
         *
         * Releases the ownership of the index data array and resets internal
         * index-related state to default. The mesh then behaves like
         * non-indexed.
         * @see @ref indexData()
         */
        Containers::Array<char> releaseIndices();

        /**
         * @brief Release vertex data storage
         *
         * Releases the ownership of the index data array and resets internal
         * attribute-related state to default. The mesh then behaves like if
         * it has no attributes.
         * @see @ref vertexData()
         */
        Containers::Array<char> releaseVertices();

        /**
         * @brief Importer-specific state
         *
         * See @ref AbstractImporter::importerState() for more information.
         */
        const void* importerState() const { return _importerState; }

    private:
        UnsignedInt attributeFor(MeshAttributeName name, UnsignedInt id) const;

        Containers::Array<char> _indexData, _vertexData;

        MeshIndexData _indices;
        Containers::Array<MeshAttributeData> _attributes;
        MeshPrimitive _primitive;
        MeshIndexType _indexType;

        const void* _importerState;
};

#if !defined(CORRADE_NO_ASSERT) || defined(CORRADE_GRACEFUL_ASSERT)
namespace Implementation {
    /* LCOV_EXCL_START */
    template<class> constexpr MeshIndexType meshIndexTypeFor();
    template<> constexpr MeshIndexType meshIndexTypeFor<UnsignedByte>() { return MeshIndexType::UnsignedByte; }
    template<> constexpr MeshIndexType meshIndexTypeFor<UnsignedShort>() { return MeshIndexType::UnsignedShort; }
    template<> constexpr MeshIndexType meshIndexTypeFor<UnsignedInt>() { return MeshIndexType::UnsignedInt; }

    template<class> constexpr MeshAttributeType meshAttributeTypeFor();
    template<> constexpr MeshAttributeType meshAttributeTypeFor<Vector2>() { return MeshAttributeType::Vector2; }
    template<> constexpr MeshAttributeType meshAttributeTypeFor<Vector3>() { return MeshAttributeType::Vector3; }
    template<> constexpr MeshAttributeType meshAttributeTypeFor<Color3>() { return MeshAttributeType::Vector3; }
    template<> constexpr MeshAttributeType meshAttributeTypeFor<Vector4>() { return MeshAttributeType::Vector4; }
    template<> constexpr MeshAttributeType meshAttributeTypeFor<Color4>() { return MeshAttributeType::Vector4; }
    /* LCOV_EXCL_STOP */
}
#endif

template<class T> Containers::ArrayView<T> MeshData::indices() const {
    CORRADE_ASSERT(Implementation::meshIndexTypeFor<T>() == _indices.type,
        "Trade::MeshData::indices(): improper type requested for" << _indices.type, nullptr);
    return Containers::arrayCast<T>(_indices.data);
}

template<class T> Containers::StridedArrayView<T> MeshData::attribute(UnsignedInt id) const {
    CORRADE_ASSERT(id < _attributes.size(),
        "Trade::MeshData::attribute(): index" << id << "out of range for" << _attributes.size() << "attributes", nullptr);
    CORRADE_ASSERT(Implementation::meshAttributeTypeFor<T>() == _attributes[id].type,
        "Trade::MeshData::attribute(): improper type requested for" << _attributes[id].type, nullptr);
    return Containers::arrayCast<T>(_attributes[id].data);
}

}}

#endif
