/*
  This file is part of an example implementation of type-erased container
  iteration

  Copyright (C) 2013,2014 Klarälvdalens Datakonsult AB, a KDAB Group company, 
  info@kdab.com
  Author: Stephen Kelly <stephen.kelly@kdab.com>

  All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, 
  this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, 
  this list of conditions and the following disclaimer in the documentation 
  and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors 
  may be used to endorse or promote products derived from this software without 
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  LIABLE FOR   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  CONSEQUENTIAL DAMAGES   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "types.h"

#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>

void print(const TypeErasure::Variant &var)
{
    std::cout << "Item: ";
    if (var.type() == typeid(int).hash_code())
        std::cout << var.as<int>();
    else if (var.type() == typeid(std::string).hash_code())
        std::cout << var.as<std::string>();
    else if (var.type() == typeid(double).hash_code())
        std::cout << var.as<double>();
      // Delibrately omitted to show that the limitation in the testcase is of
      // printing elements only.  The type-erased container abstraction can
      // iterate over the elements, can determine the size of the container etc.
//     else if (var.type() == typeid(bool).hash_code())
//         std::cout << std::boolalpha << var.as<bool>();
    else
        std::cout << "<Unknown>";
    std::cout << std::endl;
}

int main(int argc, char **argv)
{
    {
    std::vector<int> vec;
    vec.push_back(4);
    vec.push_back(7);
    vec.push_back(4);
    vec.push_back(1);

    TypeErasure::Variant var(vec);

    TypeErasure::SequentialIterable iter = var.as<TypeErasure::SequentialIterable>();

    // Demonstrate stl-style iteration.
    auto it = std::begin(iter);
    const auto endIt = std::end(iter);

    for( ; it != endIt; ++it)
        {
        print(*it);
        }
    }

    {
    std::vector<std::string> vec2;
    vec2.push_back("fee");
    vec2.push_back("fih");
    vec2.push_back("foh");
    vec2.push_back("fum");

    TypeErasure::Variant var(vec2);

    TypeErasure::SequentialIterable iter = var.as<TypeErasure::SequentialIterable>();

    // Demonstrate c++11-style range-for iteration.
    for (TypeErasure::Variant v : iter)
        {
        print(v);
        }
    }

    {
    std::list<int> li;
    li.push_back(42);
    li.push_back(57);
    li.push_back(47);
    li.push_back(15);

    TypeErasure::Variant var(li);

    TypeErasure::SequentialIterable iter = var.as<TypeErasure::SequentialIterable>();

    std::cout << "List size: " << iter.size();

    // Demonstrate the runtime determination of whether it is possible to
    // iterate backwards over a container.
    std::cout << " (Can " << (iter.canReverseIterate() ? "" : "not ") << "reverse iterate)" << std::endl;

    // Demonstrate c++11-style range-for iteration.
    for (auto v : iter)
        {
        print(v);
        }

    std::cout << "Reverse:" << std::endl;
    const auto beginIt = std::begin(iter);
    auto it = std::end(iter);

    do
        {
        --it;
        print(*it);
        } while (it != beginIt);
    }

    {
    std::deque<bool> de;
    de.push_back(true);
    de.push_back(false);
    de.push_back(true);

    TypeErasure::Variant var(de);

    TypeErasure::SequentialIterable iter = var.as<TypeErasure::SequentialIterable>();

    std::cout << "Deque size: " << iter.size() << std::endl;

    for (auto v : iter)
        {
        print(v);
        }
    }

    {
    std::forward_list<double> fl;
    fl.push_front(3.14);
    fl.push_front(9.8);

    TypeErasure::Variant var(fl);

    TypeErasure::SequentialIterable iter = var.as<TypeErasure::SequentialIterable>();

    std::cout << "Forward list size: " << iter.size();
    std::cout << " (Can " << (iter.canReverseIterate() ? "" : "not ") << "reverse iterate)" << std::endl;

    for (auto v : iter)
        {
        print(v);
        }
    }

    return 0;
}
