# Native Extension Demo

This repo implements a simple native extension in C. The exposed types are
Hail virtual types, meant to show how to set up a hierarchy of Python types
implemented within a native extension. There are a few examples of simple
behavior like formatting, item getters, and implementing the sequence protocol.

In many ways, this is an argument to not use the CPython API directly as it is
boilerplate heavy and error prone. Something like [Cython] or [PyO3] should
probably be used instead of the CPython API as they support alternative
implementations (like PyPy) and remove boilerplate, creating ease of maintenance.

[Cython]: https://cython.org/
[PyO3]: https://pyo3.rs/

## Navigation

All the interesting code with respect to the CPython api is in [libhail/module.c].
Read that file.

[libhail/module.c]: libhail/module.c

## Building
Using setuptools:
```
python setup.py build_ext -i
```

## Running
```
python
``>>> import hail as hl
>>> hl.tint32  # singletons
dtype('int32')
>>> hl.tarray(hl.tfloat64)  # constructors
dtype('array<float64>')
>>> hl.ttuple(hl.tstr, hl.tarray(hl.tint32))  # varargs constructors
dtype('tuple(str, array<int32>)')
>>> array_i32 = hl.tarray(element_type=hl.tint32)  # kwarg constructor
>>> array_i32.element_type  # getter support
dtype('int32')
>>> tuple_type = hl.ttuple(hl.tbool, hl.tarray(hl.tint32), hl.tfloat64)
>>> tuple_type
dtype('tuple(bool, array<int32>, float64)')
>>> tuple_type[1]  # __getitem__ support
dtype('array<int32>')
>>> list(tuple_type)  # __iter__ support because of __len__ and __getitem__
[dtype('bool'), dtype('array<int32>'), dtype('float64')]
```
