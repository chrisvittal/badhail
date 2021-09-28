# Native Extension Demo

This repo implements a simple native extension in C. The exposed types are
Hail virtual types, meant to show how to set up a hierarchy of Python types
implemented within a native extension. There are a few examples of simple
behavior like formatting, item getters, and implementing the sequence protocol.

In many ways, this is an argument to not use the CPython API directly as it is
boilerplate heavy and error prone. Something like [Cython] or [PyO3] should
probably be used instead of the CPython API as they support alternative
implementations (PyPy), creating ease of maintenance.
