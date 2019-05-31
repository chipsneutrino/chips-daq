#include <util/async_component.h>

namespace Daqonite {
class Observer : public AsyncComponent {
protected:
    void run() override;

public:
    explicit Observer() = default;
    virtual ~Observer() = default;
};
}