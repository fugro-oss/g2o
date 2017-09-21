#pragma once

#define __GCHANDLE_TO_VOIDPTR(x) ((GCHandle::operator System::IntPtr(x)).ToPointer())
#define __VOIDPTR_TO_GCHANDLE(x) (GCHandle::operator GCHandle(System::IntPtr(x)))
#define __NULLPTR nullptr

template <class T> struct gcroot_weak
{
  typedef System::Runtime::InteropServices::GCHandle GCHandle;

  gcroot_weak()
  {
    _handle = __GCHANDLE_TO_VOIDPTR(GCHandle::Alloc(__NULLPTR));
  }

  gcroot_weak(T t)
  {
    _handle = __GCHANDLE_TO_VOIDPTR(GCHandle::Alloc(t, System::Runtime::InteropServices::GCHandleType::Weak));
  }

  gcroot_weak(const gcroot_weak& r) 
  {
    // don't copy a handle, copy what it points to (see above)
    _handle = __GCHANDLE_TO_VOIDPTR(GCHandle::Alloc(__VOIDPTR_TO_GCHANDLE(r._handle).Target, System::Runtime::InteropServices::GCHandleType::Weak));
  }

  ~gcroot_weak() 
  {
    GCHandle g = __VOIDPTR_TO_GCHANDLE(_handle);
    g.Free();
    _handle = 0; // should fail if reconstituted
  }

  gcroot_weak& operator=(T t) 
  {
    // no need to check for valid handle; was allocated in ctor
    __VOIDPTR_TO_GCHANDLE(_handle).Target = t;
    return *this;
  }

  operator T () const 
  {
    return static_cast<T>(__VOIDPTR_TO_GCHANDLE(_handle).Target);
  }

  T operator->() const 
  {
    return static_cast<T>(__VOIDPTR_TO_GCHANDLE(_handle).Target);
  }

private:
  void* _handle;
  T* operator& ();
};