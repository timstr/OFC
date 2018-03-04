#include "formsmodel.h"


namespace ui {
	namespace forms {

		Property::Property(std::type_index _typeindex) : typeindex(_typeindex) {

		}
		Property::~Property(){

		}



		Model::Proxy& Model::operator[](const std::string& name){
			return properties[name];
		}

		Property* Model::Proxy::operator->(){
			return property.get();
		}

	}
}