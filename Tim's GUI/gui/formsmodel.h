#pragma once

#include "formscontrol.h"
#include <map>
#include <memory>
#include <typeinfo>
#include <typeindex>

namespace ui {
	namespace forms {

		struct Property {
			Property(std::type_index _typeindex) : typeindex(_typeindex) {

			}
			virtual ~Property(){

			}

			template<typename T>
			T& get(){
				if (typeindex == std::type_index(typeid(T))){
					return *(T*)getValue();
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
			TypeProperty(Type _val = {})
				: Property(std::type_index(typeid(Type))),
				value(_val) {

			}

			Type value;

			private:

			void* getValue() override {
				return static_cast<void*>(&value);
			}
		};

		struct Model {

			struct Proxy;

			Proxy& operator[](const std::string& name){
				return properties[name];
			}

			struct Proxy {
				template<typename PropertyType>
				void operator=(PropertyType&& p){
					static_assert(std::is_base_of<Property, PropertyType>::value, "The provided type must derive from Property");
					property = std::make_shared<PropertyType>(std::forward<PropertyType>(p));
				}

				operator Property*(){
					return property.get();
				}

				Property* operator->(){
					return property.get();
				}

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