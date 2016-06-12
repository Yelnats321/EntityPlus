# EntityPlus
EntityPlus is an Entity Component System written in C++11, offering fast compilation and runtime speeds (to be benchmarked). The only external dependency is `boost` (specifically `boost::variant` and `boost:pool`), and the library is header only, saving you the trouble of fidgeting with build systems.

An Entity Component System is an attempt to decouple data from mechanics. In doing so, it lets you create objects out of building blocks that mesh together to create a whole. It models a has-a relationship, letting you expand without worrying about dependency trees and inheritence. The three main aspects of an ECS are of course Entities, Components, and Systems.

###Components
Components contain information. This can be anything, such as health, a peice of armor, or a status effect. An example component could be the identity of a person, which could be modeled like this:
```c++
struct Identity {
    std::string name_;
    int age_;
};
```
Components don't have to be aggregate types, they can be as complicated as they need to be. For example, if we wanted a health component that would only let you heal to a maximum health, we could do it like this:
```c++
class Health {
    int health_, maxHealth_;
public:
    Health(int health, int maxHealth) :
    health_(health), maxHealth_(maxHealth){}
    
    int addHealth(int health) {
        return health_ = std::max(health+health_, maxHealth);
    }
}
```
As you may have noticed, these are just free `class`es. Usually, to use them with other ECSs you'd have to make them inherit from some common base, probably along with CRTP. However, EntityPlus takes advantage of the type system to eliminate these needs. To use these `class`es we have to do absolutley nothing to them!

###Entities
Entities model something. You can think of them as containers for components. If you want to model a player character, you might want a name, a measurement of their health, and an inventory. To use an entity, we must first create it. However, you can't just create an `Entity`, it needs context on where it exists. We use an `EntityManager` to manage all our `Entity`s for us.
```c++
using MyComponentList = ComponentList<Identity, Health>;
using MyTagList = TagList<>;
EntityManager<MyComponentList, MyTagList> entityManager;
```
Don't be scared by the syntax. Since we don't rely on inheritence or CRTP, we must give the `EntityManager` the a list of `Component`s we will use with it, as well as a list of `Tags`. Don't worry about `Tag`s, we'll cover those later. To create a list of `Component`s, we simply use a `ComponentList`. `ComponentList`s and `TagList`s have to be unique, and the `ComponentList` and `TagList` can't have overlapping types. You'll be told about this via compiler error.
```c++
error C2338: ComponentList must be unique
```
Not so bad right? Anyway, back to creating an entity.
```c++
auto entity = entityManager.createEntity();
```
You probably want to add those components to the entity.
```c++
auto retId = entity.addComponent<Identity>("John", 25");
retId.first.name_ = "Smith";
entity.addComponent<Health>(100, 100);
```
It's quite similair to using `vector::emplace`, because the function forwards it's args to the constructor. What gets returned is a `pair<Component&, bool>`, which contains the `Component` you just added, and a `bool` indicating if you overwrote the previos `Component`. Since the `EntityManager` manages our `Entity`s, we don't hold on to the return of `createEntity()`. In fact, it would be wrong to do so, due to the internal workings of the library. All `Entity`s become invalidated after a call to `step()`, which I will explain the rationale for later.

So now you know how to add components to entities, but what good is that if we can't even hold onto them? Well, that brings us to the final peice of the puzzle.

###Systems
WIP

###Tags

###`step()` rationale
Consider a `System` worthing through an `EntityList`. The list looks something like this:
```c++
EntityList = {A, B, C};
```
Where `A` `B` and `C` are entities. Imagine that this system is used to simulate combat between two entities. Let's say `A` and `C` are in the middle of a fight. Since `A` is earlier than `C` in the list, it will get simulated first. What if `A` attacks and kills `C`? We have two options:

1. Remove `C` and don't simulate it
2. Let `C` remain until some later time

It would be unfair to go with option 1. What if `C` also attacks and kills `A`? Then `A` would be spared its death simply because of the ordering of `EntityList`. This is a problem of determenism, as the order of the list is unspecfied. So option 2 remains.

A similair question is raised with removing components. Since a system operates on a list of entities that all have the components it requires, removing components would cause the system to malfunction. Therefore, we delay removing components until some later time aswell.

How about adding entities and adding components? Let's say we have an `AI` system that controls our NPCs. Two NPCs are fighting, `A`, and `B`. The `AI` system works with `A` first. It sees that `A`s health is dipping low, so it activates a Potion of Invulnerability. `A`s turn now over, it works with `B`. `B` chooses to strike `A`. Without the invulnerability's effect on `A`, `A` would surely die. However, this is not the case as `A` activated the potion! But what if the order of resolution was reversed. What if `B` attacked `A` before `A` had time to activate the potion? The resolution of both scenarios should be the same: `A` should die. To accomplish this, the component that models `Invlunerability` is only added to the local instance of `A`. This change is only propogated troughout the `EntityManager` at a later time.

We can either limit the interactions of systems, or we can limit when changes to entities occur. EntityPlus opts to do the latter. Any changes to `EntityManager` are gated by a call to `EntityManager::step()`. Between two `step()` calls, all calls to `EntityManager::getEntities()` will behave the same. That means that adding a component to an entity, or even creating an entity, will not show up until you call `step()`. However, you are still able to manipulate the new component (and entity).

For example:
```c++
auto ent = entityManager.addEntity();
auto ents = entityManager.getEntities<>(); //ents does not contain ent
auto compPair = ent.addComponent<Component1>();
auto ents2 = entityManager.getEntities<Component1>(); //ents2 does not contain ent
auto comp = compPair.first; //ok, the component already exists
comp.foo = "bar"; //ok too
assert(ent.hasComponent<Component1>() == false);
entityManager.step();
auto ents = entityManager.getEntities<>();
auto ents2 = entityManager.getEntities<Component1>(); //ents and ents2 now both contain ent
```

This way, all systems will work on the same set of data, regardless of entity addition, deletion, or component addition/deletion. Of course, components are still persistent and affected in real time, so modifying them is not gated by `step()`.

You may notice that even though we added a component to the entity, the change isn't even propogated locally. The reasoning is that we should gurantee the invariance of data between two `step()` calls regardless of where the entity came from. That is to say, an `Entity` we created and modified vs an `Entity` from `getEntities()` should behave exactly the same between two `step()` calls. 
