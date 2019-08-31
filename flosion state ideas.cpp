// most basic state type
class State; // { virtual void reset() = 0; State(); virtual ~State(); State(State&&); };

// state that is aware of its parent, its owner, and of its current time
class PrimaryState; // : State { const PrimaryState* parent; StateOwner* owner; uint32_t current_time; };

// state that is always associated with a primary state
class SecondaryState;

// basic interface for managing lifetimes of runtime-determined state types which
// are constructed in raw memory
class StateAllocator; // { virtual void construct(void*); virtual void deconstruct(void*); virtual void move_construct(void*, void*); };

// concrete allocator for managing the lifetime of a specific state type
template<typename StateType>
class ConcreteStateAllocator; // : StateAllocator { ... overridden functions ... };

// container that manages a contiguous chunk of raw storage wherein
// desired state types are constructed and managed.
// The container has N slots, and in each slot there is 1 primary state and
// M secondary states. The primary state type is fixed, but secondary state
// types may be added and removed dynamically.
// Each slot has an index, and each state type is assigned an offset.
// Concretely, primary and secondary states are stored one-after-another in
// descending order of size within a slot. This mimics inheritance with multiple
// base classes, but this container allows types to be added at runtime.
// There are of course much simpler ways to achieve something functionally
// identical. This design is highly optimized for spatial locality, cache
// friendliness, and memory efficiency.
template<typename PrimaryStateType>
class StateBuffer; // { resize(size_t n); addSecondaryState<T>(); removeSecondaryState<T>(); getPrimaryState(index); getSecondaryState(index, offset); };

// contains a mapping from parent states to owned states
// (plus any secondary states). Uses a StateBuffer
template<class PrimaryStateType>
class StateOwner; // { PrimaryStateType& findOwnState(const State* context); void addState(const State*); void resetState(const State*); void removeState(const State*); };

// contains a mapping from parent states and custom key type to
// owned states (plus any secondary states). Uses a StateBuffer
// StateBuffer slots are used in key-major order for improved spatial locality
template<class PrimaryStateType, class KeyType>
class KeyStateOwner; // { PrimaryStateType& findOwnState(const State* context); };

template<class SecondaryStateType>
class StateBorrower; // { const StateOwner* owner; size_t offset; SecondaryStateType* findOwnState(const State* context); };


// Definitions
