#include <util/async_component.h>

class Reporter : public AsyncComponent {
protected:
    void run() override;

public:
    explicit Reporter();
    virtual ~Reporter() = default;
};