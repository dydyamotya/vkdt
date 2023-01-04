#include "modules/api.h"
#include "core/lut.h"
#include "connector.h"

#include <stdio.h>
#include <string.h>

void write_sink(
    dt_module_t *module,
    void *buf)
{
  const char *basename = dt_module_param_string(module, 0);
  fprintf(stderr, "[o-lut] writing '%s'\n", basename);

  dt_lut_header_t header = (dt_lut_header_t){
    .magic    = dt_lut_header_magic,
    .version  = dt_lut_header_version,
    .channels = dt_connector_channels(module->connector+0),
    .datatype = module->connector[0].format == dt_token("f32") ? dt_lut_header_f32 :
                dt_lut_header_f16, // TODO: if not f16 output error message?
    .wd       = module->connector[0].roi.wd,
    .ht       = module->connector[0].roi.ht,
  };

  char filename[512];
  snprintf(filename, sizeof(filename), "%s.lut", basename);
  FILE* f = fopen(filename, "wb");
  if(f)
  {
    fwrite(&header, sizeof(header), 1, f);
    // use whatever connector says because that's the real allocation (in case
    // it's just uint8 or so)
    size_t bytes = header.wd * header.ht * header.channels * dt_connector_bytes_per_channel(module->connector+0);
    fwrite(buf, bytes, 1, f);
    fclose(f);
  }
}