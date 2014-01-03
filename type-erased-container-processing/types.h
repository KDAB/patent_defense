
#include <typeinfo>
#include <map>
#include <atomic>

#include <assert.h>

namespace TypeErasure
{

struct VariantData
{
    VariantData(const std::size_t metaTypeId_,
                const void *data_)
      : metaTypeId(metaTypeId_)
      , data(data_)
    {
    }
    const std::size_t metaTypeId;
    const void *data;
};

template<typename const_iterator>
struct IteratorAPI
{
    static void assign(void **ptr, const_iterator iterator)
    {
        *ptr = new const_iterator(iterator);
    }
    static void assign(void **ptr, void * const * src)
    {
        *ptr = new const_iterator(*static_cast<const_iterator*>(*src));
    }

    static void advance(void **iterator, int step)
    {
        const_iterator &it = *static_cast<const_iterator*>(*iterator);
        std::advance(it, step);
    }

    static void destroy(void **ptr)
    {
        delete static_cast<const_iterator*>(*ptr);
    }

    static const void *getData(void * const *iterator)
    {
        return &**static_cast<const_iterator*>(*iterator);
    }

    static const void *getData(const_iterator it)
    {
        return &*it;
    }

    static bool equal(void * const *it, void * const *other)
    {
        return *static_cast<const_iterator*>(*it) == *static_cast<const_iterator*>(*other);
    }
};
template<typename value_type>
struct IteratorAPI<const value_type*>
{
    static void assign(void **ptr, const value_type *iterator )
    {
        *ptr = const_cast<value_type*>(iterator);
    }
    static void assign(void **ptr, void * const * src)
    {
        *ptr = static_cast<value_type*>(*src);
    }

    static void advance(void **iterator, int step)
    {
        value_type *it = static_cast<value_type*>(*iterator);
        std::advance(it, step);
        *iterator = it;
    }

    static void destroy(void **)
    {
    }

    static const void *getData(void * const *iterator)
    {
        return *iterator;
    }

    static const void *getData(const value_type *it)
    {
        return it;
    }

    static bool equal(void * const *it, void * const *other)
    {
        return static_cast<value_type*>(*it) == static_cast<value_type*>(*other);
    }
};

enum IteratorCapability
{
    ForwardCapability = 1,
    BiDirectionalCapability = 2,
    RandomAccessCapability = 4
};

template<typename T, typename Category = typename std::iterator_traits<typename T::const_iterator>::iterator_category>
struct CapabilitiesImpl;

template<typename T>
struct CapabilitiesImpl<T, std::forward_iterator_tag>
{ enum { IteratorCapabilities = ForwardCapability }; };
template<typename T>
struct CapabilitiesImpl<T, std::bidirectional_iterator_tag>
{ enum { IteratorCapabilities = BiDirectionalCapability | ForwardCapability }; };
template<typename T>
struct CapabilitiesImpl<T, std::random_access_iterator_tag>
{ enum { IteratorCapabilities = RandomAccessCapability | BiDirectionalCapability | ForwardCapability }; };

template<typename T>
struct ContainerAPI : CapabilitiesImpl<T>
{
    static int size(const T *t) { return std::distance(t->begin(), t->end()); }
};

class SequentialIterableImplementation
{
public:
    const void * _iterable;
    void *_iterator;
    std::size_t _metaType_id;
    unsigned _iteratorCapabilities;
    typedef int(*sizeFunc)(const void *p);
    typedef const void * (*atFunc)(const void *p, int);
    typedef void (*moveIteratorFunc)(const void *p, void **);
    typedef void (*advanceFunc)(void **p, int);
    typedef VariantData (*getFunc)( void * const *p, std::size_t metaTypeId);
    typedef void (*destroyIterFunc)(void **p);
    typedef bool (*equalIterFunc)(void * const *p, void * const *other);
    typedef void (*copyIterFunc)(void **, void * const *);

    sizeFunc _size;
    atFunc _at;
    moveIteratorFunc _moveToBegin;
    moveIteratorFunc _moveToEnd;
    advanceFunc _advance;
    getFunc _get;
    destroyIterFunc _destroyIter;
    equalIterFunc _equalIter;
    copyIterFunc _copyIter;

    template<class T>
    static int sizeImpl(const void *p)
    { return ContainerAPI<T>::size(static_cast<const T*>(p)); }

    template<class T>
    static const void* atImpl(const void *p, int idx)
    {
        typename T::const_iterator i = static_cast<const T*>(p)->begin();
        std::advance(i, idx);
        return IteratorAPI<typename T::const_iterator>::getData(i);
    }

    template<class T>
    static void advanceImpl(void **p, int step)
    { IteratorAPI<typename T::const_iterator>::advance(p, step); }

    template<class T>
    static void moveToBeginImpl(const void *container, void **iterator)
    { IteratorAPI<typename T::const_iterator>::assign(iterator, static_cast<const T*>(container)->begin()); }

    template<class T>
    static void moveToEndImpl(const void *container, void **iterator)
    { IteratorAPI<typename T::const_iterator>::assign(iterator, static_cast<const T*>(container)->end()); }

    template<class T>
    static void destroyIterImpl(void **iterator)
    { IteratorAPI<typename T::const_iterator>::destroy(iterator); }

    template<class T>
    static bool equalIterImpl(void * const *iterator, void * const *other)
    { return IteratorAPI<typename T::const_iterator>::equal(iterator, other); }

    template<class T>
    static VariantData getImpl(void * const *iterator, std::size_t metaTypeId)
    { return VariantData(metaTypeId, IteratorAPI<typename T::const_iterator>::getData(iterator)); }

    template<class T>
    static void copyIterImpl(void **dest, void * const * src)
    { IteratorAPI<typename T::const_iterator>::assign(dest, src); }

public:
    template<class T> SequentialIterableImplementation(const T*p)
      : _iterable(p)
      , _iterator(0)
      , _metaType_id(typeid(typename T::value_type).hash_code())
      , _iteratorCapabilities(ContainerAPI<T>::IteratorCapabilities)
      , _size(sizeImpl<T>)
      , _at(atImpl<T>)
      , _moveToBegin(moveToBeginImpl<T>)
      , _moveToEnd(moveToEndImpl<T>)
      , _advance(advanceImpl<T>)
      , _get(getImpl<T>)
      , _destroyIter(destroyIterImpl<T>)
      , _equalIter(equalIterImpl<T>)
      , _copyIter(copyIterImpl<T>)
    {
    }

    SequentialIterableImplementation()
      : _iterable(0)
      , _iterator(0)
      , _metaType_id(typeid(void).hash_code())
      , _iteratorCapabilities(0)
      , _size(0)
      , _at(0)
      , _moveToBegin(0)
      , _moveToEnd(0)
      , _advance(0)
      , _get(0)
      , _destroyIter(0)
      , _equalIter(0)
      , _copyIter(0)
    {
    }

    inline void moveToBegin() { _moveToBegin(_iterable, &_iterator); }
    inline void moveToEnd() { _moveToEnd(_iterable, &_iterator); }
    inline bool equal(const SequentialIterableImplementation &other) const { return _equalIter(&_iterator, &other._iterator); }
    inline SequentialIterableImplementation &advance(int i) {
      assert(i > 0 || _iteratorCapabilities & BiDirectionalCapability);
      _advance(&_iterator, i);
      return *this;
    }

    inline VariantData getCurrent() const {
      return _get(&_iterator, _metaType_id);
    }

    VariantData at(int idx) const
    { return VariantData(_metaType_id, _at(_iterable, idx)); }

    int size() const { assert(_iterable); return _size(_iterable); }

    inline void destroyIter() { _destroyIter(&_iterator); }

    void copy(const SequentialIterableImplementation &other)
    {
      *this = other;
      _copyIter(&_iterator, &other._iterator);
    }
};

std::map<std::size_t, SequentialIterableImplementation> converterRegistry;

class SequentialIterable
{
    SequentialIterableImplementation m_impl;
public:
    struct const_iterator;

    friend struct const_iterator;

    explicit SequentialIterable(SequentialIterableImplementation impl);

    const_iterator begin() const;
    const_iterator end() const;

    int size() const;

    bool canReverseIterate() const;
};

int SequentialIterable::size() const
{
    return m_impl.size();
}

bool SequentialIterable::canReverseIterate() const
{
    return m_impl._iteratorCapabilities & BiDirectionalCapability;
}

struct Variant
{
  VariantData data;

  template<typename T>
  Variant(const T& t)
    : data(typeid(T).hash_code(), &t)
  {
      converterRegistry.insert(std::make_pair(typeid(T).hash_code(), SequentialIterableImplementation(&t)));
  }
  Variant(const VariantData data_)
    : data(data_)
  {
  }

  std::size_t type() const
  {
      return data.metaTypeId;
  }

  template<typename T>
  T as() const
  {
      return *reinterpret_cast<const T*>(data.data);
  }
};

template<>
SequentialIterable Variant::as<SequentialIterable>() const
{
    assert(converterRegistry.find(data.metaTypeId) != converterRegistry.end());

    auto impl = converterRegistry[data.metaTypeId];

    return SequentialIterable{ impl };
}

struct SequentialIterable::const_iterator
{
private:
    SequentialIterableImplementation m_impl;
    std::atomic<int>* ref;
    friend class SequentialIterable;
    explicit const_iterator(const SequentialIterable &iter, std::atomic<int>* ref_);

    explicit const_iterator(const SequentialIterableImplementation &impl, std::atomic<int>* ref_);

    void begin();
    void end();
public:
    ~const_iterator();

    const_iterator(const const_iterator &other);

    const_iterator& operator=(const const_iterator &other);

    const Variant operator*() const;
    bool operator==(const const_iterator &o) const;
    bool operator!=(const const_iterator &o) const;
    const_iterator &operator++();
    const_iterator operator++(int);
    const_iterator &operator--();
    const_iterator operator--(int);
    const_iterator &operator+=(int j);
    const_iterator &operator-=(int j);
    const_iterator operator+(int j) const;
    const_iterator operator-(int j) const;
};

SequentialIterable::SequentialIterable(SequentialIterableImplementation impl)
  : m_impl(impl)
{
}

SequentialIterable::const_iterator::const_iterator(const SequentialIterable &iter, std::atomic<int>* ref_)
  : m_impl(iter.m_impl), ref(ref_)
{
    ref->fetch_add(1);
}

SequentialIterable::const_iterator::const_iterator(const SequentialIterableImplementation &impl, std::atomic<int>* ref_)
  : m_impl(impl), ref(ref_)
{
    ref->fetch_add(1);

}

void SequentialIterable::const_iterator::begin()
{
    m_impl.moveToBegin();
}

void SequentialIterable::const_iterator::end()
{
    m_impl.moveToEnd();
}

SequentialIterable::const_iterator SequentialIterable::begin() const
{
    const_iterator it(*this, new std::atomic<int>{});
    it.begin();
    return it;
}

SequentialIterable::const_iterator SequentialIterable::end() const
{
    const_iterator it(*this, new std::atomic<int>{});
    it.end();
    return it;
}

SequentialIterable::const_iterator::~const_iterator() {
    if (!ref->fetch_sub(1)) {
        m_impl.destroyIter();
        delete ref;
    }
}

SequentialIterable::const_iterator::const_iterator(const const_iterator &other)
  : m_impl(other.m_impl), ref(other.ref)
{
    ref->fetch_add(1);
}

SequentialIterable::const_iterator&
SequentialIterable::const_iterator::operator=(const const_iterator &other)
{
    if (!m_impl.equal(other.m_impl)) {
        m_impl = other.m_impl;
        ref = other.ref;
    }
    ref->fetch_add(1);
    return *this;
}

const Variant SequentialIterable::const_iterator::operator*() const
{
    const VariantData d = m_impl.getCurrent();
    if (d.metaTypeId == typeid(Variant).hash_code())
        return *reinterpret_cast<const Variant*>(d.data);
    Variant v = { d };
    return v;
}

bool SequentialIterable::const_iterator::operator==(const const_iterator &other) const
{
    return m_impl.equal(other.m_impl);
}

bool SequentialIterable::const_iterator::operator!=(const const_iterator &other) const
{
    return !m_impl.equal(other.m_impl);
}

SequentialIterable::const_iterator &SequentialIterable::const_iterator::operator++()
{
    m_impl.advance(1);
    return *this;
}

SequentialIterable::const_iterator SequentialIterable::const_iterator::operator++(int)
{
    SequentialIterableImplementation impl;
    impl.copy(m_impl);
    m_impl.advance(1);
    return const_iterator(impl, new std::atomic<int>{});
}

SequentialIterable::const_iterator &SequentialIterable::const_iterator::operator--()
{
    m_impl.advance(-1);
    return *this;
}

SequentialIterable::const_iterator SequentialIterable::const_iterator::operator--(int)
{
    SequentialIterableImplementation impl;
    impl.copy(m_impl);
    m_impl.advance(-1);
    return const_iterator(impl, new std::atomic<int>{});
}

SequentialIterable::const_iterator &SequentialIterable::const_iterator::operator+=(int j)
{
    m_impl.advance(j);
    return *this;
}

SequentialIterable::const_iterator &SequentialIterable::const_iterator::operator-=(int j)
{
    m_impl.advance(-j);
    return *this;
}

SequentialIterable::const_iterator SequentialIterable::const_iterator::operator+(int j) const
{
    SequentialIterableImplementation impl;
    impl.copy(m_impl);
    impl.advance(j);
    return const_iterator(impl, new std::atomic<int>{});
}

SequentialIterable::const_iterator SequentialIterable::const_iterator::operator-(int j) const
{
    SequentialIterableImplementation impl;
    impl.copy(m_impl);
    impl.advance(-j);
    return const_iterator(impl, new std::atomic<int>{});
}

}
