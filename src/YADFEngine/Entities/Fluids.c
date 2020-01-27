//
// Created by s152717 on 28-12-2019.
//

#include "Fluids.h"

PURE int fluid_at_tile(WorldTile* tile) {
  if (tile->fluids == NULL) return 0;

  int amount = 0;
  for (int i = 0; i < FluidTypeSize; ++i) {
    amount = tile->fluids->amount[i];
  }

  return amount;
}

/**
 * @return flow of a to b, 0 if b has more fluid than a
 */
PURE static inline FluidAmount fluid_flow(WorldTile* a, WorldTile* b) {
  if (b->flags & TILE_FLAG_BLOCKING) return 0;
  int flow = fluid_at_tile(a) - fluid_at_tile(b);
  return (FluidAmount) min_i(AMOUNT_MAX, max_i(0, flow));
}

void fluid_flow_update(FluidFlow* this, Vector3i position, WorldTile* tile, WorldChunk* tile_chunk) {
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
  for (int i = 0; i < 6; ++i) {
    WorldTile* other_tile = world_get_tile_from_chunk(tile_chunk, &neighs[i]);
    max_flow[i] = (other_tile == NULL) ? 0 : fluid_flow(tile, other_tile);
  }

  // collect amount of fluid available
  int fluid_available = 0;
  for (int i = 0; i < FluidTypeSize; ++i) {
    fluid_available = this->amount_prev[i];
  }
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

  const float side_flow = (flow_budget < side_max_flow) ? ((float) flow_budget / side_max_flow) : flow_budget;

  // spread by pressure
  this->flow_out_xp = (FluidAmount) (max_flow[0] * side_flow);
  this->flow_out_yp = (FluidAmount) (max_flow[1] * side_flow);
  this->flow_out_xn = (FluidAmount) (max_flow[3] * side_flow);
  this->flow_out_yn = (FluidAmount) (max_flow[4] * side_flow);

  flow_budget -= side_max_flow;
  if (flow_budget <= 0) return;

  // reserve up to AMOUNT_MAX to not move at all
  if (flow_budget > AMOUNT_MAX) {
    this->flow_out_zp = flow_budget - AMOUNT_MAX; // if any is still left, push up
    this->flow_out_total -= AMOUNT_MAX;
  } else {
    this->flow_out_total -= flow_budget;
  }

  assert(this->flow_out_xp + this->flow_out_yp + this->flow_out_zp + this->flow_out_xn + this->flow_out_yn + this->flow_out_zn == this->flow_out_total);
}

void fluid_amount_update(WorldTile* tile, Vector3i position, WorldChunk* tile_chunk) {
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
    neigh_fluids[i] = world_get_tile_from_chunk(tile_chunk, &neighs[i])->fluids;
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

  FluidFlow* this = world_tile_get_fluid(tile, tile_chunk);

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
