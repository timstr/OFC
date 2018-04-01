#pragma once

#include "formscontrol.h"
#include <map>
#include <memory>
#include <typeinfo>
#include <typeindex>

namespace ui {
	namespace forms {

		struct Property {
			Property(std::type_index _typeindex);
			virtual ~Property();

			template<typename T>
			T& get(){
				if (typeindex == std::type_index(typeid(T))){
					return *static_cast<T*>(getValue());
				} else {
					throw std::bad_cast();
				}
			}

			template<typename T>
			void operator=(const T& t){
				get<T>() = t;
			}

			template<typename T>
			operator const T&(){
				return get<T>();
			}

			virtual Control* makeControl(const sf::Font& font) = 0;

			private:
			const std::type_index typeindex;

			virtual void* getValue() = 0;
		};

		template<typename Type>
		struct TypeProperty : Property {
			TypeProperty()
				: Property(std::type_index(typeid(Type))){

			}
			TypeProperty(Type _val)
				: Property(std::type_index(typeid(Type))),
				value(_val) {

			}
			TypeProperty(Type&& _val)
				: Property(std::type_index(typeid(Type))),
				value(std::move(_val)) {

			}

			Type value;

			private:

			void* getValue() override {
				return static_cast<void*>(&value);
			}
		};

		template<typename Type>
		struct PropertyTemplate {
			PropertyTemplate(){
				static_assert(false, "A template specialization is not defined for this type");
			}
		};

		struct Model {

			struct Proxy;

			Proxy& operator[](const std::string& name);

			struct Proxy {

				template<typename PropertyType, typename = std::enable_if<std::is_base_of<Property, PropertyType>::value>::type>
				void operator=(PropertyType p){
					property = std::make_shared<PropertyType>(p);
				}

				template<typename Type, typename = std::enable_if<std::is_base_of<Property, PropertyTemplate<std::remove_reference<Type>::type>>::value>::type>
				void operator=(Type&& t){
					typedef std::remove_reference<Type>::type NonRefType;
					property = std::make_shared<PropertyTemplate<NonRefType>>(std::forward<Type>(t));
				}

				Property* operator->();

				template<typename T>
				operator const T&(){
					return property->get<T>();
				}

				private:
				std::shared_ptr<Property> property;
			};

			private:
			std::map<std::string, Proxy> properties;
			friend struct Form;
		};

	}
}