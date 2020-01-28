//
// Created by s152717 on 28-12-2019.
//

#include "Fluids.h"
#include "../World/World.h"

void fluid_spawn(World* world, Vector3ic* coord, enum FluidType type, FluidAmount amount) {
  FluidFlow* target = world_get_fluid(world, coord);
  target->amount[type] += amount;
  target->amount_total += amount;

  FluidUpdateData elt = {target, world_get_chunk(world, coord), *coord};
  list_add(world_get_fluids_to_update(world), &elt);
}

/**
 * @return flow of a to b, 0 if b has more fluid than a
 */
PURE static inline FluidAmount fluid_flow(WorldTile* a, WorldChunk* ac, WorldTile* b, WorldChunk* bc) {
  if (b->flags & TILE_FLAG_BLOCKING) return 0;

  FluidFlow* flow1 = tile_get_fluid(a, ac);
  FluidFlow* flow2 = tile_get_fluid(b, bc);
  int flow = flow1->amount_total - flow2->amount_total;
  return (FluidAmount) min_i(FLUID_AMOUNT_MAX, max_i(0, flow));
}

void fluid_flow_update(FluidFlow* this, Vector3i position, WorldChunk* tile_chunk) {
  // first cache fluid levels
  for (int i = 0; i < FluidTypeSize; ++i) {
    this->amount_prev[i] = this->amount[i];
  }
  this->amount_total_prev = this->amount_total;

  // all neighbour coordinates
  const Vector3i neighs[6] = {
          {position.x + 1, position.y,     position.z},
          {position.x,     position.y + 1, position.z},
          {position.x,     position.y,     position.z + 1},
          {position.x - 1, position.y,     position.z},
          {position.x,     position.y - 1, position.z},
          {position.x,     position.y,     position.z - 1},
  };
  // maximum flow per neighbour
  FluidAmount max_flow[6] = {};

  // collect fluid level differences
  WorldTile* tile = world_get_tile_from_chunk(tile_chunk, &position);
  for (int i = 0; i < 6; ++i) {
    WorldChunk* other_chunk = world_get_chunk_from_chunk(tile_chunk, &neighs[i]);
    WorldTile* other_tile = world_get_tile_from_chunk(other_chunk, &neighs[i]);
    max_flow[i] = (other_tile == NULL) ? 0 : fluid_flow(tile, tile_chunk, other_tile, other_chunk);
  }

  // collect amount of fluid available
  int fluid_available = this->amount_total_prev;
  int flow_budget = fluid_available; // this absolutely prevents fluids to be pushed up
  this->flow_out_total = fluid_available; // unless this tile will stack up, this will be the case

  // first handle 'down'
  if (max_flow[5] > flow_budget) {
    this->flow_out_zn = flow_budget;
    return; // all budget spent

  } else {
    this->flow_out_zn = max_flow[5];
    flow_budget -= max_flow[5];
  }

  // handle sides
  int side_max_flow = 0;
  side_max_flow += max_flow[0];
  side_max_flow += max_flow[1];
  side_max_flow += max_flow[3];
  side_max_flow += max_flow[4];

    if (flow_budget >= side_max_flow) {
        this->flow_out_xp = max_flow[0];
        this->flow_out_yp = max_flow[1];
        this->flow_out_xn = max_flow[3];
        this->flow_out_yn = max_flow[4];
        
    } else {
        float side_flow = (float) flow_budget / side_max_flow;

        // spread by pressure
        FluidAmount flow_xp = (FluidAmount) (max_flow[0] * side_flow);
        this->flow_out_xp = flow_xp;
        FluidAmount flow_yp = (FluidAmount) (max_flow[1] * side_flow);
        this->flow_out_yp = flow_yp;
        FluidAmount flow_xn = (FluidAmount) (max_flow[3] * side_flow);
        this->flow_out_xn = flow_xn;

        FluidAmount remainder = side_max_flow - flow_xp - flow_yp - flow_xn;
        this->flow_out_yn = (FluidAmount) min_i(max_flow[4], remainder); // reduced, but present chance of fluid deletion
        return;
    }


    if (side_max_flow > flow_budget) return;
    flow_budget -= side_max_flow;

    // reserve up to AMOUNT_MAX to not move at all
    if (flow_budget > FLUID_AMOUNT_MAX) {
        this->flow_out_zp = flow_budget - FLUID_AMOUNT_MAX; // if any is still left, push up
        this->flow_out_total -= FLUID_AMOUNT_MAX;
    } else {
        this->flow_out_total -= flow_budget;
    }

    assert(this->flow_out_xp + this->flow_out_yp + this->flow_out_zp + this->flow_out_xn + this->flow_out_yn + this->flow_out_zn == this->flow_out_total);
}

void fluid_amount_update(FluidFlow* this, Vector3i position, WorldChunk* tile_chunk) {
  // all neighbour coordinates again, now immediately collect all neighbour fluid flows
  const Vector3i neighs[6] = {
          {position.x + 1, position.y,     position.z},
          {position.x,     position.y + 1, position.z},
          {position.x,     position.y,     position.z + 1},
          {position.x - 1, position.y,     position.z},
          {position.x,     position.y - 1, position.z},
          {position.x,     position.y,     position.z - 1},
  };
  const FluidFlow* neigh_fluids[6];
  for (int i = 0; i < 6; ++i) {
    WorldChunk* other_chunk = world_get_chunk_from_chunk(tile_chunk, &neighs[i]);
    WorldTile* other_tile = world_get_tile_from_chunk(other_chunk, &neighs[i]);
    neigh_fluids[i] = tile_get_fluid(other_tile, other_chunk);
  }

  // collect total in-flow per type
  int type_in_flow[FluidTypeSize];
  for (int i = 0; i < FluidTypeSize; ++i) {
    // calculate what portion of the fluid is represented by each type, then calculate in-flow of this type from each direction
    type_in_flow[i]  = neigh_fluids[0]->flow_out_xn * ((float) neigh_fluids[0]->amount_prev[i] / neigh_fluids[0]->amount_total_prev);
    type_in_flow[i] += neigh_fluids[1]->flow_out_yn * ((float) neigh_fluids[1]->amount_prev[i] / neigh_fluids[1]->amount_total_prev);
    type_in_flow[i] += neigh_fluids[2]->flow_out_zn * ((float) neigh_fluids[2]->amount_prev[i] / neigh_fluids[2]->amount_total_prev);
    type_in_flow[i] += neigh_fluids[3]->flow_out_xp * ((float) neigh_fluids[3]->amount_prev[i] / neigh_fluids[3]->amount_total_prev);
    type_in_flow[i] += neigh_fluids[4]->flow_out_yp * ((float) neigh_fluids[4]->amount_prev[i] / neigh_fluids[4]->amount_total_prev);
    type_in_flow[i] += neigh_fluids[5]->flow_out_zp * ((float) neigh_fluids[5]->amount_prev[i] / neigh_fluids[5]->amount_total_prev);
  }

  // now just add all of these incoming flows current amounts
  // also accumulate totals
  this->amount_total = 0;
  for (int i = 0; i < FluidTypeSize; ++i) {
    this->amount[i] += type_in_flow[i];
    // calculate what portion of the fluid is represented by each type, then calculate flow of this type individually
    this->amount[i] -= this->flow_out_total * ((float) this->amount_prev[i] / this->amount_total_prev);
    this->amount_total += this->amount[i];
  }
}
