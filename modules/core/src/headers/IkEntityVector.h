#ifndef IKNOW_CORE_IKENTITYVECTOR_
#define IKNOW_CORE_IKENTITYVECTOR_
#include <memory>
#include <vector>
#include <deque>
#include "utlExceptionFrom.h"

namespace iknow {
  namespace core {
    //Convenience types we want to keep out of the
    //"large" core namespace.
    namespace EV {
      enum Direction {
	kRight,
	kLeft
      };
      enum Order {
	kFront,
	kBack
      };
    }

    template<typename T, typename Alloc = std::allocator<T> >
    class IkEntityVector {
    public:
      typedef EV::Direction Direction;
      typedef EV::Order Order;
      IkEntityVector() {
      }

      void VectorBegin() {
	left_queue_.clear();
	right_queue_.clear();
	vector_.clear();
      }
      
      void PhaseBegin() {
	left_buf_queue_.clear();
	right_buf_queue_.clear();
      }

      void Insert(const T& t, EV::Direction direction, EV::Order order) {
	Queue& queue = direction == EV::kLeft ? left_buf_queue_ : right_buf_queue_;
	if (order == EV::kBack) {
	  queue.push_back(t);
	}
	else {
	  queue.push_front(t);
	}
      }
      
      void InsertLeftFront(const T& t) {
	left_buf_queue_.push_front(t);
      }

      void InsertLeftBack(const T& t) {
	left_buf_queue_.push_back(t);
      }

      void InsertRightFront(const T& t) {
	right_buf_queue_.push_front(t);
      }

      void InsertRightBack(const T& t) {
	right_buf_queue_.push_back(t);
      }

      void PhaseEnd() {
	left_queue_.insert(left_queue_.end(),
			   left_buf_queue_.begin(),
			   left_buf_queue_.end());
	right_queue_.insert(right_queue_.begin(),
			    right_buf_queue_.begin(),
			    right_buf_queue_.end());
      }

      void VectorEnd() {
	vector_.reserve(left_queue_.size() + right_queue_.size());
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
	vector_.insert(vector_.end(),
		       left_queue_.begin(),
		       left_queue_.end());
	vector_.insert(vector_.end(),
		       right_queue_.begin(),
		       right_queue_.end());
#else
	//Stupid Solaris Rogue Wave workaround
	for (Queue::iterator i = left_queue_.begin();
	    i != left_queue_.end();
	    ++i) {
	  vector_.push_back(*i);
	}
	for (Queue::iterator i = right_queue_.begin();
	    i != right_queue_.end();
	    ++i) {
	  vector_.push_back(*i);
	}
#endif
      }

      typedef std::vector<T, Alloc> Vector;
      const Vector& ReadVector() const {
	return vector_;
      }

    private:
      typedef std::deque<T, Alloc> Queue;
      Queue left_buf_queue_;
      Queue right_buf_queue_;
      Queue left_queue_;
      Queue right_queue_;
      Vector vector_;
    };
    
    template<typename K, typename V>
    class EVSlot {
    public:
      EVSlot(const K& key) : key_(key), value_(V()), filled_(0) {}
      K GetKey() const {
	return key_;
      }
      V GetValue() const {
	return value_;
      }
      void Fill(const V& value) {
	value_ = value;
	filled_ = true;
      }
      bool IsFilled() const {
	return filled_;
      }
    private:
      K key_;
      V value_;
      bool filled_;
    };
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
    template<typename K, typename V, typename Alloc>
#else
    template<typename K, typename V>
#endif
    class IkEntityVectorEvaluator {
    public:
      typedef EV::Direction Direction;
      typedef EV::Order Order;
      ~IkEntityVectorEvaluator() {
	ClearSlots();
      }
      void VectorBegin() {
	entity_vector_.VectorBegin();
	ClearSlots();
      }
      void PhaseBegin() {
	entity_vector_.PhaseBegin();
	phase_slots_.clear();
      }
      void AddSlot(const K& key, Direction direction, Order order) {
	Slot* slot = AllocateSlot(key);
	entity_vector_.Insert(slot, direction, order);
	phase_slots_.push_back(slot);
	all_slots_.push_back(slot);
      }
      void AddValue(const K& key, const V& value, Direction direction, Order order) {
	for (typename Slots::reverse_iterator i = phase_slots_.rbegin();
	     i != phase_slots_.rend();
	     ++i) {
	  if (!((*i)->IsFilled()) && (*i)->GetKey() == key) {
	    (*i)->Fill(value);
	    return;
	  }
	}
	AddSlot(key, direction, order);
	AddValue(key, value, direction, order);
      }
      void PhaseEnd() {
	entity_vector_.PhaseEnd();
      }
      void VectorEnd() {
	entity_vector_.VectorEnd();
      }


      template<typename OutIterT>
      OutIterT ReadValues(OutIterT out) const {
	typedef typename EntityVector::Vector EV;
	const EV& ev = entity_vector_.ReadVector();
	for (typename EV::const_iterator i = ev.begin(); i != ev.end(); ++i) {
	  if ((*i)->IsFilled()) {
	    *out = (*i)->GetValue();
	    //A workaround for Solaris compiler, which thinks
	    //back_inserter is creating a ref-to-temp when we
	    //use *out++
	    ++out;
	  }
	  
	}
	return out;
      }

    private:
      typedef EVSlot<K,V> Slot;
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
      typedef typename Alloc::template rebind<Slot>::other SlotAlloc;
      typedef typename Alloc::template rebind<Slot*>::other SlotPtrAlloc;
#endif
      Slot* AllocateSlot(const K& key) {
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
	return new (SlotAlloc().allocate(1)) Slot(key);
#else
	return new Slot(key);
#endif
      }
      void DeallocateSlot(Slot* slot) {
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
	SlotAlloc().destroy(slot);
	SlotAlloc().deallocate(slot, 1);
#else
	delete slot;
#endif
      }
      void ClearSlots() {
	for (typename Slots::iterator i = all_slots_.begin(); i != all_slots_.end(); ++i) {
	  DeallocateSlot(*i);
	}
	all_slots_.clear();
      }
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
      typedef std::vector<Slot*, SlotPtrAlloc> Slots;
#else
      typedef std::vector<Slot*> Slots;
#endif
      Slots all_slots_;
      Slots phase_slots_;
      //Bad RogueWave behavior on Solaris again
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
      typedef IkEntityVector<Slot*, SlotPtrAlloc> EntityVector;
#else
      typedef IkEntityVector<Slot*> EntityVector;
#endif
      EntityVector entity_vector_;
    };
    
    

    
  }
}

#endif //IKNOW_CORE_IKENTITYVECTOR_
