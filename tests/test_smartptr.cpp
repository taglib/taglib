#include <cppunit/extensions/HelperMacros.h>
#include "taglib.h"
#include "tsmartptr.h"
#include "utils.h"

using namespace std;
using namespace TagLib;

bool baseDestructorCalled;
bool derivedDestructorCalled;
bool incompleteDestructorCalled;

class TestSmartptr : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestSmartptr);
  CPPUNIT_TEST(testSharedptrBasic);
  CPPUNIT_TEST(testDerivedClass);
  CPPUNIT_TEST(testIncompleteClass);
  CPPUNIT_TEST_SUITE_END();

private:
  template<class T>
  void ck( const T* v1, T v2 ) { CPPUNIT_ASSERT( *v1 == v2 ); }

public:
  void testSharedptrBasic()
  {
    int * ip = new int;
    SHARED_PTR<int> cp ( ip );
    CPPUNIT_ASSERT( ip == cp.get() );
    CPPUNIT_ASSERT( cp.use_count() == 1 );

    *cp = 54321;
    CPPUNIT_ASSERT( *cp == 54321 );
    CPPUNIT_ASSERT( *ip == 54321 );
    ck( static_cast<int*>(cp.get()), 54321 );
    ck( static_cast<int*>(ip), *cp );

    SHARED_PTR<int> cp2 ( cp );
    CPPUNIT_ASSERT( ip == cp2.get() );
    CPPUNIT_ASSERT( cp.use_count() == 2 );
    CPPUNIT_ASSERT( cp2.use_count() == 2 );

    CPPUNIT_ASSERT( *cp == 54321 );
    CPPUNIT_ASSERT( *cp2 == 54321 );
    ck( static_cast<int*>(cp2.get()), 54321 );
    ck( static_cast<int*>(ip), *cp2 );

    SHARED_PTR<int> cp3 ( cp );
    CPPUNIT_ASSERT( cp.use_count() == 3 );
    CPPUNIT_ASSERT( cp2.use_count() == 3 );
    CPPUNIT_ASSERT( cp3.use_count() == 3 );
    cp.reset();
    CPPUNIT_ASSERT( cp2.use_count() == 2 );
    CPPUNIT_ASSERT( cp3.use_count() == 2 );
    cp.reset( new int );
    *cp =  98765;
    CPPUNIT_ASSERT( *cp == 98765 );
    *cp3 = 87654;
    CPPUNIT_ASSERT( *cp3 == 87654 );
    CPPUNIT_ASSERT( *cp2 == 87654 );
    cp.swap( cp3 );
    CPPUNIT_ASSERT( *cp == 87654 );
    CPPUNIT_ASSERT( *cp2 == 87654 );
    CPPUNIT_ASSERT( *cp3 == 98765 );
    cp.swap( cp3 );
    CPPUNIT_ASSERT( *cp == 98765 );
    CPPUNIT_ASSERT( *cp2 == 87654 );
    CPPUNIT_ASSERT( *cp3 == 87654 );
    cp2 = cp2;
    CPPUNIT_ASSERT( cp2.use_count() == 2 );
    CPPUNIT_ASSERT( *cp2 == 87654 );
    cp = cp2;
    CPPUNIT_ASSERT( cp2.use_count() == 3 );
    CPPUNIT_ASSERT( *cp2 == 87654 );
    CPPUNIT_ASSERT( cp.use_count() == 3 );
    CPPUNIT_ASSERT( *cp == 87654 );

    SHARED_PTR<int> cp4;
    swap( cp2, cp4 );
    CPPUNIT_ASSERT( cp4.use_count() == 3 );
    CPPUNIT_ASSERT( *cp4 == 87654 );
    CPPUNIT_ASSERT( cp2.get() == 0 );

    std::set< SHARED_PTR<int> > scp;
    scp.insert(cp4);
    CPPUNIT_ASSERT( scp.find(cp4) != scp.end() );
    CPPUNIT_ASSERT( scp.find(cp4) == scp.find( SHARED_PTR<int>(cp4) ) );  
  }
  
private:
  class DummyBase
  {
  public:
    explicit DummyBase(int x) : value(x)
    {
    }
  
    virtual ~DummyBase()
    {
      baseDestructorCalled = true;
    }
  
    int getValue() const
    {
      return value;
    }
  
  private:
    int value;
  };
  
  class DummyDerived : public DummyBase
  {
  public:
    explicit DummyDerived(int x) : DummyBase(x)
    {
    }
  
    virtual ~DummyDerived()
    {
      derivedDestructorCalled = true;
    }
  };
  
public:
  void testDerivedClass()
  {
    baseDestructorCalled = false;
    derivedDestructorCalled = false;
  
    {
      SHARED_PTR<DummyBase> p1(new DummyDerived(100));
      CPPUNIT_ASSERT(p1->getValue() == 100);
    }
  
    CPPUNIT_ASSERT(baseDestructorCalled);
    CPPUNIT_ASSERT(derivedDestructorCalled);
  
    baseDestructorCalled = false;
    derivedDestructorCalled = false;
  
    {
      SHARED_PTR<DummyDerived> p1(new DummyDerived(100));
      SHARED_PTR<DummyBase> p2 = p1;
  
      CPPUNIT_ASSERT(p1->getValue() == 100);
      CPPUNIT_ASSERT(p2->getValue() == 100);
    }
  
    CPPUNIT_ASSERT(baseDestructorCalled);
    CPPUNIT_ASSERT(derivedDestructorCalled);
  
    baseDestructorCalled = false;
    derivedDestructorCalled = false;
  
    {
      SHARED_PTR<DummyDerived> p1;
      SHARED_PTR<DummyBase> p2;
  
      p1.reset(new DummyDerived(100));
      p2 = p1;
  
      CPPUNIT_ASSERT(p1->getValue() == 100);
      CPPUNIT_ASSERT(p2->getValue() == 100);
    }
  
    CPPUNIT_ASSERT(baseDestructorCalled);
    CPPUNIT_ASSERT(derivedDestructorCalled);
  }

private:
  class DummyIncomplete;
  SHARED_PTR<DummyIncomplete> pincomplete;
 
  class DummyIncomplete
  {
  public:
    ~DummyIncomplete()
    {
      incompleteDestructorCalled = true;
    }
  
    int getValue() const { return 100; }
  };

public:
  void testIncompleteClass()
  {
    incompleteDestructorCalled = false;
  
    pincomplete.reset(new DummyIncomplete());
    CPPUNIT_ASSERT(pincomplete->getValue() == 100);
  
    pincomplete.reset();
    CPPUNIT_ASSERT(incompleteDestructorCalled);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSmartptr);
