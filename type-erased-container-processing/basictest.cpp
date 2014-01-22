/*
  This file is part of an example implementation of type-erased container
  iteration

  Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Stephen Kelly <stephen.kelly@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    std::cout << " (Can " << (iter.canReverseIterate() ? "" : "not ") << "reverse iterate)" << std::endl;

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
