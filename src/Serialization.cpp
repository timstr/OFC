#include <OFC/Serialization.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <type_traits>
#include <string_view>

namespace ofc {

    namespace detail {

        enum class Flag : std::uint8_t {
            Byte = 0x01,
            Bool = 0x02,
            Uint8 = 0x03,
            Uint16 = 0x04,
            Uint32 = 0x05,
            Uint64 = 0x06,
            Int8 = 0x07,
            Int16 = 0x08,
            Int32 = 0x09,
            Int64 = 0x0A,
            Float32 = 0x0B,
            Float64 = 0x0C,
            ByteString = 0x0D,

            ArrayOf = 0x20,
            ArrayLength = 0x21,

            DynamicObject = 0x30,
            StaticObject = 0x31,

            // TODO: strings?

            // TODO: versioning
        };

        template<Flag F>
        struct flag_size {};

        template<Flag F>
        struct flag_type {};

#define SPECIALIZE_TAG(FLAGNAME, SIZE, TYPE) \
template<> struct flag_size<Flag::FLAGNAME> { \
    static_assert(sizeof(TYPE) == SIZE); \
    static constexpr std::size_t value = SIZE; \
}; \
template<> struct flag_type<Flag::FLAGNAME> { \
    typedef TYPE type; \
};

        SPECIALIZE_TAG(Byte, 1, std::byte);
        SPECIALIZE_TAG(Bool, 1, bool);
        SPECIALIZE_TAG(Uint8, 1, std::uint8_t);
        SPECIALIZE_TAG(Uint16, 2, std::uint16_t);
        SPECIALIZE_TAG(Uint32, 4, std::uint32_t);
        SPECIALIZE_TAG(Uint64, 8, std::uint64_t);
        SPECIALIZE_TAG(Int8, 1, std::int8_t);
        SPECIALIZE_TAG(Int16, 2, std::int16_t);
        SPECIALIZE_TAG(Int32, 4, std::int32_t);
        SPECIALIZE_TAG(Int64, 8, std::int64_t);
        SPECIALIZE_TAG(Float32, 4, float);
        SPECIALIZE_TAG(Float64, 8, double);

#undef SPECIALIZE_TAG

        template<Flag F>
        constexpr std::size_t flag_size_v = flag_size<F>::value;

        template<Flag F>
        using flag_type_t = typename flag_type<F>::type;

        void writeFlag(std::vector<std::byte>& v, Flag f) {
            static_assert(sizeof(Flag) == 1);
            v.push_back(static_cast<std::byte>(f));
        }

        Flag readFlag(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it) {
            static_assert(sizeof(Flag) == 1);
            if (it == end(v)) {
                throw SerializationException{};
            }
            const std::byte* b_ptr = &*it;
            const auto flag = static_cast<Flag>(*b_ptr);
            ++it;
            return flag;
        }

        void readAssertFlag(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it, Flag expected) {
            if (readFlag(v, it) != expected) {
                throw SerializationException{};
            }
        }


        template<Flag F, typename T>
        void writeScalarUnchecked(std::vector<std::byte>& v, const T& x) {
            static_assert(std::is_same_v<flag_type_t<F>, T>);

            const auto x_bptr = reinterpret_cast<const std::byte*>(&x);

            const auto numBytes = flag_size_v<F>;
            if constexpr (numBytes == 1) {
                v.push_back(x_bptr[0]);
            } else if constexpr (numBytes == 2) {
                v.push_back(x_bptr[0]);
                v.push_back(x_bptr[1]);
            } else if constexpr (numBytes == 4) {
                v.push_back(x_bptr[0]);
                v.push_back(x_bptr[1]);
                v.push_back(x_bptr[2]);
                v.push_back(x_bptr[3]);
            } else if constexpr (numBytes == 8) {
                v.push_back(x_bptr[0]);
                v.push_back(x_bptr[1]);
                v.push_back(x_bptr[2]);
                v.push_back(x_bptr[3]);
                v.push_back(x_bptr[4]);
                v.push_back(x_bptr[5]);
                v.push_back(x_bptr[6]);
                v.push_back(x_bptr[7]);
            } else {
                static_assert(std::false_type<T>::value);
            }
        }

        template<Flag F, typename T>
        void writeScalar(std::vector<std::byte>& v, const T& x) {
            static_assert(std::is_same_v<flag_type_t<F>, T>);

            writeFlag(v, F);
            writeScalarUnchecked<F>(v, x);
        }

        template<Flag F>
        flag_type_t<F> readScalarUnchecked(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it) {
            flag_type_t<F> x;
            const auto x_bptr = reinterpret_cast<std::byte*>(&x);
            const std::byte* b_ptr = &*it;

            const auto numBytes = flag_size_v<F>;
            if (distance(it, end(v)) < numBytes) {
                throw SerializationException{};
            }
            if constexpr (numBytes == 1) {
                x_bptr[0] = b_ptr[0];
            } else if constexpr (numBytes == 2) {
                x_bptr[0] = b_ptr[0];
                x_bptr[1] = b_ptr[1];
            } else if constexpr (numBytes == 4) {
                x_bptr[0] = b_ptr[0];
                x_bptr[1] = b_ptr[1];
                x_bptr[2] = b_ptr[2];
                x_bptr[3] = b_ptr[3];
            } else if constexpr (numBytes == 8) {
                x_bptr[0] = b_ptr[0];
                x_bptr[1] = b_ptr[1];
                x_bptr[2] = b_ptr[2];
                x_bptr[3] = b_ptr[3];
                x_bptr[4] = b_ptr[4];
                x_bptr[5] = b_ptr[5];
                x_bptr[6] = b_ptr[6];
                x_bptr[7] = b_ptr[7];
            } else {
                static_assert(std::false_type<F>::value);
            }

            std::advance(it, numBytes);
            return x;
        }

        template<Flag F>
        flag_type_t<F> readScalar(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it) {
            readAssertFlag(v, it, F);
            return readScalarUnchecked<F>(v, it);
        }

        void writeByteString(std::vector<std::byte>& v, std::string_view sv) {
            const auto data = sv.data();
            const auto len = static_cast<std::uint64_t>(sv.size());
            writeFlag(v, Flag::ByteString);
            writeScalarUnchecked<Flag::Uint64>(v, len);
            for (std::uint64_t i = 0; i < len; ++i) {
                writeScalarUnchecked<Flag::Byte>(v, static_cast<std::byte>(data[i]));
            }
        }

        std::string readByteString(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it) {
            std::string x;
            readAssertFlag(v, it, Flag::ByteString);
            const auto len = readScalarUnchecked<Flag::Uint64>(v, it);
            x.resize(len, '\0');
            const auto data = x.data();
            for (std::uint64_t i = 0; i < len; ++i) {
                data[i] = static_cast<char>(readScalarUnchecked<Flag::Byte>(v, it));
            }
            return x;
        }

        template<Flag F>
        void writeSpan(std::vector<std::byte>& v, const flag_type_t<F>* src, std::uint64_t len) {
            writeFlag(v, Flag::ArrayOf);
            writeFlag(v, F);
            writeFlag(v, Flag::ArrayLength);
            writeScalarUnchecked<Flag::Uint64>(v, len);
            for (std::uint64_t i = 0; i < len; ++i) {
                writeScalarUnchecked<F>(v, src[i]);
            }
        }

        std::uint64_t peekSpanLength(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it) {
            const auto prevIt = it;
            readAssertFlag(v, it, Flag::ArrayOf);
            readFlag(v, it); // Array element type is consumed but not checked
            readAssertFlag(v, it, Flag::ArrayLength);
            const auto len = readScalarUnchecked<Flag::Uint64>(v, it);
            it = prevIt;
            return len;
        }

        template<Flag F>
        void readSpan(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it, flag_type_t<F>* dst, std::uint64_t len) {
            readAssertFlag(v, it, Flag::ArrayOf);
            readAssertFlag(v, it, F);
            readAssertFlag(v, it, Flag::ArrayLength);
            if (readScalarUnchecked<Flag::Uint64>(v, it) != len) {
                throw SerializationException{};
            }
            for (std::uint64_t i = 0; i < len; ++i) {
                dst[i] = readScalarUnchecked<F>(v, it);
            }
        }

        template<Flag F>
        std::vector<flag_type_t<F>> readVec(const std::vector<std::byte>& v, std::vector<std::byte>::const_iterator& it) {
            readAssertFlag(v, it, Flag::ArrayOf);
            readAssertFlag(v, it, F);
            readAssertFlag(v, it, Flag::ArrayLength);
            const auto len = readScalarUnchecked<Flag::Uint64>(v, it);
            auto vec = std::vector<flag_type_t<F>>{};
            vec.reserve(len);
            for (std::uint64_t i = 0; i < len; ++i) {
                vec.push_back(readScalarUnchecked<F>(v, it));
            }
            return vec;
        }

    } // namespace detail

    const char* SerializationException::what() const noexcept {
        return "Serialization failed.";
    }

    const std::vector<std::byte>& Serializer::dump() const {
        return m_buffer;
    }

    Serializer& Serializer::b(bool x){
        detail::writeScalar<detail::Flag::Bool>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::u8(std::uint8_t x){
        detail::writeScalar<detail::Flag::Uint8>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::u16(std::uint16_t x){
        detail::writeScalar<detail::Flag::Uint16>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::u32(std::uint32_t x){
        detail::writeScalar<detail::Flag::Uint32>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::u64(std::uint64_t x){
        detail::writeScalar<detail::Flag::Uint64>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::i8(std::int8_t x){
        detail::writeScalar<detail::Flag::Int8>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::i16(std::int16_t x){
        detail::writeScalar<detail::Flag::Int16>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::i32(std::int32_t x){
        detail::writeScalar<detail::Flag::Int32>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::i64(std::int64_t x){
        detail::writeScalar<detail::Flag::Int64>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::f32(float x){
        detail::writeScalar<detail::Flag::Float32>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::f64(double x){
        detail::writeScalar<detail::Flag::Float64>(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::str(std::string_view x){
        detail::writeByteString(m_buffer, x);
        return *this;
    }

    Serializer& Serializer::dynamic_object(const ISerializable* o) {
        using namespace detail;
        detail::writeFlag(m_buffer, Flag::DynamicObject);
        if (o) {
            detail::writeByteString(m_buffer, o->getIdentifier());
            o->serialize(*this);
        } else {
            detail::writeByteString(m_buffer, "");
        }
        return *this;
    }

    Serializer& Serializer::u8_span(const std::uint8_t* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Uint8>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::u16_span(const std::uint16_t* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Uint16>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::u32_span(const std::uint32_t* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Uint32>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::u64_span(const std::uint64_t* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Uint64>(m_buffer, src, len);
        return *this;
    }

    Deserializer::Deserializer(std::vector<std::byte> buffer)
        : m_buffer(std::move(buffer))
        , m_pos(m_buffer.cbegin()) {

        assert(m_pos == m_buffer.begin());
    }

    Deserializer& Deserializer::b(bool& x){
        x = b();
        return *this;
    }

    Deserializer& Deserializer::u8(std::uint8_t& x) {
        x = u8();
        return *this;
    }

    Deserializer& Deserializer::u16(std::uint16_t& x) {
        x = u16();
        return *this;
    }

    Deserializer& Deserializer::u32(std::uint32_t& x) {
        x = u32();
        return *this;
    }

    Deserializer& Deserializer::u64(std::uint64_t& x) {
        x = u64();
        return *this;
    }

    Deserializer& Deserializer::i8(std::int8_t& x) {
        x = i8();
        return *this;
    }

    Deserializer& Deserializer::i16(std::int16_t& x) {
        x = i16();
        return *this;
    }

    Deserializer& Deserializer::i32(std::int32_t& x) {
        x = i32();
        return *this;
    }

    Deserializer& Deserializer::i64(std::int64_t& x) {
        x = i64();
        return *this;
    }

    Deserializer& Deserializer::f32(float& x) {
        x = f32();
        return *this;
    }

    Deserializer& Deserializer::f64(double& x) {
        x = f64();
        return *this;
    }

    Deserializer& Deserializer::str(std::string& x) {
        x = str();
        return *this;
    }

    std::vector<std::string> Deserializer::str_vec() {
        using namespace detail;
        readAssertFlag(m_buffer, m_pos, Flag::ArrayOf);
        readAssertFlag(m_buffer, m_pos, Flag::ByteString);
        readAssertFlag(m_buffer, m_pos, Flag::ArrayLength);
        const auto len = readScalarUnchecked<Flag::Uint64>(m_buffer, m_pos);
        auto vec = std::vector<std::string>{};
        vec.reserve(len);
        for (std::uint64_t i = 0; i < len; ++i) {
            vec.push_back(readByteString(m_buffer, m_pos));
        }
        return vec;
    }

    std::vector<std::unique_ptr<ISerializable>> Deserializer::dynamic_object_vec() {
        using namespace detail;
        readAssertFlag(m_buffer, m_pos, Flag::ArrayOf);
        readAssertFlag(m_buffer, m_pos, Flag::DynamicObject);
        readAssertFlag(m_buffer, m_pos, Flag::ArrayLength);
        auto len = readScalarUnchecked<Flag::Uint64>(m_buffer, m_pos);
        auto vec = std::vector<std::unique_ptr<ISerializable>>{};
        vec.reserve(len);
        for (std::uint64_t i = 0; i < len; ++i) {
            vec.push_back(dynamic_object());
        }
        return vec;
    }

    std::uint64_t Deserializer::readObjectSpanHeader() {
        using namespace detail;
        readAssertFlag(m_buffer, m_pos, Flag::ArrayOf);
        readAssertFlag(m_buffer, m_pos, Flag::StaticObject);
        readAssertFlag(m_buffer, m_pos, Flag::ArrayLength);
        return readScalarUnchecked<Flag::Uint64>(m_buffer, m_pos);
    }

    Serializer& Serializer::i8_span(const std::int8_t* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Int8>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::i16_span(const std::int16_t* src, std::uint64_t len){
        detail::writeSpan<detail::Flag::Int16>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::i32_span(const std::int32_t* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Int32>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::i64_span(const std::int64_t* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Int64>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::f32_span(const float* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Float32>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::f64_span(const double* src, std::uint64_t len) {
        detail::writeSpan<detail::Flag::Float64>(m_buffer, src, len);
        return *this;
    }

    Serializer& Serializer::str_span(const std::string* src, std::uint64_t len) {
        using namespace detail;
        auto& v = m_buffer;
        writeFlag(v, Flag::ArrayOf);
        writeFlag(v, Flag::ByteString);
        writeFlag(v, Flag::ArrayLength);
        writeScalarUnchecked<Flag::Uint64>(v, len);
        for (std::uint64_t i = 0; i < len; ++i) {
            writeByteString(v, src[i]);
        }
        return *this;
    }

    Serializer& Serializer::u8_vec(const std::vector<std::uint8_t>& v) {
        return u8_span(v.data(), v.size());
    }

    Serializer& Serializer::u16_vec(const std::vector<std::uint16_t>& v) {
        return u16_span(v.data(), v.size());
    }

    Serializer& Serializer::u32_vec(const std::vector<std::uint32_t>& v) {
        return u32_span(v.data(), v.size());
    }

    Serializer& Serializer::u64_vec(const std::vector<std::uint64_t>& v) {
        return u64_span(v.data(), v.size());
    }

    Serializer& Serializer::i8_vec(const std::vector<std::int8_t>& v) {
        return i8_span(v.data(), v.size());
    }

    Serializer& Serializer::i16_vec(const std::vector<std::int16_t>& v) {
        return i16_span(v.data(), v.size());
    }

    Serializer& Serializer::i32_vec(const std::vector<std::int32_t>& v) {
        return i32_span(v.data(), v.size());
    }

    Serializer& Serializer::i64_vec(const std::vector<std::int64_t>& v) {
        return i64_span(v.data(), v.size());
    }

    Serializer& Serializer::f32_vec(const std::vector<float>& v) {
        return f32_span(v.data(), v.size());
    }

    Serializer& Serializer::f64_vec(const std::vector<double>& v) {
        return f64_span(v.data(), v.size());
    }

    Serializer& Serializer::str_vec(const std::vector<std::string>& v) {
        return str_span(v.data(), v.size());
    }

    Serializer& Serializer::dynamic_object_vec(const std::vector<std::unique_ptr<ISerializable>>& objs) {
        using namespace detail;
        auto& v = m_buffer;
        auto len = static_cast<std::uint64_t>(v.size());
        writeFlag(v, Flag::ArrayOf);
        writeFlag(v, Flag::DynamicObject);
        writeFlag(v, Flag::ArrayLength);
        writeScalarUnchecked<Flag::Uint64>(v, len);
        for (std::uint64_t i = 0; i < len; ++i) {
            dynamic_object(objs[i].get());
        }
        return *this;
    }

    void Serializer::writeObjectSpanHeader(std::uint64_t len) {
        using namespace detail;
        detail::writeFlag(m_buffer, Flag::ArrayOf);
        detail::writeFlag(m_buffer, Flag::StaticObject);
        detail::writeFlag(m_buffer, Flag::ArrayLength);
        detail::writeScalarUnchecked<Flag::Uint64>(m_buffer, len);
    }

    Deserializer& Deserializer::dynamic_object(std::unique_ptr<ISerializable>& p) {
        p = dynamic_object();
        return *this;
    }

    bool Deserializer::b(){
        return detail::readScalar<detail::Flag::Bool>(m_buffer, m_pos);
    }

    std::uint8_t Deserializer::u8(){
        return detail::readScalar<detail::Flag::Uint8>(m_buffer, m_pos);
    }

    std::uint16_t Deserializer::u16(){
        return detail::readScalar<detail::Flag::Uint16>(m_buffer, m_pos);
    }

    std::uint32_t Deserializer::u32(){
        return detail::readScalar<detail::Flag::Uint32>(m_buffer, m_pos);
    }

    std::uint64_t Deserializer::u64(){
        return detail::readScalar<detail::Flag::Uint64>(m_buffer, m_pos);
    }

    std::int8_t Deserializer::i8(){
        return detail::readScalar<detail::Flag::Int8>(m_buffer, m_pos);
    }

    std::int16_t Deserializer::i16(){
        return detail::readScalar<detail::Flag::Int16>(m_buffer, m_pos);
    }

    std::int32_t Deserializer::i32(){
        return detail::readScalar<detail::Flag::Int32>(m_buffer, m_pos);
    }

    std::int64_t Deserializer::i64(){
        return detail::readScalar<detail::Flag::Int64>(m_buffer, m_pos);
    }

    float Deserializer::f32(){
        return detail::readScalar<detail::Flag::Float32>(m_buffer, m_pos);
    }

    double Deserializer::f64(){
        return detail::readScalar<detail::Flag::Float64>(m_buffer, m_pos);
    }

    std::string Deserializer::str(){
        return detail::readByteString(m_buffer, m_pos);
    }

    std::unique_ptr<ISerializable> Deserializer::dynamic_object() {
        using namespace detail;
        readAssertFlag(m_buffer, m_pos, Flag::DynamicObject);
        auto id = readByteString(m_buffer, m_pos);
        if (id == "") {
            return nullptr;
        }
        return Factory::create(id, *this);
    }

    std::uint64_t Deserializer::peekSpanLength(){
        return detail::peekSpanLength(m_buffer, m_pos);
    }

    Deserializer& Deserializer::b_span(bool* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Bool>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::u8_span(std::uint8_t* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Uint8>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::u16_span(std::uint16_t* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Uint16>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::u32_span(std::uint32_t* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Uint32>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::u64_span(std::uint64_t* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Uint64>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::i8_span(std::int8_t* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Int8>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::i16_span(std::int16_t* dst, std::uint64_t len){
        detail::readSpan<detail::Flag::Int16>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::i32_span(std::int32_t* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Int32>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::i64_span(std::int64_t* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Int64>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::f32_span(float* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Float32>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::f64_span(double* dst, std::uint64_t len) {
        detail::readSpan<detail::Flag::Float64>(m_buffer, m_pos, dst, len);
        return *this;
    }

    Deserializer& Deserializer::str_span(std::string* dst, std::uint64_t len) {
        using namespace detail;
        readAssertFlag(m_buffer, m_pos, Flag::ArrayOf);
        readAssertFlag(m_buffer, m_pos, Flag::ByteString);
        if (readScalarUnchecked<Flag::Uint64>(m_buffer, m_pos) != len) {
            throw SerializationException{};
        }
        for (std::uint64_t i = 0; i < len; ++i) {
            dst[i] = readByteString(m_buffer, m_pos);
        }
        return *this;
    }

    std::vector<bool> Deserializer::b_vec() {
        return detail::readVec<detail::Flag::Bool>(m_buffer, m_pos);
    }

    std::vector<std::uint8_t> Deserializer::u8_vec() {
        return detail::readVec<detail::Flag::Uint8>(m_buffer, m_pos);
    }

    std::vector<std::uint16_t> Deserializer::u16_vec() {
        return detail::readVec<detail::Flag::Uint16>(m_buffer, m_pos);
    }

    std::vector<std::uint32_t> Deserializer::u32_vec() {
        return detail::readVec<detail::Flag::Uint32>(m_buffer, m_pos);
    }

    std::vector<std::uint64_t> Deserializer::u64_vec() {
        return detail::readVec<detail::Flag::Uint64>(m_buffer, m_pos);
    }

    std::vector<std::int8_t> Deserializer::i8_vec() {
        return detail::readVec<detail::Flag::Int8>(m_buffer, m_pos);
    }

    std::vector<std::int16_t> Deserializer::i16_vec(){
        return detail::readVec<detail::Flag::Int16>(m_buffer, m_pos);
    }

    std::vector<std::int32_t> Deserializer::i32_vec() {
        return detail::readVec<detail::Flag::Int32>(m_buffer, m_pos);
    }

    std::vector<std::int64_t> Deserializer::i64_vec() {
        return detail::readVec<detail::Flag::Int64>(m_buffer, m_pos);
    }

    std::vector<float> Deserializer::f32_vec() {
        return detail::readVec<detail::Flag::Float32>(m_buffer, m_pos);
    }

    std::vector<double> Deserializer::f64_vec() {
        return detail::readVec<detail::Flag::Float64>(m_buffer, m_pos);
    }

    void Factory::add(std::string identifier, ObjectCreatorFn f) {
        assert(f);
        auto& m = getObjectMap();
        assert(m.find(identifier) == m.end());
        m.emplace(std::move(identifier), f);
    }

    void Factory::remove(const std::string& identifier) {
        auto& m = getObjectMap();
        auto it = m.find(identifier);
        assert(it != m.end());
        m.erase(it);
    }

    std::unique_ptr<ISerializable> Factory::create(const std::string& identifier, Deserializer& d) {
        auto& m = getObjectMap();
        auto it = m.find(identifier);
        assert(it != m.end());
        auto f = it->second;
        assert(f);
        return f(d);
    }

    Factory::ObjectMap& Factory::getObjectMap() noexcept {
        static auto s_theMap = ObjectMap{};
        return s_theMap;
    }

} // namespace ofc
