
Type-Erased Container Iteration
*******************************

.. sidebar:: Status

    Published
      23-Jan-2014
    Author
      Stephen Kelly <stephen.kelly@kdab.com>
    Disclosure-Number
      IPCOM000234630D
    Implementations
      `QVariant <https://qt.gitorious.org/qt/qtbase/source/v5.2.0:src/corelib/kernel/qvariant.h>`_/`QMetaType <https://qt.gitorious.org/qt/qtbase/source/v5.2.0:src/corelib/kernel/qmetatype.h>`_
    Links
      * `IP.com <http://ip.com/IPCOM/000234630>`_
      * `defensivepublications.org <http://www.defensivepublications.org/publications/type-erased-container-iteration>`_

Abstract
========

Disclosed is a method of processing the elements of a container in a strongly
typed language without compile-time knowledge of the type of the container or
of the element type within the container.

.. admonition:: Keywords

   Type-erasure, container, iteration, rtti, runtime type information,
   dynamic type, dynamic typing, variable type, any type

Introduction
============

A strong type system provides readability, safety of data interpretation and
can be used for compatibility between different code from different vendors.
The type of all code components is determined at time of compilation of source
code and is immutable.  The consumer of an instance of data must know the type
of the data in order to process it.  Intermediate code may transport
references to data, without communicating the type of the data, by using the
void pointer type.  Such a void pointer may be coupled with an runtime integral
value which corresponds to the strict, static type.  The integral value may be
used at runtime to determine the viability of interpretation of the data as a
particular static type.  This is already common practice.

Containers are a category of type which refer to a collection of instances of
a particular element type.  Containers differ in implementation details such
as whether elements are stored contiguously in memory, or as a linked list
for example.  Containers have associated types which may be used to iterate
over and process the elements in the array.  Such iterator types are strongly
coupled to the element type in the container.  Thus, to iterate over and
process the elements in a container, the type of the container must be
statically (ie, at compile time) known and the type of the elements in the
container must be statically known.

Description
===========

A runtime registration system is required to assign an integral value to
each unique type which may be contained, and each container.  This may use
``rtti`` (run-time type information) supplied by the compiler, or it may use
external scaffolding.

What is disclosed is a method of ``type erasure`` whereby the elements of
a container may be iterated over, without static knowledge of the type of the
container or the type of the elements of the container.

This method is applicable to a language which:
1) Is statically typed
2) Has a way to represent data whose type is not known (eg a void pointer)
3) Has a way to cast/interpret such non-typed data as a particular static
type, where the type is defined at compile-time.
4) Has a common way to represent operations on containers (eg iterators,
algorithms) with "static polymorphism" - that is, the API is the same
regardless of the precise type of the container or the type of the element in
the container.
5) Has no convenient built-in to provide container abstraction features.

That includes C++, but may include other languages.

.. figure:: drawing.svg
   :alt: Diagram of iteraction of components for container processing

   Diagram of Iteraction of Components for Container Processing

In the diagram, the strongly typed container is on the left side 'Container
Type', and it has an associated 'Iterator Type'.

1) A static ``Variant`` type is defined as an abstraction containing a void
pointer to data and a run-time type identifier.  Such a type may be created with
statically typed data only.  When such a type is created with
data which is a container, a registry is populated to relate the run-time type
identifier to a container abstraction implementation.

2) The container abstraction implementation ('Implementation' in the diagram)
stores a type-erased, immutable void-pointer to the container, a type-erased,
mutable void pointer representing an iterator, and various function pointers
for the operations ('Typed Operation' in the diagram) which need to be
performed on an iterator to the container.  Such operations include but are
not limited to increment, decrement, dereference, advance, distance,
comparison and assignment.

3) The static type used to create the ``Variant`` instance is also used to
construct the container abstraction implementation.  The function pointers in
the container abstraction implementation are generated with a factory
template-dependent method.  The API of the pointed-to functions are type-erased.

4) The implementation of the pointed-to functions operate on iterators and are
implemented according to standards and library features for iterators,
including but not limited to increment, decrement dereference, advance,
distance, comparison and assignment.

5) There are generally two different forms of container iterators.  One is a
pointer to contained data, which is suitable for containers which store
elements contiguously in memory.  The other is a standalone type which supports
the standard API required of an iterator type.  The two forms are handled
differently.  In the first case, the type-erased mutable void pointer representing
an iterator stored in the container abstraction implementation may point to the
actual data in the container.  In the second case, the type-erased mutable void
pointer representing an iterator stored in the container abstraction
implementation may point to a heap-assigned copy of the standalone type which
supports the standard API required of an iterator type.  The heap-assigned copy
is deleted when no longer required.

6) An abstraction is defined which encapsulates the operations on the container
iterator objects.  The abstraction must handle at least assignment (which may
include heap construction), deletion (in the case of heap construction only),
advancing the iterator, dereferencing and comparison. The abstraction is
implemented in terms of an iterator as a-pointer-to-contained data and as a
standalone type. In all cases, the result of mutation is stored in the
type-erased, mutable void pointer representing an iterator in the container
abstraction implementation.

7) The container abstraction is defined in terms of the container abstraction
implementation.  The container abstraction ('Type-erased Iterable' in the
diagram) contains standard methods to allow creation of type-erased iterator
implementations pointing to the beginning and end of a container, as well as
iterators pointing to particular seeked elements.  This allows the use of
standard library algorithms and language features as they apply to the
containers.  The Type-erased Iterator is implemented in terms of the container
abstraction implementation.

8) Dereferencing a type-erased iterator yields an instance of a ``Variant``,
which contains a type-erased void pointer to data in the container, and an
integral value representing the type.  The integral value may be examined to
interpret the void pointer as a static type which can be used with other code.

Listing 1
=========

.. literalinclude:: basictest.cpp
   :language: c++

Listing 2
=========

.. literalinclude:: types.h
   :language: c++
