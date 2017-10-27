#if !NDEBUG
static_assert(false, "Running in debug move");
#endif

#include <chrono>
#include <entityplus/entity.h>
#include <entityx/entityx.h>
#include <iostream>

class Timer {
    std::chrono::high_resolution_clock::time_point start;
    const char* str;

public:
    Timer(const char* str) : start(std::chrono::high_resolution_clock::now()), str(str) {}
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << str
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "\n";
    }
};

void entPlusTest(int entityCount, int iterationCount, int tagProb) {
    using namespace entityplus;
    entity_manager<component_list<int>, tag_list<struct Tag>> em;
    em.create_grouping<int, Tag>();
    std::cout << "EntityPlus\n";
    {
        Timer timer("Add entities: ");
        for (int i = 0; i < entityCount; ++i) {
            auto ent = em.create_entity();
            ent.add_component<int>(i);
            if (i % tagProb == 0)
                ent.set_tag<Tag>(true);
        }
    }
    {
        Timer timer("For_each entities: ");
        std::uint64_t sum = 0;
        for (int i = 0; i < iterationCount; ++i) {
            em.for_each<Tag, int>([&](auto ent, auto i) { sum += i; });
        }
        std::cout << sum << "\n";
    }
}

void entXTest(int entityCount, int iterationCount, int tagProb) {
    using namespace entityx;
    struct Tag {};
    entityx::EntityX ex;
    std::cout << "EntityX\n";
    {
        Timer timer("Add entities: ");
        for (int i = 0; i < entityCount; ++i) {
            auto ent = ex.entities.create();
            ent.assign<int>(i);
            if (i % tagProb == 0)
                ent.assign<Tag>();
        }
    }
    {
        Timer timer("For_each entities: ");
        std::uint64_t sum = 0;
        for (int i = 0; i < iterationCount; ++i) {
            ex.entities.each<Tag, int>([&](auto ent, auto&, auto i) { sum += i; });
        }
        std::cout << sum << "\n";
    }
}

void runTest(int entityCount, int iterationCount, int tagProb) {
    std::cout << "Count: " << entityCount << " ItrCount: " << iterationCount
              << " TagProb: " << tagProb << "\n";
    entPlusTest(entityCount, iterationCount, tagProb);
    // std::cout << "\n";
    // entXTest(entityCount, iterationCount, tagProb);
    std::cout << "\n\n";
}

int main() {
    runTest(1'000, 1'000'000, 3);
    runTest(10'000, 1'000'000, 3);
    runTest(30'000, 100'000, 3);
    runTest(100'000, 100'000, 5);
    runTest(10'000, 1'000'000, 1'000);
    runTest(100'000, 1'000'000, 1'000);
}
