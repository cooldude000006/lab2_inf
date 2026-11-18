#pragma once

// АВЛ-дерево — метод 2 (М-3, М-3.1; DESIGN.md, п. 3.3 «Метод 2», п. 4.3).
//
// Бинарное дерево поиска, в котором высоты левого и правого поддерева
// любого узла различаются не больше чем на 1. После каждой вставки и
// удаления баланс восстанавливается поворотами на пути от изменённого
// места к корню. Высота дерева из n узлов не больше ~1,44·log2(n), поэтому
// время операций не зависит от порядка, в котором приходят данные.
//
// Сложность (n — число элементов, k — размер результата):
//   Insert, Remove, Find, Contains — O(log n);
//   GetRange — O(log n + k); ToArraySequence — O(n).
//
// Одинаковых элементов в дереве нет: Insert равного элемента ничего
// не меняет и возвращает false.
//
// Требования к T: копирование и перемещение. Compare — функциональный
// объект bool(const T&, const T&), задающий строгий слабый порядок;
// элементы a и b равны, если !compare(a, b) && !compare(b, a).

#include <cstddef>

#include "ArraySequence.h"
#include "Compare.h"

template <class T, class Compare = Less<T>>
class AvlTree
{
public:
    // --- Создание и уничтожение (правило пяти) ---------------------------

    explicit AvlTree(Compare compare = Compare());

    // Глубокая копия: копируются все узлы, форма дерева сохраняется.
    AvlTree(const AvlTree& other);

    // Перемещение за O(1): узлы забираются, other остаётся пустым.
    AvlTree(AvlTree&& other) noexcept;

    AvlTree& operator=(const AvlTree& other);
    AvlTree& operator=(AvlTree&& other) noexcept;

    ~AvlTree();

    // --- Свойства ---------------------------------------------------------

    std::size_t GetCount() const;
    bool GetIsEmpty() const;

    // Число узлов на самом длинном пути от корня до листа; 0 у пустого дерева.
    int GetHeight() const;

    // --- Изменение --------------------------------------------------------

    // Добавляет элемент. Возвращает false, если равный элемент уже есть
    // (дерево при этом не меняется).
    bool Insert(const T& value);

    // Удаляет элемент, равный value. Бросает KeyNotFound, если его нет.
    void Remove(const T& value);

    void Clear();

    // --- Поиск ------------------------------------------------------------

    bool Contains(const T& value) const;

    // Указатель на элемент дерева, равный value, или nullptr.
    const T* Find(const T& value) const;

    // То же, но элемент можно изменить. Менять можно только ту часть
    // элемента, которая не влияет на сравнение (например, значение пары
    // при сравнении по ключу), иначе порядок дерева нарушится.
    T* FindForUpdate(const T& value);

    // Элементы из [low, high] (обе границы включаются) по возрастанию.
    // Бросает InvalidArgument, если high < low.
    ArraySequence<T> GetRange(const T& low, const T& high) const;

    // Все элементы по возрастанию (симметричный обход).
    ArraySequence<T> ToArraySequence() const;

    // Проверка инвариантов: порядок дерева поиска, правильные высоты узлов,
    // баланс АВЛ и число элементов. Нужна тестам.
    bool CheckInvariants() const;

private:
    // Узел — простая запись без поведения (Голуб, правило 114).
    struct Node
    {
        T value;
        Node* left;
        Node* right;
        int height;
    };

    // Рекурсивные операции над поддеревом; возвращают его новый корень.
    Node* InsertInto(Node* node, const T& value, bool& inserted);
    Node* RemoveFrom(Node* node, const T& value);
    Node* RemoveMinimum(Node* node, T& minimum);

    // Балансировка.
    static int HeightOf(const Node* node);
    static void UpdateHeight(Node* node);
    static int BalanceOf(const Node* node);
    static Node* RotateLeft(Node* node);
    static Node* RotateRight(Node* node);
    static Node* Rebalance(Node* node);

    // Обходы.
    Node* FindNode(const T& value) const;
    void CollectRange(const Node* node, const T& low, const T& high,
                      ArraySequence<T>& result) const;
    static void CollectAll(const Node* node, ArraySequence<T>& result);

    static Node* CopySubtree(const Node* node);
    static void DeleteSubtree(Node* node);
    bool CheckSubtree(const Node* node, const T* low, const T* high, std::size_t& count) const;

    void Swap(AvlTree& other) noexcept;

    // Наибольшая допустимая разница высот поддеревьев узла.
    static constexpr int kMaxBalance = 1;

    Node* root_;
    std::size_t count_;
    Compare compare_;
};

#include "AvlTree.tpp"
