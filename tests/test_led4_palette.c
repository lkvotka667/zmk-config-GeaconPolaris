#include <assert.h>
#include "../src/led4_palette.h"
int main(void) {
    assert(led4_phase(0) == 0);
    assert(led4_phase(1999) == 0);
    assert(led4_phase(2000) == 1);
    assert(led4_phase(3999) == 1);
    assert(led4_phase(4000) == 2);
    assert(led4_pixel(1000, 0, true, 100, 0).h != led4_pixel(1000, 1, true, 100, 0).h);
    for (unsigned i = 0; i < 4; i++) {
        assert(led4_pixel(2000 + i * 250 + 125, i, true, 0, 0).l > 3);
        assert(led4_pixel(2000 + i * 250 + 125, i, true, 0, 0).h == 200);
        assert(led4_pixel(2000 + i * 250 + 125, i, false, 0, 0).h == 0);
    }
    assert(led4_battery(0, 0).h == 0);
    assert(led4_battery(29, 0).h == 0);
    assert(led4_battery(30, 0).h == 120);
    assert(led4_battery(79, 0).h == 120);
    assert(led4_battery(80, 0).h == 200);
    assert(led4_battery(100, 0).h == 200);
    assert(led4_battery(-1, 0).s == 0);
    assert(led4_battery(-1, 1000).l > led4_battery(-1, 0).l);
    for (unsigned i = 0; i < 4; i++)
        assert(led4_pixel(4000, i, true, 100, 0).h == (i < 2 ? 200 : 0));
    return 0;
}
