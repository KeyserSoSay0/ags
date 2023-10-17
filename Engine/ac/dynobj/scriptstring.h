//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
#ifndef __AC_SCRIPTSTRING_H
#define __AC_SCRIPTSTRING_H

#include <memory>
#include "ac/dynobj/cc_agsdynamicobject.h"

struct ScriptString final : AGSCCDynamicObject
{
public:
    struct Header
    {
        uint32_t Length = 0u;
    };

    struct Buffer
    {
        friend ScriptString;
    public:
        Buffer() = default;
        ~Buffer() = default;
        Buffer(Buffer &&buf) = default;
        char *Get() { return reinterpret_cast<char*>(_buf.get() - MemHeaderSz); }
        size_t GetSize() const { return _sz; }

    private:
        Buffer(std::unique_ptr<uint8_t[]> &&buf, size_t buf_sz)
            : _buf(std::move(buf)), _sz(buf_sz) {}

        std::unique_ptr<uint8_t[]> _buf;
        size_t _sz;
    };


    ScriptString() = default;
    ~ScriptString() = default;

    inline static const Header &GetHeader(const void *address)
    {
        return reinterpret_cast<const Header&>(*(static_cast<const uint8_t*>(address) - MemHeaderSz));
    }

    // 
    static Buffer CreateBuffer(size_t data_sz);
    // Create a new script string by copying the given text
    static DynObjectRef Create(const char *text);
    //
    static DynObjectRef Create(Buffer &&strbuf);

    const char *GetType() override;
    int Dispose(void *address, bool force) override;
    void Unserialize(int index, AGS::Common::Stream *in, size_t data_sz) override;

private:
    friend ScriptString::Buffer;
    // The size of the array's header in memory, prepended to the element data
    static const size_t MemHeaderSz = sizeof(Header);
    // The size of the serialized header
    static const size_t FileHeaderSz = sizeof(uint32_t);

    static DynObjectRef CreateObject(uint8_t *buf);

    // Savegame serialization
    // Calculate and return required space for serialization, in bytes
    size_t CalcSerializeSize(const void *address) override;
    // Write object data into the provided stream
    void Serialize(const void *address, AGS::Common::Stream *out) override;
};

extern ScriptString myScriptStringImpl;

#endif // __AC_SCRIPTSTRING_H
