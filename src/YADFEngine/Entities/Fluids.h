//
// Created by s152717 on 28-12-2019.
//

#ifndef YADF_FLUIDS_H
#define YADF_FLUIDS_H

#include <global.h>
#include <stdint-gcc.h>
#include "WorldAPI.h"

#define AMOUNT_MAX 255
typedef uint8_t FluidAmount;

ENUM(FluidType,
     WATER,
     FIRE,
);

// this is a property list, which contains an entry for each FluidType.
// Usage: float water_weight = FluidProperties[WATER].weight;
// TODO replace with JSON
static const struct FluidProperties {
  float weight;
  float burn_energy;
} FluidProperties[ENUM_SIZE(FluidType)] = {
        [WATER] = {
                9.81f,
                0
        },
        [FIRE] = {
                -1.0f,
                0
        }
};

typedef struct _FluidFlow {
  FluidAmount amount[ENUM_SIZE(FluidType)];
  FluidAmount amount_prev[ENUM_SIZE(FluidType)];
  FluidAmount amount_total;
  FluidAmount amount_total_prev;
  FluidAmount flow_out_total;
  FluidAmount flow_out_xp;
  FluidAmount flow_out_yp;
  FluidAmount flow_out_zp;
  FluidAmount flow_out_xn;
  FluidAmount flow_out_yn;
  FluidAmount flow_out_zn;
} FluidFlow;

/**
 * calculates the flow out of this tile, without changing flows or amounts of other tiles.
 * Is thread-safe with other calls to this function.
 * Updates _prev fields.
 * @param this tile->fluids
 * @param position position of tile
 * @param tile the tile with the fluid
 * @param tile_chunk the chunk containing tile
 */
void fluid_flow_update(FluidFlow* this, Vector3i position, WorldTile* tile, WorldChunk* tile_chunk);

/**
 * calculates the new amount of fluids on this tile for each type. One must call fluid_flow_update on all nearby tiles before this function.
 * Is thread-safe with other calls to this function.
 * @param tile
 * @param position
 * @param tile_chunk
 */
void fluid_amount_update(WorldTile* tile, Vector3i position, WorldChunk* tile_chunk);

#endif //YADF_FLUIDS_H
