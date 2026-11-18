// Реализация шаблона AvlTree. Подключается только из AvlTree.h.

#include <utility>

#include "Exceptions.h"

// --- Создание и уничтожение -------------------------------------------------

template <class T, class Compare>
AvlTree<T, Compare>::AvlTree(Compare compare)
    : root_(nullptr), count_(0), compare_(compare)
{
}

template <class T, class Compare>
AvlTree<T, Compare>::AvlTree(const AvlTree& other)
    : root_(CopySubtree(other.root_)), count_(other.count_), compare_(other.compare_)
{
}

template <class T, class Compare>
AvlTree<T, Compare>::AvlTree(AvlTree&& other) noexcept
    : root_(nullptr), count_(0), compare_(other.compare_)
{
    Swap(other);
}

template <class T, class Compare>
AvlTree<T, Compare>& AvlTree<T, Compare>::operator=(const AvlTree& other)
{
    AvlTree copy(other);
    Swap(copy);
    return *this;
}

template <class T, class Compare>
AvlTree<T, Compare>& AvlTree<T, Compare>::operator=(AvlTree&& other) noexcept
{
    AvlTree taken(std::move(other));
    Swap(taken);
    return *this;
}

template <class T, class Compare>
AvlTree<T, Compare>::~AvlTree()
{
    DeleteSubtree(root_);
}

// --- Свойства ---------------------------------------------------------------

template <class T, class Compare>
std::size_t AvlTree<T, Compare>::GetCount() const
{
    return count_;
}

template <class T, class Compare>
bool AvlTree<T, Compare>::GetIsEmpty() const
{
    return count_ == 0;
}

template <class T, class Compare>
int AvlTree<T, Compare>::GetHeight() const
{
    return HeightOf(root_);
}

// --- Изменение --------------------------------------------------------------

template <class T, class Compare>
bool AvlTree<T, Compare>::Insert(const T& value)
{
    bool inserted = false;
    root_ = InsertInto(root_, value, inserted);
    if (inserted)
    {
        ++count_;
    }
    return inserted;
}

template <class T, class Compare>
void AvlTree<T, Compare>::Remove(const T& value)
{
    root_ = RemoveFrom(root_, value);
    --count_;
}

template <class T, class Compare>
void AvlTree<T, Compare>::Clear()
{
    DeleteSubtree(root_);
    root_ = nullptr;
    count_ = 0;
}

// --- Поиск ------------------------------------------------------------------

template <class T, class Compare>
bool AvlTree<T, Compare>::Contains(const T& value) const
{
    return FindNode(value) != nullptr;
}

template <class T, class Compare>
const T* AvlTree<T, Compare>::Find(const T& value) const
{
    const Node* node = FindNode(value);
    return node == nullptr ? nullptr : &node->value;
}

template <class T, class Compare>
T* AvlTree<T, Compare>::FindForUpdate(const T& value)
{
    Node* node = FindNode(value);
    return node == nullptr ? nullptr : &node->value;
}

template <class T, class Compare>
ArraySequence<T> AvlTree<T, Compare>::GetRange(const T& low, const T& high) const
{
    if (compare_(high, low))
    {
        throw InvalidArgument("AvlTree: верхняя граница диапазона меньше нижней; "
                              "поменяйте границы местами");
    }
    ArraySequence<T> result;
    CollectRange(root_, low, high, result);
    return result;
}

template <class T, class Compare>
ArraySequence<T> AvlTree<T, Compare>::ToArraySequence() const
{
    ArraySequence<T> result;
    result.Reserve(count_);
    CollectAll(root_, result);
    return result;
}

template <class T, class Compare>
bool AvlTree<T, Compare>::CheckInvariants() const
{
    std::size_t count = 0;
    return CheckSubtree(root_, nullptr, nullptr, count) && count == count_;
}

// --- Рекурсивные операции ---------------------------------------------------

// Спуск как в обычном дереве поиска, на обратном пути — балансировка.
// Если new бросит исключение, ни один узел ещё не изменён.
template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::InsertInto(Node* node, const T& value,
                                                                     bool& inserted)
{
    if (node == nullptr)
    {
        inserted = true;
        return new Node{value, nullptr, nullptr, 1};
    }
    if (compare_(value, node->value))
    {
        node->left = InsertInto(node->left, value, inserted);
    }
    else if (compare_(node->value, value))
    {
        node->right = InsertInto(node->right, value, inserted);
    }
    else
    {
        return node; // равный элемент уже есть
    }
    return Rebalance(node);
}

// Три случая: у узла нет детей или один ребёнок — узел заменяется ребёнком;
// два ребёнка — значение узла заменяется минимумом правого поддерева,
// а сам минимум удаляется из правого поддерева.
// Если элемента нет, исключение возникает до любых изменений дерева.
template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::RemoveFrom(Node* node, const T& value)
{
    if (node == nullptr)
    {
        throw KeyNotFound("AvlTree: удаляемого элемента нет в дереве");
    }
    if (compare_(value, node->value))
    {
        node->left = RemoveFrom(node->left, value);
    }
    else if (compare_(node->value, value))
    {
        node->right = RemoveFrom(node->right, value);
    }
    else if (node->left == nullptr || node->right == nullptr)
    {
        Node* child = node->left != nullptr ? node->left : node->right;
        delete node;
        return child;
    }
    else
    {
        node->right = RemoveMinimum(node->right, node->value);
    }
    return Rebalance(node);
}

// Удаляет самый левый узел поддерева, перенося его значение в minimum.
template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::RemoveMinimum(Node* node, T& minimum)
{
    if (node->left == nullptr)
    {
        minimum = std::move(node->value);
        Node* right = node->right;
        delete node;
        return right;
    }
    node->left = RemoveMinimum(node->left, minimum);
    return Rebalance(node);
}

// --- Балансировка -----------------------------------------------------------

template <class T, class Compare>
int AvlTree<T, Compare>::HeightOf(const Node* node)
{
    return node == nullptr ? 0 : node->height;
}

template <class T, class Compare>
void AvlTree<T, Compare>::UpdateHeight(Node* node)
{
    const int left = HeightOf(node->left);
    const int right = HeightOf(node->right);
    node->height = (left > right ? left : right) + 1;
}

// Положительный баланс — перевешивает левое поддерево.
template <class T, class Compare>
int AvlTree<T, Compare>::BalanceOf(const Node* node)
{
    return HeightOf(node->left) - HeightOf(node->right);
}

/*
 * Малый левый поворот:
 *
 *       node              pivot
 *      /    \            /     \
 *     A    pivot  =>   node     C
 *          /   \      /    \
 *         B     C    A      B
 */
template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::RotateLeft(Node* node)
{
    Node* pivot = node->right;
    node->right = pivot->left;
    pivot->left = node;
    UpdateHeight(node);
    UpdateHeight(pivot);
    return pivot;
}

/*
 * Малый правый поворот:
 *
 *         node          pivot
 *        /    \        /     \
 *     pivot    C  =>  A      node
 *     /   \                 /    \
 *    A     B               B      C
 */
template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::RotateRight(Node* node)
{
    Node* pivot = node->left;
    node->left = pivot->right;
    pivot->right = node;
    UpdateHeight(node);
    UpdateHeight(pivot);
    return pivot;
}

// После вставки или удаления баланс узла может стать ±2.
// Перекос «левый-левый» лечится малым правым поворотом, «левый-правый» —
// большим (сначала левый поворот ребёнка, затем правый поворот узла);
// для правой стороны — зеркально.
template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::Rebalance(Node* node)
{
    UpdateHeight(node);
    const int balance = BalanceOf(node);
    if (balance > kMaxBalance)
    {
        if (BalanceOf(node->left) < 0)
        {
            node->left = RotateLeft(node->left);
        }
        return RotateRight(node);
    }
    if (balance < -kMaxBalance)
    {
        if (BalanceOf(node->right) > 0)
        {
            node->right = RotateRight(node->right);
        }
        return RotateLeft(node);
    }
    return node;
}

// --- Обходы -----------------------------------------------------------------

template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::FindNode(const T& value) const
{
    Node* node = root_;
    while (node != nullptr)
    {
        if (compare_(value, node->value))
        {
            node = node->left;
        }
        else if (compare_(node->value, value))
        {
            node = node->right;
        }
        else
        {
            return node;
        }
    }
    return nullptr;
}

// Симметричный обход с отсечением: в левое поддерево заходим, только если
// значение узла больше low (слева могут быть подходящие элементы), в правое —
// только если оно меньше high.
template <class T, class Compare>
void AvlTree<T, Compare>::CollectRange(const Node* node, const T& low, const T& high,
                                       ArraySequence<T>& result) const
{
    if (node == nullptr)
    {
        return;
    }
    if (compare_(low, node->value))
    {
        CollectRange(node->left, low, high, result);
    }
    if (!compare_(node->value, low) && !compare_(high, node->value))
    {
        result.Append(node->value);
    }
    if (compare_(node->value, high))
    {
        CollectRange(node->right, low, high, result);
    }
}

template <class T, class Compare>
void AvlTree<T, Compare>::CollectAll(const Node* node, ArraySequence<T>& result)
{
    if (node == nullptr)
    {
        return;
    }
    CollectAll(node->left, result);
    result.Append(node->value);
    CollectAll(node->right, result);
}

// --- Копирование и освобождение ---------------------------------------------

// Если на каком-то узле не хватит памяти, уже скопированная часть
// освобождается, и исключение уходит дальше: утечки нет.
template <class T, class Compare>
typename AvlTree<T, Compare>::Node* AvlTree<T, Compare>::CopySubtree(const Node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }
    Node* copy = new Node{node->value, nullptr, nullptr, node->height};
    try
    {
        copy->left = CopySubtree(node->left);
        copy->right = CopySubtree(node->right);
    }
    catch (...)
    {
        DeleteSubtree(copy);
        throw;
    }
    return copy;
}

template <class T, class Compare>
void AvlTree<T, Compare>::DeleteSubtree(Node* node)
{
    if (node == nullptr)
    {
        return;
    }
    DeleteSubtree(node->left);
    DeleteSubtree(node->right);
    delete node;
}

// low и high — строгие границы значений в поддереве (nullptr — границы нет).
template <class T, class Compare>
bool AvlTree<T, Compare>::CheckSubtree(const Node* node, const T* low, const T* high,
                                       std::size_t& count) const
{
    if (node == nullptr)
    {
        return true;
    }
    const bool inOrder = (low == nullptr || compare_(*low, node->value))
                         && (high == nullptr || compare_(node->value, *high));
    if (!inOrder)
    {
        return false;
    }
    if (!CheckSubtree(node->left, low, &node->value, count)
        || !CheckSubtree(node->right, &node->value, high, count))
    {
        return false;
    }
    const int left = HeightOf(node->left);
    const int right = HeightOf(node->right);
    const int expectedHeight = (left > right ? left : right) + 1;
    const int balance = left - right;
    ++count;
    return node->height == expectedHeight && balance >= -kMaxBalance && balance <= kMaxBalance;
}

// --- Прочее -----------------------------------------------------------------

template <class T, class Compare>
void AvlTree<T, Compare>::Swap(AvlTree& other) noexcept
{
    std::swap(root_, other.root_);
    std::swap(count_, other.count_);
    std::swap(compare_, other.compare_);
}
