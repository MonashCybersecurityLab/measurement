//
// Created by shangqi on 10/3/20.
//

#ifndef MEASUREMENT_STASH_H
#define MEASUREMENT_STASH_H

#include "../../../Common/CommonUtil.h"

class Stash {
private:
    uint32_t current_size;
    uint32_t stash_data_size;
    //Static upper bound on stash size
    uint32_t STASH_SIZE;
};


#endif //MEASUREMENT_STASH_H
