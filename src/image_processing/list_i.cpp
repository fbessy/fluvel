#include "list_i.hpp"

#ifdef SINGLY_LINKED_LIST_WITH_PREALLOC

#include <cmath>

namespace ofeli_ip
{

// ================= NodePool =================
NodePool::NodePool(size_t pool_size) {
    nodes.resize(pool_size);
    free_list.reserve(pool_size);
    for (auto& n : nodes)
        free_list.push_back(&n);
}

Node* NodePool::acquire() {
    if (free_list.empty())
        return new Node(); // rare, hors pool

    Node* n = free_list.back();
    free_list.pop_back();
    return n;
}

void NodePool::release(Node* n) {
    if (n >= nodes.data() && n < nodes.data() + nodes.size())
        free_list.push_back(n);
    else
        delete n;
}

size_t NodePool::estimatePoolSize(int w, int h, float safety) {
    size_t perimeter = 2 * (w + h);
    return static_cast<size_t>(perimeter * safety) * 2; // deux listes
}

// ================= List_i =================
List_i::List_i(size_t initial_pool_size)
    : pool(std::make_shared<NodePool>(initial_pool_size)) {}

List_i::List_i(std::shared_ptr<NodePool> shared_pool)
    : pool(std::move(shared_pool)) {}

List_i::~List_i() {
    clear();
}

// ================= Copy constructor =================
List_i::List_i(const List_i& other)
    : pool(std::make_shared<NodePool>(other.pool->size())),
    head(nullptr), tail(nullptr)
{
    for (Node* n = other.head; n != nullptr; n = n->next)
        push_back(n->data);  // plus besoin de gérer tail à la main
}

// ================= Copy assignment =================
List_i& List_i::operator=(const List_i& other)
{
    if (this == &other) return *this;

    clear();
    pool = std::make_shared<NodePool>(other.pool->size());
    tail = nullptr;
    head = nullptr;

    for (Node* n = other.head; n != nullptr; n = n->next)
        push_back(n->data);

    return *this;
}

// ================= Move constructor =================
List_i::List_i(List_i&& other) noexcept
    : head(other.head), tail(other.tail),
    pool(std::move(other.pool))
{
    other.head = nullptr;
    other.tail = nullptr;
}

// ================= Move assignment =================
List_i& List_i::operator=(List_i&& other) noexcept
{
    if (this == &other) return *this;

    clear();

    head = other.head;
    tail = other.tail;
    pool = std::move(other.pool);

    other.head = nullptr;
    other.tail = nullptr;

    return *this;
}

void List_i::push_front(int value) {
    Node* n = pool->acquire();
    n->data = value;
    n->next = head;
    head = n;
}

List_i::Itera_i List_i::erase(Itera_i it)
{
    if (!it.current)
        return it;

    Node* next = it.current->next;

    if (it.prev)
        it.prev->next = next;
    else
        head = next;

    pool->release(it.current);

    return Itera_i{ next, it.prev };
}

void List_i::clear() {
    Node* n = head;
    while (n) {
        Node* next = n->next;
        pool->release(n);
        n = next;
    }
    head = nullptr;
}

void List_i::push_back(int value)
{
    Node* n = pool->acquire();
    n->data = value;
    n->next = nullptr;

    if (!head)
    {
        head = tail = n;
    }
    else
    {
        tail->next = n;
        tail = n;
    }
}

bool List_i::empty() const {
    return head == nullptr;
}

List_i::Itera_i List_i::begin() {
    return Itera_i{ head, nullptr };
}

List_i::Itera_i List_i::end() {
    return Itera_i{ nullptr, nullptr };
}

List_i::const_Itera_i List_i::cbegin() const {
    return const_Itera_i{ head, nullptr };
}

List_i::const_Itera_i List_i::cend() const {
    return const_Itera_i{ nullptr, nullptr };
}

List_i::Itera_i& List_i::Itera_i::operator++() {
    prev = current;
    current = current->next;
    return *this;
}

List_i::const_Itera_i& List_i::const_Itera_i::operator++() {
    prev = current;
    current = current->next;
    return *this;
}

}

#endif
