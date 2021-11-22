#include "modules/api.h"
#include "connector.h"

void
create_nodes(
    dt_graph_t  *graph,
    dt_module_t *module)
{
  // TODO: network configuration:
  // optional 5x5(?) blur, = 76 coeffs per output channel
  // sum input back to result of network
  // last layer witouth relu
  const uint32_t oc[] = { // output channels of the convolutional layers
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  };
  const uint32_t numl = sizeof(oc)/sizeof(oc[0]); // number of layers
  float push[] = { 0.01, 1.0 };
  uint32_t *pushi = (uint32_t *)push;

  assert(graph->num_nodes < graph->max_nodes);
  const uint32_t id_blur = graph->num_nodes++;
  graph->node[id_blur] = (dt_node_t) {
    .name   = dt_token("cnn"),
    .kernel = dt_token("blur"),
    .module = module,
    .wd     = module->connector[0].roi.wd,
    .ht     = module->connector[0].roi.ht,
    .dp     = 1,
    .num_connectors = 3,
    .connector = {{
      .name   = dt_token("input"),
      .type   = dt_token("read"),
      .chan   = dt_token("rgba"),
      .format = dt_token("f16"),
      .roi    = module->connector[0].roi,
      .connected_mi = -1,
    },{
      .name   = dt_token("wgt"),
      .type   = dt_token("read"),
      .chan   = dt_token("rgba"),
      .format = dt_token("f16"),
      .roi    = module->connector[1].roi,
      .connected_mi = -1,
    },{
      .name   = dt_token("output"),
      .type   = dt_token("write"),
      .chan   = dt_token("rgba"),
      .format = dt_token("f16"),
      .roi    = module->connector[0].roi,
    }},
  };

  uint32_t id_C16 = id_blur;
  int cin = 3, cout = 3;

  for(int l=0;l<numl;l++)
  {
    cout = 16;
    int off = 1 + 4*numl; // y coordinate for current weights
    assert(graph->num_nodes < graph->max_nodes);
    const uint32_t id_conv = graph->num_nodes++;
    graph->node[id_conv] = (dt_node_t) {
      .name   = dt_token("cnn"),
      .kernel = dt_token("conv"),
      .module = module,
      .wd     = module->connector[0].roi.wd,
      .ht     = module->connector[0].roi.ht,
      .dp     = (cout+3)/4,
      .num_connectors = 3,
      .connector = {{
        .name   = dt_token("input"),
        .type   = dt_token("read"),
        .chan   = dt_token("rgba"),
        .format = dt_token("f16"),
        .roi    = module->connector[0].roi,
        .array_length = (cin+3)/4,
        .connected_mi = -1,
      },{
        .name   = dt_token("wgt"),
        .type   = dt_token("read"),
        .chan   = dt_token("rgba"),
        .format = dt_token("f16"),
        .roi    = module->connector[1].roi,
        .connected_mi = -1,
      },{
        .name   = dt_token("output"),
        .type   = dt_token("write"),
        .chan   = dt_token("rgba"),
        .format = dt_token("f16"),
        .roi    = module->connector[0].roi,
        .array_length = (cout+3)/4,
      }},
      .push_constant_size = 4*sizeof(uint32_t),
      .push_constant = { cin, cout, off, l == numl-1 ? pushi[1] : pushi[0] },
    };
    dt_node_connect(graph, id_C16, 2, id_conv, 0);
    dt_connector_copy(graph, module, 1, id_conv, 1);
    id_C16 = id_conv;
    cin = cout;
  }
  assert(graph->num_nodes < graph->max_nodes);
  const uint32_t id_sum = graph->num_nodes++;
  graph->node[id_sum] = (dt_node_t) {
    .name   = dt_token("cnn"),
    .kernel = dt_token("sum"),
    .module = module,
    .wd     = module->connector[0].roi.wd,
    .ht     = module->connector[0].roi.ht,
    .dp     = 1,
    .num_connectors = 3,
    .connector = {{
      .name   = dt_token("in0"),
      .type   = dt_token("read"),
      .chan   = dt_token("rgba"),
      .format = dt_token("f16"),
      .roi    = module->connector[0].roi,
      .connected_mi = -1,
    },{
      .name   = dt_token("in1"),
      .type   = dt_token("read"),
      .chan   = dt_token("rgba"),
      .format = dt_token("f16"),
      .roi    = module->connector[0].roi,
      .connected_mi = -1,
    },{
      .name   = dt_token("output"),
      .type   = dt_token("write"),
      .chan   = dt_token("rgba"),
      .format = dt_token("f16"),
      .roi    = module->connector[0].roi,
    }},
  };
  dt_connector_copy(graph, module, 0, id_blur, 0);
  dt_connector_copy(graph, module, 1, id_blur, 1);
  dt_connector_copy(graph, module, 0, id_sum, 0);
  dt_node_connect(graph, id_C16, 2, id_sum, 1);
  dt_connector_copy(graph, module, 2, id_sum, 2);
}
