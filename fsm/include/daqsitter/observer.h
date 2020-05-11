#include <util/async_component.h>
#include <util/logging.h>

namespace Daqsitter {
class Observer : public AsyncComponent, protected Logging {
protected:
    void run() override;

public:
    explicit Observer(const std::string& bus_url);
    virtual ~Observer() = default;

private:
    std::string bus_url_;
};
}