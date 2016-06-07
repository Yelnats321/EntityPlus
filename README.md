# EntityPlus
A Entity Component System written in C++11, offering fast compilation and runtime speeds (to be benchmarked). No external dependencies required, and the library is header only saving you the trouble of fidgeting with build systems.

##Getting started:
First, you want to create the components and tags you are going to be using with the ECS. Creating them is as easy as can be. Since EntityPlus uses a strong type system, you simple need to define the components and tags. That's it, no inheritence or CTRP required.

```c++
struct Identity {
    std::string name_;
    int age_;
};

class Health {
    int health_;
}

struct PlayerTag {};
struct OgreTag {};
```

Then, you want to create your EntityManager. It takes two template parameters, a `ComponentList` and a `TagList`. These are simple variadic structures that simply take a list of types.

```c++
using MyComponentList = entityplus::ComponentList<Identity, Health>;
using MyTagList = entityplus::TagList<PlayerTag, OgreTag>;
entityplus::EntityManager<MyComponentList, MyTagList> entityManager;
```

Let's create a sample entity.

```c++
auto entity = entityManager.createEntity();
entity.addComponent<Identity>("Player One", 23);
entity.setTag<PlayerTag>(true);
```

###A note about deletion
When iterating through a list of entities, one is faced with a dilemna: what happens if an entity gets destroyed? Let me illustrate an example. Suppose we have two entities, `A` and `B`. Let's say some system is working through it's list of entities, and `A` precedes `B` in evaluation order. What happens if `A` destroys `B`? 

We have two options:
1. Destroy `B` and set a flag so that once the system gets to it, it skips it
2. Let `B` survive until some later point 

Now what if it was the other way around? What if `B` destroys `A`. Clearly option 1 is out, because the order the entities are returned in is unspecified. It doesn't make sense to let `A` act out its actions just because it preceeded `B` in being evaluated. So the only option left is 2. There is a function called `EntityManager::step()` which does precisely that. All destructive actions (TBD if it extends to removing components) are saved in a queue and only get executed once `step()` is called. It is recommended to call it after all your systems had a chance of running once, i.e. at the end of the system loop. However, if you want to call it more often you are free to (such as between system steps).
