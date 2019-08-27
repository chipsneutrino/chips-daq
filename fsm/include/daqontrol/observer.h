#include <util/async_component.h>

namespace Daqontrol {
class Observer : public AsyncComponent {
protected:
    void run() override;

public:
    explicit Observer(const std::string& bus_url);
    virtual ~Observer() = default;

private:
    std::string bus_url_;
};
}