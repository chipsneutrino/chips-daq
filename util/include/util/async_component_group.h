#pragma once

#include <list>
#include <memory>

#include <util/async_component.h>

class AsyncComponentGroup : public AsyncRunnable {
public:
    using component_ptr = std::shared_ptr<AsyncComponent>;

private:
    std::list<component_ptr> components_;

public:
    explicit AsyncComponentGroup() = default;
    virtual ~AsyncComponentGroup() = default;

    void add(std::shared_ptr<AsyncComponent>&& component);

    virtual void runAsync() override;
    virtual void notifyJoin() override;
    virtual void join() override;
};
