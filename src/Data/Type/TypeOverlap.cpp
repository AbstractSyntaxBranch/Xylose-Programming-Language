#include "Type.hpp"
#include <ranges>
#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>

bool Type::overlapsWith(const Type& other) const{
    return overlapsWithOneWay(other) || other.overlapsWithOneWay(*this);
}

//very similar to getTypeCompatibility, but there are some real differences
bool Type::overlapsWithOneWay(const Type& other) const{
    bool same_catagory = other.catagory == this->catagory;
    if(other.catagory == TypeCatagory::GENERIC)
        return overlapsWith(std::get<GenericSubdata>(other.sub_data)->requirement);
    if(other.catagory == TypeCatagory::GENERIC_OR && catagory != TypeCatagory::GENERIC_OR){
        const std::vector<Type>& other_sub_data = std::get<std::vector<Type>>(other.sub_data);
        return std::ranges::all_of(other_sub_data, [&](const Type& ty){return overlapsWith(ty);});
    }
    switch(this->catagory){
        case TypeCatagory::GENERIC_ANY:
            return true;
        case TypeCatagory::GENERIC:
        {
            const auto& sub_data_ = std::get<GenericSubdata>(this->sub_data);
            return sub_data_->requirement.overlapsWith(other);
        }
        case TypeCatagory::POINTER:
        case TypeCatagory::OPTION:
        case TypeCatagory::SET:
        {
            if(!same_catagory)
                return false;
            const auto& sub_data_ = std::get<std::shared_ptr<Type>>(this->sub_data);
            const auto& other_sub_data = std::get<std::shared_ptr<Type>>(other.sub_data);
            return sub_data_->overlapsWith(*other_sub_data);
        }
        case TypeCatagory::ARRAY:
        {
            if(!same_catagory)
                return false;
            const auto& sub_data_ = std::get<Type::ArraySubdata>(sub_data);
            const auto& other_sub_data = std::get<Type::ArraySubdata>(other.sub_data);
            bool equal_size = ignore_non_type_details || sub_data_.size == other_sub_data.size;
            return equal_size && sub_data_.type->overlapsWith(*other_sub_data.type);
        }
        case TypeCatagory::STRUCTURE:
            throw std::runtime_error{"UNIMPLEMENTED.STRUCT.GENERIC"};
        case TypeCatagory::TUPLE:
        {
            if(!same_catagory)
                return false;
            const std::vector<Type>& sub_data_ = std::get<std::vector<Type>>(sub_data);
            const std::vector<Type>& other_sub_data = std::get<std::vector<Type>>(other.sub_data);
            return std::ranges::all_of(
                std::views::zip(sub_data_, other_sub_data),
                [&](const std::pair<Type, Type>& e){return e.first.overlapsWith(e.second);}
            ) && other_sub_data.size() >= sub_data_.size()
              && (ignore_non_type_details || sub_data_.size() == other_sub_data.size());
        }
        case TypeCatagory::MAP:
        case TypeCatagory::RESULT:
        {
            if(!same_catagory)
                return false;
            const auto& sub_data_ = std::get<std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>>(sub_data);
            const auto& other_sub_data = std::get<std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>>(other.sub_data);
            return sub_data_.first->overlapsWith(*other_sub_data.first)
                && sub_data_.second->overlapsWith(*other_sub_data.second);
        }
        case TypeCatagory::ENUM:
            throw std::runtime_error{"UNIMPLEMENTED.ENUM.GENERIC"};
        case TypeCatagory::FUNCTION_PTR:
        {
            if(!same_catagory)
                return false;
            const FnPtrSubdata& sub_data_ = std::get<FnPtrSubdata>(sub_data);
            const FnPtrSubdata& other_sub_data = std::get<FnPtrSubdata>(other.sub_data);
            bool ret_compat = sub_data_.return_type->overlapsWith(*other_sub_data.return_type);
            bool param_compat = std::ranges::all_of(
                std::views::zip(sub_data_.param_types, other_sub_data.param_types),
                [&](const std::pair<Type, Type>& e){return e.first.overlapsWith(e.second);}
            ) && other_sub_data.param_types.size() >= sub_data_.param_types.size()
              && (ignore_non_type_details || sub_data_.param_types.size() == other_sub_data.param_types.size());
            return ret_compat && param_compat;
        }
        case TypeCatagory::GENERIC_OR:
        {
            if(same_catagory){
                 return std::ranges::any_of(other.getGenericOrEntries(),
                     [&](const Type& item){
                         return std::ranges::any_of(this->getGenericOrEntries(), [&](const Type& item_){
                             return item_.overlapsWith(item);
                         });
                     });
            }
            const std::vector<Type>& sub_data_ = std::get<std::vector<Type>>(sub_data);
            return std::ranges::any_of(
                sub_data_,
                [&](const Type& e){return e.overlapsWith(other);}
            );
        }
        case TypeCatagory::DYN_OR:
        {
            if(!same_catagory)
                return false;
            const std::vector<Type>& sub_data_ = std::get<std::vector<Type>>(sub_data);
            const std::vector<Type>& other_sub_data = std::get<std::vector<Type>>(other.sub_data);
            return std::ranges::all_of(sub_data_, [&](const Type& type){return std::ranges::any_of(other_sub_data, [&](const Type& other_type){return type.overlapsWith(other_type);});})
                && std::ranges::all_of(other_sub_data, [&](const Type& other_type){return std::ranges::any_of(sub_data_, [&](const Type& type){return type.overlapsWith(other_type);});});
        }
        case TypeCatagory::UNDERLYING_TYPE_ACCESS:
        {
            if(!same_catagory)
                return false;
            const auto& sub_data_ = std::get<Type::UnderlyingTypeAccessSubdata>(this->sub_data);
            const auto& other_sub_data = std::get<Type::UnderlyingTypeAccessSubdata>(other.sub_data);
            return sub_data_.type->overlapsWith(*other_sub_data.type) && sub_data_.access_index == other_sub_data.access_index;
        }
        default:
            return same_catagory;
    }
}
