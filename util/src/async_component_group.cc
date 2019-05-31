#include "async_component_group.h"

void AsyncComponentGroup::add(std::shared_ptr<AsyncComponent>&& component)
{
    components_.emplace_back(std::move(component));
}

void AsyncComponentGroup::runAsync()
{
    for (const component_ptr& component : components_) {
        component->runAsync();
    }
}

void AsyncComponentGroup::notifyJoin()
{
    for (const component_ptr& component : components_) {
        component->notifyJoin();
    }
}

void AsyncComponentGroup::join()
{
    for (const component_ptr& component : components_) {
        component->join();
    }
}
