#include <util/async_component.h>
#include "daqontrol/events.h"

namespace Daqontrol {
class Observer : public AsyncComponent {
protected:
    void run() override;

public:
    explicit Observer() = default;
    virtual ~Observer() = default;
};
}