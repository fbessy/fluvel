/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLBOOST_MEM_POOL_ALLOC_NODESARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef LIST_I_HPP
#define LIST_I_HPP

//#define SINGLY_LINKED_LIST_WITH_PREALLOC

#ifdef SINGLY_LINKED_LIST_WITH_PREALLOC
#include <vector>
#include <memory>
#include <cstddef>
#else
#include <list>
#endif

namespace ofeli_ip
{

#ifdef SINGLY_LINKED_LIST_WITH_PREALLOC

struct Node {
    int data;
    Node* next = nullptr;
};

// ================= NodePool =================
class NodePool {
public:
    explicit NodePool(size_t pool_size);

    Node* acquire();
    void  release(Node* n);

    static size_t estimatePoolSize(
        int width,
        int height,
        float safety = 1.8f
        );

    size_t size() const { return nodes.size(); }

private:
    std::vector<Node>  nodes;
    std::vector<Node*> free_list;
};

// ================= List_i =================
class List_i {
public:
    // itérateur minimal STL-like
    struct Itera_i {
        Node* current = nullptr;
        Node* prev    = nullptr;

        int& operator*() const { return current->data; }
        Itera_i& operator++();
        bool operator!=(const Itera_i& o) const { return current != o.current; }
    };

    struct const_Itera_i {
        const Node* current = nullptr;
        const Node* prev    = nullptr;

        const int& operator*() const { return current->data; }
        const_Itera_i& operator++();
        bool operator!=(const const_Itera_i& o) const {
            return current != o.current;
        }
    };

    // pool possédé (usage simple / settings)
    explicit List_i(size_t initial_pool_size = 1000);

    // pool partagé (usage algo)
    explicit List_i(std::shared_ptr<NodePool> shared_pool);

    ~List_i();

    // copy constructor
    List_i(const List_i& other);

    // copy assignment
    List_i& operator=(const List_i& other);

    // move constructor
    List_i(List_i&& other) noexcept;

    // move assignments
    List_i& operator=(List_i&& other) noexcept;

    void push_front(int value);
    Itera_i erase(Itera_i it);
    void clear();
    bool empty() const;

    Itera_i begin();
    Itera_i end();

    const_Itera_i cbegin() const;
    const_Itera_i cend()   const;

private:

    void push_back(int value);

    Node* head = nullptr;
    Node* tail = nullptr;
    std::shared_ptr<NodePool> pool;
};

using Itera_i = List_i::Itera_i;
using const_Itera_i = List_i::const_Itera_i;

#else

// use stl list with the default stl allocator

//using List_i = std::list<int>;
using List_i = std::list<int>;
using Itera_i = std::list<int>::iterator;
using const_Itera_i = std::list<int>::const_iterator;

#endif

}

#endif














