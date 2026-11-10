#include "Type.hpp"
#include <algorithm>
#include <memory>

bool Type::isGenericDependant() const{
    switch (catagory) {
        case TypeCatagory::TYPEDEF_SWITCH:
        case TypeCatagory::TUPLE:
            return std::ranges::any_of(std::get<TypeDefSwitchSubdata>(sub_data), [](const auto& type){return type.isGenericDependant();});
        case TypeCatagory::GENERIC: //Yes, generics don't need to be generic dependant, since they can be a constant
            return std::get<GenericSubdata>(sub_data)->requirement.isGenericDependant();
        case TypeCatagory::POINTER:
        case TypeCatagory::SET:
        case TypeCatagory::OPTION:
        case TypeCatagory::UNDERLYING_TYPE_ACCESS:
            return std::get<std::shared_ptr<Type>>(sub_data)->isGenericDependant();
        case TypeCatagory::GENERIC_ANY:
        case TypeCatagory::GENERIC_OR:
            return true;
        case TypeCatagory::ARRAY:
            return std::get<ArraySubdata>(sub_data).type->isGenericDependant();
        case TypeCatagory::FUNCTION_PTR:
        {
            const auto& sub_data_ =  std::get<FnPtrSubdata>(sub_data);
            return
                std::ranges::any_of(sub_data_.param_types,
                [](const auto& type){
                    return type.isGenericDependant();
                })
                || sub_data_.return_type->isGenericDependant();
        }
        case TypeCatagory::MAP:
        case TypeCatagory::RESULT:
        {
            const auto& sub_data_ = std::get<MapSubdata>(sub_data);
            return sub_data_.first->isGenericDependant()
                || sub_data_.second->isGenericDependant();
        }
        default: return false;
    }
}
