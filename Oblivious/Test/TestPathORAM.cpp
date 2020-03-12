
#include "../Enclave/ORAM/PathORAM.h"
#include "../Enclave/SpookyHash/SpookyV2.h"

int main() {
    auto *oram_instance = new PathORAM(16, 5, 1);

    int test_bid = SpookyHash::Hash32(&test_bid, sizeof(int), 1);
    uint8_t value = 10;

    oram_instance->write(test_bid, &value);
    uint8_t *res = oram_instance->read(test_bid);

    printf("read result:%d\n", *res);
}

