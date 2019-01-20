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

#include "MeshData.h"

namespace Magnum { namespace Trade {

MeshData::MeshData(Containers::Array<char>&& indexData, Containers::Array<char>&& vertexData, const MeshIndexData indices, Containers::Array<MeshAttributeData>&& attributes, const void* const importerState) noexcept: _indexData{std::move(indexData)}, _vertexData{std::move(vertexData)}, _indices{indices}, _attributes{std::move(attributes)}, _importerState{importerState} {
    /** @todo assert that index data are divisible by type size, same for other types */
    /** @todo assert that there's at least one position? */
    /** @todo assert that all arrays have the same amount of elemenc */
}

MeshData::~MeshData() = default;

MeshData::MeshData(MeshData&&) noexcept = default;

MeshData& MeshData::operator=(MeshData&&) noexcept = default;

UnsignedInt MeshData::indexCount() const {
    switch(_indices.type) {
        case MeshIndexType::UnsignedByte: return _indices.data.size();
        case MeshIndexType::UnsignedShort: return _indices.data.size()/2;
        case MeshIndexType::UnsignedInt: return _indices.data.size()/4;
    }

    CORRADE_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

UnsignedInt MeshData::attributeCount(const MeshAttributeName name) const {
    UnsignedInt count = 0;
    for(const MeshAttributeData& attribute: _attributes)
        if(attribute.name == name) ++count;
    return count;
}

MeshAttributeName MeshData::attributeName(UnsignedInt id) const {
    CORRADE_ASSERT(id < _attributes.size(),
        "Trade::MeshData::attributeName(): index" << id << "out of range for" << _attributes.size() << "attributes", {});
    return _attributes[id].name;
}

MeshAttributeType MeshData::attributeType(UnsignedInt id) const {
    CORRADE_ASSERT(id < _attributes.size(),
        "Trade::MeshData::attributeName(): index" << id << "out of range for" << _attributes.size() << "attributes", {});
    return _attributes[id].type;
}

UnsignedInt MeshData::attributeFor(const MeshAttributeName name, UnsignedInt id) const {
    for(std::size_t i = 0; i != _attributes.size(); ++i)
        if(_attributes[i].name == name && !id--) return i;
    CORRADE_ASSERT(false, "Trade::MeshData: index" << id << "out of bounds for attribute" << name, {});
}

namespace {
    template<class T> Containers::Array<UnsignedInt> convertIndices(const Containers::ArrayView<const char> data) {
        const auto input = Containers::arrayCast<const T>(data);
        Containers::Array<UnsignedInt> output{input.size()};
        for(std::size_t i = 0; i != input.size(); ++i) output[i] = input[i];
        return output;
    }
}

Containers::Array<UnsignedInt> MeshData::indices() const {
    switch(_indices.type) {
        case MeshIndexType::UnsignedByte: return convertIndices<UnsignedByte>(_indices.data);
        case MeshIndexType::UnsignedShort: return convertIndices<UnsignedShort>(_indices.data);
        case MeshIndexType::UnsignedInt: return convertIndices<UnsignedInt>(_indices.data);
    }

    CORRADE_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

Containers::Array<Vector2> MeshData::positions2D(const UnsignedInt id) const {
    const UnsignedInt attributeId = attributeFor(MeshAttributeName::Positions2D, id);
    /** @todo lift this restriction once we have more types */
    CORRADE_INTERNAL_ASSERT(_attributes[attributeId].type == MeshAttributeType::Vector2);
    const auto input = Containers::arrayCast<const T>();
    Containers::Array<Vector2> output
}

Containers::Array<char> MeshData::releaseIndices() {
    _indices.data = nullptr;
    return std::move(_indexData);
}

Containers::Array<char> MeshData::releaseVertices() {
    _attributes = nullptr;
    return std::move(_vertexData);
}

Debug& operator<<(Debug& debug, const MeshAttributeType value) {
    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case MeshAttributeType::value: return debug << "Trade::MeshAttributeType::" #value;
        _c(Vector2)
        _c(Vector3)
        _c(Vector4)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "Trade::MeshAttributeType(" << Debug::nospace << reinterpret_cast<void*>(UnsignedByte(value)) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const MeshAttributeName value) {
    if(UnsignedByte(value) >= UnsignedByte(MeshAttributeName::Custom))
        return debug << "Trade::MeshAttributeName::Custom(" << Debug::nospace << UnsignedByte(value) << Debug::nospace << ")";

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case MeshAttributeName::value: return debug << "Trade::MeshAttributeName::" #value;
        _c(Positions2D)
        _c(Positions3D)
        _c(Normals)
        _c(TextureCoordinates2D)
        _c(Colors)
        #undef _c
        /* LCOV_EXCL_STOP */

        /* To silence compiler warning about unhandled values */
        case MeshAttributeName::Custom: CORRADE_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
    }

    return debug << "Trade::MeshAttributeName(" << Debug::nospace << reinterpret_cast<void*>(UnsignedByte(value)) << Debug::nospace << ")";
}

}}
