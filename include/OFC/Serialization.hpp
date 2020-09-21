#pragma once

#include <exception>
#include <cstddef>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

// TODO: use relevant C++20 features like
// - std::span
// - <bit> header, e.g. for endianness

#include <OFC/Util/TemplateMetaProgramming.hpp>

namespace ofc {

    // Three different modes of serialization/deserialization:
    // * Basic types (numbers, booleans, strings):
    //   use member functions like Serializer::u8(uint8_t) or Deserializer::str()
    //   - these types are simply built into the interface and are not extensible
    // * Non-polymorphic class types:
    //   use member function Serializer::object<MyObjectType>(o) or Deserializer::object<MyObjectType>(
    //   - no restrictions on the implementation of the type
    //   - the following functions must exist and be found by ADL:
    //     - void serialize(Serializer&, const MyObjectType&);
    //     - MyObjectType deserialize(Deserializer&, Tag<MyObjectType>);
    //   - No additional type-checking is done; only the object contents as used by the above functions
    //     are used, and there is no additional information or checks against reading those same contents
    //     as another non-polymorphic object with equivalent contents or as individual basic types
    // * Polymorphic class types:
    //   use member functions Serializer::dynamicObject(ISerializable&) or std::unique_ptr<ISerializable> Deserializer::dynamicObject()
    //   - classes must inherit from Serializable<Derived, Identifier>, which:
    //     - also inherits from ISerializable
    //     - gives them a fixed string identifier
    //     - registers them with a factory for deserialization
    //   - classes must be constructible from Deserializer&
    //   - classes must override void serialize(Serializer&) const
    //   - Additional type information is stored about the runtime type of the object (using Identifier),
    //     and the data can only be deserialized as a polymorphic object, not via non-polymorphic objects
    //     or basic types that would be equivalent to the contents of the derived object.

    class Serializer;
    class Deserializer;
    class ISerializable;

    class SerializationException : public std::exception {
    public:
        const char* what() const noexcept override;
    };



    class Factory {
    public:
        Factory() = delete;

        using ObjectCreatorFn = std::unique_ptr<ISerializable>(*)(Deserializer&);

        static void add(std::string identifier, ObjectCreatorFn);

        static void remove(const std::string& identifier);

        static std::unique_ptr<ISerializable> create(const std::string& identifier, Deserializer&);

    private:
        using ObjectMap = std::map<std::string, ObjectCreatorFn>;

        static ObjectMap& getObjectMap() noexcept;
    };



    template<typename T, const char* Identifier>
    class FactoryRegistration {
    public:
        FactoryRegistration() {
            static_assert(std::is_base_of_v<ISerializable, T>, "T must derive from ui::ISerializable (via ui::Serializable)");
            static_assert(std::is_constructible_v<T, Deserializer&>, "T be constructible from a reference to ui::Deserializer");
            Factory::add<T>(
                Identifier,
                [](Deserializer& d) {
                    return std::make_unique<T>(d);
                }
            );
        }
        ~FactoryRegistration() {
            Factory::remove(Identifier);
        }

        FactoryRegistration(FactoryRegistration&&) = delete;
        FactoryRegistration(const FactoryRegistration&) = delete;

        FactoryRegistration& operator=(FactoryRegistration&&) = delete;
        FactoryRegistration& operator=(const FactoryRegistration&) = delete;
    };



    // Base class of polymorphic serializable objects
    class ISerializable {
    public:
        ISerializable() = default;
        virtual ~ISerializable() = default;

        virtual void serialize(Serializer&) const = 0;

        virtual const char* getIdentifier() const noexcept = 0;

    private:

        friend Serializer;
    };



    template<typename Derived, const char* I>
    class Serializable : public ISerializable {
    public:
        Serializable() {
            // ODR-use the registration, forcing its inclusion by the linker,
            // and thus its initialization and registration of the desired type
            (void)&s_factoryRegistration;
        }

    private:
        const char* getIdentifier() const noexcept override final {
            return I;
        }

        inline static FactoryRegistration<Derived, I> s_factoryRegistration = {};
    };

    class Serializer {
    public:
        Serializer() = default;
        ~Serializer() = default;

        Serializer(Serializer&&) = delete;
        Serializer(const Serializer&) = delete;
        Serializer& operator=(Serializer&&) = delete;
        Serializer& operator=(const Serializer&) = delete;

        const std::vector<std::byte>& dump() const;

    public:

        // Chainable inserters for single values

        Serializer& b(bool);

        Serializer& u8(std::uint8_t);
        Serializer& u16(std::uint16_t);
        Serializer& u32(std::uint32_t);
        Serializer& u64(std::uint64_t);

        Serializer& i8(std::int8_t);
        Serializer& i16(std::int16_t);
        Serializer& i32(std::int32_t);
        Serializer& i64(std::int64_t);

        Serializer& f32(float);
        Serializer& f64(double);

        Serializer& str(std::string_view);

        template<typename T>
        Serializer& object(const T&);

        Serializer& dynamic_object(const ISerializable*);

        // Chainable inserters for spans

        Serializer& u8_span(const std::uint8_t* src, std::uint64_t len);
        Serializer& u16_span(const std::uint16_t* src, std::uint64_t len);
        Serializer& u32_span(const std::uint32_t* src, std::uint64_t len);
        Serializer& u64_span(const std::uint64_t* src, std::uint64_t len);

        Serializer& i8_span(const std::int8_t* src, std::uint64_t len);
        Serializer& i16_span(const std::int16_t* src, std::uint64_t len);
        Serializer& i32_span(const std::int32_t* src, std::uint64_t len);
        Serializer& i64_span(const std::int64_t* src, std::uint64_t len);

        Serializer& f32_span(const float* src, std::uint64_t len);
        Serializer& f64_span(const double* src, std::uint64_t len);

        Serializer& str_span(const std::string* src, std::uint64_t len);

        template<typename T>
        Serializer& object_span(const T* src, std::uint64_t len);
        

        // Chainable inserters for vectors (equivalent to using spans)

        Serializer& u8_vec(const std::vector<std::uint8_t>&);
        Serializer& u16_vec(const std::vector<std::uint16_t>&);
        Serializer& u32_vec(const std::vector<std::uint32_t>&);
        Serializer& u64_vec(const std::vector<std::uint64_t>&);

        Serializer& i8_vec(const std::vector<std::int8_t>&);
        Serializer& i16_vec(const std::vector<std::int16_t>&);
        Serializer& i32_vec(const std::vector<std::int32_t>&);
        Serializer& i64_vec(const std::vector<std::int64_t>&);

        Serializer& f32_vec(const std::vector<float>&);
        Serializer& f64_vec(const std::vector<double>&);

        Serializer& str_vec(const std::vector<std::string>&);

        template<typename T>
        Serializer& object_vec(const std::vector<T>&);

        Serializer& dynamic_object_vec(const std::vector<std::unique_ptr<ISerializable>>&);

    private:
        void writeObjectSpanHeader(std::uint64_t len);

        std::vector<std::byte> m_buffer;
    };


    class Deserializer {
    public:
        // Creates a Deserializer from a vector of binary data.
        // This data needs to have been created by a Serializer.

        Deserializer(std::vector<std::byte> buffer);
        Deserializer() = delete;
        ~Deserializer() = default;

        Deserializer(Deserializer&&) = delete;
        Deserializer(const Deserializer&) = delete;
        Deserializer& operator=(Deserializer&&) = delete;
        Deserializer& operator=(const Deserializer&) = delete;

    public:

        // Chainable extractors (pass by reference)

        Deserializer& b(bool&);

        Deserializer& u8(std::uint8_t&);
        Deserializer& u16(std::uint16_t&);
        Deserializer& u32(std::uint32_t&);
        Deserializer& u64(std::uint64_t&);

        Deserializer& i8(std::int8_t&);
        Deserializer& i16(std::int16_t&);
        Deserializer& i32(std::int32_t&);
        Deserializer& i64(std::int64_t&);

        Deserializer& f32(float&);
        Deserializer& f64(double&);

        Deserializer& str(std::string&);

        template<typename T>
        Deserializer& object(T&);

        Deserializer& dynamic_object(std::unique_ptr<ISerializable>&);

        // Non-chainable extractors (simple return values)

        bool b();

        std::uint8_t u8();
        std::uint16_t u16();
        std::uint32_t u32();
        std::uint64_t u64();

        std::int8_t i8();
        std::int16_t i16();
        std::int32_t i32();
        std::int64_t i64();

        float f32();
        double f64();

        std::string str();

        template<typename T>
        T object();

        std::unique_ptr<ISerializable> dynamic_object();

        // Spans

        std::uint64_t peekSpanLength();
        
        Deserializer& b_span(bool* dst, std::uint64_t len);

        Deserializer& u8_span(std::uint8_t* dst, std::uint64_t len);
        Deserializer& u16_span(std::uint16_t* dst, std::uint64_t len);
        Deserializer& u32_span(std::uint32_t* dst, std::uint64_t len);
        Deserializer& u64_span(std::uint64_t* dst, std::uint64_t len);

        Deserializer& i8_span(std::int8_t* dst, std::uint64_t len);
        Deserializer& i16_span(std::int16_t* dst, std::uint64_t len);
        Deserializer& i32_span(std::int32_t* dst, std::uint64_t len);
        Deserializer& i64_span(std::int64_t* dst, std::uint64_t len);

        Deserializer& f32_span(float* dst, std::uint64_t len);
        Deserializer& f64_span(double* dst, std::uint64_t len);

        Deserializer& str_span(std::string* dst, std::uint64_t len);

        template<typename T>
        Deserializer& object_span(T* dst, std::uint64_t len);

        // Vectors
        std::vector<bool> b_vec();

        std::vector<std::uint8_t> u8_vec();
        std::vector<std::uint16_t> u16_vec();
        std::vector<std::uint32_t> u32_vec();
        std::vector<std::uint64_t> u64_vec();

        std::vector<std::int8_t> i8_vec();
        std::vector<std::int16_t> i16_vec();
        std::vector<std::int32_t> i32_vec();
        std::vector<std::int64_t> i64_vec();

        std::vector<float> f32_vec();
        std::vector<double> f64_vec();

        std::vector<std::string> str_vec();

        template<typename T>
        std::vector<T> object_vec();

        std::vector<std::unique_ptr<ISerializable>> dynamic_object_vec();

    private:
        std::uint64_t readObjectSpanHeader();

        std::vector<std::byte> m_buffer;
        std::vector<std::byte>::const_iterator m_pos;
    };

    template<typename T>
    inline Serializer& Serializer::object(const T& t) {
        static_assert(std::is_class_v<T>, "T must be a class or struct");
        serialize(*this, t);
        return *this;
    }

    template<typename T>
    inline Serializer& Serializer::object_span(const T* src, std::uint64_t len) {
        static_assert(std::is_class_v<T>, "T must be a class or struct");
        writeObjectSpanHeader(len);
        for (std::uint64_t i = 0; i < len; ++i) {
            serialize(*this, src[i]);
        }
        return *this;
    }

    template<typename T>
    inline Serializer& Serializer::object_vec(const std::vector<T>& v) {
        static_assert(std::is_class_v<T>, "T must be a class or struct");
        auto len = static_cast<std::uint64_t>(v.size());
        auto src = v.data();
        return object_span<T>(src, len);
    }

    template<typename T>
    inline Deserializer& Deserializer::object(T& t) {
        static_assert(std::is_class_v<T>, "T must be a class or struct");
        deserialize(*this, t);
        return *this;
    }

    template<typename T>
    inline T Deserializer::object() {
        static_assert(std::is_class_v<T>, "T must be a class or struct");
        static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
        auto t = T{};
        deserialize(*this, t);
        return t;
    }

    template<typename T>
    inline Deserializer& Deserializer::object_span(T* dst, std::uint64_t len) {
        static_assert(std::is_class_v<T>, "T must be a class or struct");
        auto actualLen = readObjectSpanHeader();
        if (len != actualLen) {
            throw SerializationException{};
        }
        for (std::uint64_t i = 0; i < len; ++i) {
            dst[i] = object<T>();
        }
        return *this;
    }

    template<typename T>
    inline std::vector<T> Deserializer::object_vec() {
        static_assert(std::is_class_v<T>, "T must be a class or struct");
        std::vector<T> ret;
        auto len = readObjectSpanHeader();
        ret.reserve(len);
        for (std::uint64_t i = 0; i < len; ++i) {
            ret.push_back(object<T>());
        }
        return ret;
    }

} // namespace ofc
