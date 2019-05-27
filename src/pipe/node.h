#pragma once
#include "token.h"
#include "connector.h"

#include <vulkan/vulkan.h>

// fwd declare
typedef struct dt_module_t dt_module_t;

typedef struct dt_node_t
{
  dt_token_t name;
  dt_token_t kernel;

  dt_module_t *module;  // reference back to module and class

  dt_connector_t connector[DT_MAX_CONNECTORS];
  int num_connectors;

  VkPipeline            pipeline;
  VkPipelineLayout      pipeline_layout;
  VkDescriptorSet       dset;
  VkDescriptorSetLayout dset_layout;

  uint32_t wd, ht, dp;  // dimensions of kernel to be run
}
dt_node_t;

