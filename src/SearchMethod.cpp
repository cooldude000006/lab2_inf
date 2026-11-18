#include "SearchMethod.h"

std::string GetMethodName(SearchMethod method)
{
    switch (method)
    {
        case SearchMethod::Linear:
            return "линейный поиск";
        case SearchMethod::BinarySearch:
            return "бинарный поиск";
        case SearchMethod::Avl:
            return "АВЛ-дерево";
    }
    return "?";
}

ArraySequence<SearchMethod> GetAllSearchMethods()
{
    return ArraySequence<SearchMethod>{SearchMethod::Linear, SearchMethod::BinarySearch,
                                       SearchMethod::Avl};
}
