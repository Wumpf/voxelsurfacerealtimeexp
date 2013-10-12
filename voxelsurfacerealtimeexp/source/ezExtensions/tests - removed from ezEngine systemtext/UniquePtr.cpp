#include <PCH.h>
#include <Foundation/Basics/Types/UniquePtr.h>
#include <Foundation/Memory/CommonAllocators.h>

/// \brief Test class for ezUniquePtr content
class UniquePtrTestClass
{
public:
  UniquePtrTestClass()
  {
    s_LastConstructorIndexCalled = 0;
    ++s_NumInstancesAlive;
  }
  UniquePtrTestClass(ezInt8 arg0)
  {
    s_LastConstructorIndexCalled = 1;
    ++s_NumInstancesAlive;
  }
  UniquePtrTestClass(ezInt8 arg0, ezInt16 arg1)
  {
    s_LastConstructorIndexCalled = 2;
    ++s_NumInstancesAlive;
  }
  UniquePtrTestClass(ezInt8 arg0, ezInt16 arg1, ezInt32 arg2)
  {
    s_LastConstructorIndexCalled = 3;
    ++s_NumInstancesAlive;
  }

  ~UniquePtrTestClass()
  {
    --s_NumInstancesAlive;
  }

  bool DoSomething() const
  {
    return true;
  }

  bool DoSomething()
  {
    ++s_DoSomethingDummy;
    return true;
  }

  static ezInt32 s_LastConstructorIndexCalled;
  static ezInt32 s_NumInstancesAlive;
  static ezInt32 s_DoSomethingDummy;

private:
  char m_DataBlock[32];
};
ezInt32 UniquePtrTestClass::s_LastConstructorIndexCalled = -1;
ezInt32 UniquePtrTestClass::s_NumInstancesAlive = 0;
ezInt32 UniquePtrTestClass::s_DoSomethingDummy = 0;


/// \brief Test allocator wrapper.
struct TestAllocatorWrapper
{
  typedef ezAllocator<ezMemoryPolicies::ezHeapAllocation, ezMemoryPolicies::ezStackTracking, ezMutex> Allocator;

  static Allocator* GetAllocator()
  {
    return m_pAllocator;
  }

  static Allocator* m_pAllocator;
};
TestAllocatorWrapper::Allocator* TestAllocatorWrapper::m_pAllocator = NULL;


// typedefs for ezUniquePtr for testing
typedef ezUniquePtr<UniquePtrTestClass, TestAllocatorWrapper> TestUniquePtr;
typedef ezUniquePtr<UniquePtrTestClass[], TestAllocatorWrapper> TestUniquePtrArray;

// some functions for testing ezUniquePtr as return value
static TestUniquePtr ExternalAllocFunc()
{
  TestUniquePtr ptr = EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 1, 2);
  ptr->DoSomething();
  return ptr;
}

// The test itself
EZ_CREATE_SIMPLE_TEST(Basics, UniquePtr)
{
  TestAllocatorWrapper::m_pAllocator = EZ_DEFAULT_NEW(TestAllocatorWrapper::Allocator)("SmartPointer test allocator");

  EZ_TEST_BLOCK(true, "New, destructor, release")
  {
    // test new and scope destructor
    {
      TestUniquePtr pArgs0 = EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass); // implicit
      EZ_TEST_INT(UniquePtrTestClass::s_LastConstructorIndexCalled, 0);
      TestUniquePtr pArgs1(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 1)); // explicit
      EZ_TEST_INT(UniquePtrTestClass::s_LastConstructorIndexCalled, 1);

      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 2);
      {
        TestUniquePtr pArgs2(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 0, 1));
        EZ_TEST_INT(UniquePtrTestClass::s_LastConstructorIndexCalled, 2);
        EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 3);
        {
          TestUniquePtr pArgs3(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 0, 1, 2));
          EZ_TEST_INT(UniquePtrTestClass::s_LastConstructorIndexCalled, 3);
          EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 4);
        }
        EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 3);
      }
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 2);
      EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetAllocationSize(), sizeof(UniquePtrTestClass) * 2);
      EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetNumLiveAllocations(), 2);

      // test release func
      pArgs0.Release();
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 1);
      EZ_TEST(pArgs0.Get() == NULL);
      EZ_TEST(!pArgs0);
    }

    // is everything clean?
    EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 0);
    EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetAllocationSize(), 0);
    EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetNumLiveAllocations(), 0);
  }

  EZ_TEST_BLOCK(true, "rvalue assignment, swap")
  {
    // swap
    {
      TestUniquePtr pFirst(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass));
      UniquePtrTestClass* pFirstInternal = pFirst.Get();
      TestUniquePtr pSecond(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass));
      UniquePtrTestClass* pSecondInternal = pSecond.Get();
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 2);

      pSecond.Swap(pFirst);
      EZ_TEST(pFirst);
      EZ_TEST(pSecond);
      EZ_TEST(pFirstInternal == pSecond.Get());
      EZ_TEST(pSecondInternal == pFirst.Get());
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 2);
    }

    // create from rvalue other than macro creation
    {
      TestUniquePtr pImplicit = ExternalAllocFunc();  // implicit
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 1);
      TestUniquePtr pExplicit(ExternalAllocFunc());   // explicit
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 2);
    }

    // move ptr around
    {
      TestUniquePtr pOrigin = EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass);
      UniquePtrTestClass* pOriginalPtr = pOrigin.Get();

      // implicit
      TestUniquePtr pImplicit = std::move(pOrigin); 
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 1);
      EZ_TEST(pOriginalPtr == pImplicit.Get());
      EZ_TEST(!pOrigin);

      // explicit
      TestUniquePtr pExplicit(std::move(pImplicit)); 
      EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 1);
      EZ_TEST(pOriginalPtr == pExplicit.Get());
      EZ_TEST(!pImplicit);
    }

    // is everything clean?
    EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 0);
    EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetAllocationSize(), 0);
    EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetNumLiveAllocations(), 0);
  }

  EZ_TEST_BLOCK(true, "pointer access")
  {
    // const get
    {
      UniquePtrTestClass::s_DoSomethingDummy = 0;
      const TestUniquePtr pConst(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 1,2));
      EZ_TEST(pConst.Get()->DoSomething());
      EZ_TEST_INT(UniquePtrTestClass::s_DoSomethingDummy, 0);
    }

    // const ->
    {
      UniquePtrTestClass::s_DoSomethingDummy = 0;
      const TestUniquePtr pConst(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 1,2));
      EZ_TEST(pConst->DoSomething());
      EZ_TEST_INT(UniquePtrTestClass::s_DoSomethingDummy, 0);
    }

    // get
    {
      UniquePtrTestClass::s_DoSomethingDummy = 0;
      TestUniquePtr pPtr(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 1,2));
      EZ_TEST(pPtr.Get()->DoSomething());
      EZ_TEST_INT(UniquePtrTestClass::s_DoSomethingDummy, 1);
    }

    // ->
    {
      UniquePtrTestClass::s_DoSomethingDummy = 0;
      TestUniquePtr pPtr(EZ_NEW_UNIQUE(TestAllocatorWrapper, UniquePtrTestClass, 1,2));
      EZ_TEST(pPtr->DoSomething());
      EZ_TEST_INT(UniquePtrTestClass::s_DoSomethingDummy, 1);
    }

    // is everything clean?
    EZ_TEST_INT(UniquePtrTestClass::s_NumInstancesAlive, 0);
    EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetAllocationSize(), 0);
    EZ_TEST_INT(TestAllocatorWrapper::GetAllocator()->GetTracker().GetNumLiveAllocations(), 0);
  }

  EZ_DEFAULT_DELETE(TestAllocatorWrapper::m_pAllocator);
}